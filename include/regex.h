/**
 * regex.h
 *
 * URI regular expression in PCRE2
 * reference: RFC 3986 Appendix A (https://tools.ietf.org/html/rfc3986#appendix-A)
 *
 * NOTE:
 *   * URI is repleced by SCHEMELESS_URI, because the entire URI regex is dynamically constructed
 *     using an user-configured list of target schemes. SCHEME is used to validate the list.
 *   * Also, the following definitions are omitted:
 *     - URI-reference      only used in relative URIs.
 *     - relative-ref       (as above)
 *     - relative-part      (as above)
 *     - path-noscheme      (as above)
 *     - segment-nz-nc      (as above)
 *     - absolute-URI       special case of URI. not distinguishable in regex.
 *     - path               not referenced from any other rules.
 *     - path-empty         to avoid highlighting meaningless URI like `foo:`.
 *
 * Copyright (c) 2020 endaaman, iTakeshi
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */


#ifndef REGEX_H
#define REGEX_H


/*
 * inherited from RFC 2234 Section 6.1 (https://tools.ietf.org/html/rfc2234#section-6.1)
 * NOTE: the difinitions for ALPHA and HEXDIG assume those are used in a case-insensitive manner.
 */

#define ALPHA           "(?:" "[a-z]" ")"

#define DIGIT           "(?:" "[0-9]" ")"

#define HEXDIG          "(?:" DIGIT "|" "[a-f]" ")"

#define SP              "(?:" " " ")"


/*
 * rules to validate the user-configured scheme list
 */

#define SCHEME          "(" ALPHA "(?:" ALPHA "|" DIGIT "|" "[\\+\\-\\.]" ")*" ")"

#define SCHEME_LIST     SCHEME "(?:" SP SCHEME ")*"


/*
 * main rules
 */

#define SCHEMELESS_URI  "(?:" ":" HIER_PART "(?:" "\\?" QUERY ")?" "(?:" "\\#" FRAGMENT ")?" ")"

#define HIER_PART       "(?:" "\\/\\/" AUTHORITY PATH_ABEMPTY "|" PATH_ABSOLUTE "|" PATH_ROOTLESS ")"

#define AUTHORITY       "(?:" USERINFO "@" ")?" HOST "(?:" ":" PORT ")?"

#define USERINFO        "(?:" UNRESERVED "|" PCT_ENCODED "|" SUB_DELIMS "|" ":" ")*"

#define HOST            "(?:" IP_LITERAL "|" IPV4ADDRESS "|" REG_NAME")"

#define PORT            "(?:" DIGIT ")*"

#define IP_LITERAL      "(?:" "\\[" "(?:" IPV6ADDRESS "|" IPVFUTURE ")" "\\]" ")"

#define IPVFUTURE       "(?:" "v" "(?:" HEXDIG ")+" "\\." "(?:" UNRESERVED "|" SUB_DELIMS "|" ":" ")+" ")"

#define IPV6ADDRESS     "(?:"                                            "(?:" H16 ":" "){6}" LS32 \
                          "|"                                       "::" "(?:" H16 ":" "){5}" LS32 \
                          "|" "(?:"                        H16 ")?" "::" "(?:" H16 ":" "){4}" LS32 \
                          "|" "(?:" "(?:" H16 ":" "){0,1}" H16 ")?" "::" "(?:" H16 ":" "){3}" LS32 \
                          "|" "(?:" "(?:" H16 ":" "){0,2}" H16 ")?" "::" "(?:" H16 ":" "){2}" LS32 \
                          "|" "(?:" "(?:" H16 ":" "){0,3}" H16 ")?" "::" "(?:" H16 ":" "){1}" LS32 \
                          "|" "(?:" "(?:" H16 ":" "){0,4}" H16 ")?" "::"                      LS32 \
                          "|" "(?:" "(?:" H16 ":" "){0,5}" H16 ")?" "::"                      H16  \
                          "|" "(?:" "(?:" H16 ":" "){0,6}" H16 ")?" "::"                           \
                          ")"

#define H16             "(?:" HEXDIG "){1,4}"

#define LS32            "(?:" H16 ":" H16 "|" IPV4ADDRESS ")"

#define IPV4ADDRESS     "(?:" DEC_OCTET "\\." DEC_OCTET "\\." DEC_OCTET "\\." DEC_OCTET ")"

#define DEC_OCTET       "(?:" DIGIT "|" "[1-9]" DIGIT "|" "1" DIGIT DIGIT "|" "2" "[0-4]" DIGIT "|" "25" "[0-5]" ")"

#define REG_NAME        "(?:" UNRESERVED "|" PCT_ENCODED "|" SUB_DELIMS ")*"

#define PATH_ABEMPTY    "(?:" "\\/" SEGMENT ")*"

#define PATH_ABSOLUTE   "(?:" "\\/" "(?:" SEGMENT_NZ PATH_ABEMPTY ")?" ")"

#define PATH_ROOTLESS   "(?:" SEGMENT_NZ PATH_ABEMPTY ")"

#define SEGMENT         "(?:" PCHAR ")*"

#define SEGMENT_NZ      "(?:" PCHAR ")+"

#define PCHAR           "(?:" UNRESERVED "|" PCT_ENCODED "|" SUB_DELIMS "|" ":" "|" "@" ")"

#define QUERY           "(?:" PCHAR "|" "\\/" "|" "\\?" ")*"

#define FRAGMENT        "(?:" PCHAR "|" "\\/" "|" "\\?" ")*"

#define PCT_ENCODED     "(?:" "%" HEXDIG HEXDIG ")"

#define UNRESERVED      "(?:" ALPHA "|" DIGIT "|" "[\\-\\._~]" ")"

#define RESERVED        "(?:" GEN_DELIMS "|" SUB_DELIMS ")"

#define GEN_DELIMS      "(?:" "[:\\/\\?\\#\\[\\]@]" ")"

#define SUB_DELIMS      "(?:" "[!\\$&'\\(\\)\\*\\+,;=]" ")"


#endif
