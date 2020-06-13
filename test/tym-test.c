/**
 * tym-test.c
 *
 * Copyright (c) 2019 endaaman, iTakeshi
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"
#include "regex.h"

#define URI "(?:http|https|file|mailto)" SCHEMELESS_URI


int check_match(int anchored, const char* pattern, const char* subject, const char* expected, int invert)
{
  if (expected == NULL) expected = subject;

  int errorcode;
  PCRE2_SIZE erroroffset;
  pcre2_code* code = pcre2_compile(
    pattern,
    PCRE2_ZERO_TERMINATED,
    PCRE2_UTF |
    PCRE2_NO_UTF_CHECK |
    PCRE2_MULTILINE |
    PCRE2_CASELESS |
    PCRE2_NEVER_BACKSLASH_C |
    PCRE2_USE_OFFSET_LIMIT |
    (anchored ? PCRE2_ANCHORED : 0),
    &errorcode,
    &erroroffset,
    NULL
  );
  if (!code) {
    printf("pcre2_compile failed for errorcode `%d` at offset `%d`\n", errorcode, (int)erroroffset);
    return 1;
  }

  pcre2_match_data_8 *match_data = pcre2_match_data_create_8(256, NULL);
  int res = pcre2_match(
    code,
    subject,
    PCRE2_ZERO_TERMINATED,
    0,
    PCRE2_NO_UTF_CHECK |
    PCRE2_NOTEMPTY |
    (anchored ? PCRE2_ANCHORED : 0),
    match_data,
    NULL
  );

  if (res > 0) {
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
    int offset = ovector[0];
    int length = ovector[1] - ovector[0];
    char matched[256] = { 0 };
    strncpy(matched, &subject[offset], length);
    if (length == strlen(expected) && strncmp(matched, expected, length) == 0) {
      if (invert) {
        printf("  UNEXPECTED MATCH: matched=\"%s\", expected=fail\n", matched);
        return 0;
      } else {
        printf("  MATCH SUCCESS: %s\n", matched);
        return 1;
      }
    } else {
      if (invert) {
        printf("  EXPECTED UNMATCH: %s\n", subject);
        return 1;
      } else {
        printf("  UNEXPECTED MATCH: matched=\"%s\", expected=\"%s\"\n", matched, expected);
        return 0;
      }
    }
  } else {
    if (invert && res == PCRE2_ERROR_NOMATCH) {
      printf("  EXPECTED UNMATCH: %s\n", subject);
      return 1;
    } else {
      char mes[256] = {};
      pcre2_get_error_message(res, mes, 256);
      printf("  PCRE2_MATCH ERROR: code=%d, message=\"%s\"\n", res, mes);
      return 0;
    }
  }

  pcre2_match_data_free(match_data);
  pcre2_code_free(code);
}

void test_regex()
{
  printf("Testing HOST\n");
  assert(check_match(1, SCHEME , "http"    , NULL , 0));
  assert(check_match(1, SCHEME , "HTTP"    , NULL , 0));
  assert(check_match(1, SCHEME , "foo0.-+" , NULL , 0));
  assert(check_match(1, SCHEME , "0foo"    , NULL , 1)); // disallow non-alphabet character at the beginning

  printf("Testing USERINFO\n");
  assert(check_match(1, USERINFO , "foo.bar-baz"          , NULL , 0));
  assert(check_match(1, USERINFO , "user:pass!$&'()*+,;=" , NULL , 0));
  assert(check_match(1, USERINFO , "user@"                , NULL , 1)); // disallow `@`
  assert(check_match(1, USERINFO , "user:pass@"           , NULL , 1)); // disallow `@`

  printf("Testing HOST\n");
  assert(check_match(1 , HOST , "localhost"                 , NULL , 0));
  assert(check_match(1 , HOST , "example.com"               , NULL , 0));
  assert(check_match(1 , HOST , "a-abc_d;.e.012"            , NULL , 0));
  assert(check_match(1 , HOST , "172.0.0.1"                 , NULL , 0));
  assert(check_match(1 , HOST , "[2001:db8::1234:0:0:9abc]" , NULL , 0));
  assert(check_match(1 , HOST , "あいう.example.com"        , NULL , 1)); // disallow non-ascii
  assert(check_match(1 , HOST , "172.0.0.300"               , NULL , 1)); // check ip-v4 range
  assert(check_match(1 , HOST , "example.co/m"              , NULL , 1)); // disallow `/`

  printf("Testing QUERY\n");
  assert(check_match(1 , QUERY , "foo0=bar0" , NULL , 0));
  assert(check_match(1 , QUERY , "foo0=bar0&foo1=bar1" , NULL , 0));
  assert(check_match(1 , QUERY , "foo0=bar0&path=baz/qux?quux" , NULL , 0));

  // cases are cited from RFC3987 and RFC6068
  printf("Integrated tests\n");
  assert(check_match(0 , URI , "http://www.example.org/D%C3%BCrst"                                        , "http://www.example.org/D%C3%BCrst"                                      , 0));
  assert(check_match(0 , URI , "http://www.example.org/D&#xFC;rst"                                        , "http://www.example.org/D&#xFC;rst"                                      , 0));
  assert(check_match(0 , URI , "http://www.example.org/D%FCrst"                                           , "http://www.example.org/D%FCrst"                                         , 0));
  assert(check_match(0 , URI , "http://xn--99zt52a.example.org/%e2%80%ae"                                 , "http://xn--99zt52a.example.org/%e2%80%ae"                               , 0));
  assert(check_match(0 , URI , "\"http://ab.CDEFGH.ij/kl/mn/op.html\""                                    , "http://ab.CDEFGH.ij/kl/mn/op.html"                                      , 0));
  assert(check_match(0 , URI , "\"http://AB.CD.EF/GH/IJ/KL?MN=OP;QR=ST#UV\""                              , "http://AB.CD.EF/GH/IJ/KL?MN=OP;QR=ST#UV"                                , 0));
  assert(check_match(0 , URI , "\"http://VU#TS=RQ;PO=NM?LK/JI/HG/FE.DC.BA\""                              , "http://VU#TS=RQ;PO=NM?LK/JI/HG/FE.DC.BA"                                , 0));
  assert(check_match(0 , URI , "\"http://AB.CD.ef/gh/IJ/KL.html\""                                        , "http://AB.CD.ef/gh/IJ/KL.html"                                          , 0));
  assert(check_match(0 , URI , "<mailto:addr1@an.example,addr2@an.example>"                               , "mailto:addr1@an.example,addr2@an.example"                               , 0));
  assert(check_match(0 , URI , "<mailto:chris@example.com>"                                               , "mailto:chris@example.com"                                               , 0));
  assert(check_match(0 , URI , "<mailto:infobot@example.com?subject=current-issue>"                       , "mailto:infobot@example.com?subject=current-issue"                       , 0));
  assert(check_match(0 , URI , "<mailto:infobot@example.com?body=send%20current-issue>"                   , "mailto:infobot@example.com?body=send%20current-issue"                   , 0));
  assert(check_match(0 , URI , "<mailto:infobot@example.com?body=send%20current-issue%0D%0Asend%20index>" , "mailto:infobot@example.com?body=send%20current-issue%0D%0Asend%20index" , 0));
  assert(check_match(0 , URI , "<mailto:list@example.org?In-Reply-To=%3C3469A91.D10AF4C@example.com%3E>"  , "mailto:list@example.org?In-Reply-To=%3C3469A91.D10AF4C@example.com%3E"  , 0));
  assert(check_match(0 , URI , "<mailto:majordomo@example.com?body=subscribe%20bamboo-l>"                 , "mailto:majordomo@example.com?body=subscribe%20bamboo-l"                 , 0));
  assert(check_match(0 , URI , "<mailto:joe@example.com?cc=bob@example.com&body=hello>"                   , "mailto:joe@example.com?cc=bob@example.com&body=hello"                   , 0));
  assert(check_match(0 , URI , "<mailto:addr1@an.example?to=addr2@an.example>"                            , "mailto:addr1@an.example?to=addr2@an.example"                            , 0));
  assert(check_match(0 , URI , "<mailto:gorby%25kremvax@example.com>"                                     , "mailto:gorby%25kremvax@example.com"                                     , 0));
  assert(check_match(0 , URI , "<mailto:unlikely%3Faddress@example.com?blat=foop>"                        , "mailto:unlikely%3Faddress@example.com?blat=foop"                        , 0));
  assert(check_match(0 , URI , "<a href=\"mailto:joe@an.example?cc=bob@an.example&amp;body=hello\">"      , "mailto:joe@an.example?cc=bob@an.example&amp;body=hello"                 , 0));
  assert(check_match(0 , URI , "<mailto:Mike%26family@example.org>."                                      , "mailto:Mike%26family@example.org"                                       , 0));
  assert(check_match(0 , URI , "<mailto:%22not%40me%22@example.org>."                                     , "mailto:%22not%40me%22@example.org"                                      , 0));
  assert(check_match(0 , URI , "<mailto:%22oh%5C%5Cno%22@example.org>."                                   , "mailto:%22oh%5C%5Cno%22@example.org"                                    , 0));
  assert(check_match(0 , URI , "<mailto:%22%5C%5C%5C%22it's%5C%20ugly%5C%5C%5C%22%22@example.org>."       , "mailto:%22%5C%5C%5C%22it's%5C%20ugly%5C%5C%5C%22%22@example.org"        , 0));
  assert(check_match(0 , URI , "<mailto:user@example.org?subject=caf%C3%A9>"                              , "mailto:user@example.org?subject=caf%C3%A9"                              , 0));
  assert(check_match(0 , URI , "<mailto:user@example.org?subject=%3D%3Futf-8%3FQ%3Fcaf%3DC3%3DA9%3F%3D>"  , "mailto:user@example.org?subject=%3D%3Futf-8%3FQ%3Fcaf%3DC3%3DA9%3F%3D"  , 0));
  assert(check_match(0 , URI , "<mailto:user@example.org?subject=%3D%3Fiso-8859-1%3FQ%3Fcaf%3DE9%3F%3D>"  , "mailto:user@example.org?subject=%3D%3Fiso-8859-1%3FQ%3Fcaf%3DE9%3F%3D"  , 0));
  assert(check_match(0 , URI , "<mailto:user@example.org?subject=caf%C3%A9&body=caf%C3%A9>"               , "mailto:user@example.org?subject=caf%C3%A9&body=caf%C3%A9"               , 0));
  assert(check_match(0 , URI , "<mailto:user@%E7%B4%8D%E8%B1%86.example.org?subject=Test&body=NATTO>"     , "mailto:user@%E7%B4%8D%E8%B1%86.example.org?subject=Test&body=NATTO"     , 0));
  assert(check_match(0 , URI , "file:///"                                                                 , "file:///"                                                               , 0));
  assert(check_match(0 , URI , "file:///home/user/example.txt"                                            , "file:///home/user/example.txt"                                          , 0));

  // NOT match
  assert(check_match(0 , URI , "foo:" , NULL , 1));  // only scheme-like part

  // TODO https://github.com/endaaman/tym/issues/46
  //
  // assert(check_match(0, URI, "[link](https://example.com)"  , "https://example.com" , 0));
  // assert(check_match(0, URI, "link to https://example.com." , "https://example.com" , 0));

  printf("regex tests complete!\n");
}

int main(int argc, char* argv[])
{
  test_regex();
  return 0;
}
