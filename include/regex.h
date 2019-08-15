/**
 * regex.h
 *
 * IRI regular expression in PCRE2
 * reference: RFC3987 http://www.faqs.org/rfcs/rfc3987.html
 *
 * NOTE
 * Those rules are defined in the order of appearance in the RFC document.
 * `IRI_REFERENCE`, `IRELATIVE_REF`, `IRELATIVE_PART`, `IPATH_NOSCHEME`, and
 * `ISEGMENT_NZ_NC` are omitted because they are only used for relative IRIs,
 * which we do not consider in this implementation.
 * Also, `ipath` is omitted because it is not used in any other rules.
 *
 * Copyright (c) 2019 endaaman, iTakeshi
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef REGEX_H
#define REGEX_H


#define IRI             ABSOLUTE_IRI "(?:\\#" IFRAGMENT ")?"

#define IHIER_PART      "(?:\\/\\/" IAUTHORITY IPATH_ABEMPTY "|" IPATH_ABSOLUTE "|" IPATH_ROOTLESS "|" IPATH_EMPTY ")"

#define ABSOLUTE_IRI    SCHEME ":" IHIER_PART "(?:\\?" IQUERY ")?"

#define IAUTHORITY      "(?:" IUSERINFO "@)?" IHOST "(?::" PORT ")?"

#define IUSERINFO       "(?:" PCT_ENCODED "|[" IUNRESERVED SUB_DELIMS ":])*"

#define IHOST           "(?:" IP_LITERAL "|" IPV4ADDRESS "|" IREG_NAME")"

#define IREG_NAME       "(?:" PCT_ENCODED "|[" IUNRESERVED SUB_DELIMS "])*"

#define IPATH_ABEMPTY   "(?:\\/" ISEGMENT ")*"

#define IPATH_ABSOLUTE  "\\/(?:" ISEGMENT_NZ IPATH_ABEMPTY ")?"

#define IPATH_ROOTLESS  ISEGMENT_NZ IPATH_ABEMPTY

#define IPATH_EMPTY     "(?!" IPCHAR ")"

#define ISEGMENT        "(?:" IPCHAR ")*"

#define ISEGMENT_NZ     "(?:" IPCHAR ")+"

#define IPCHAR          "(?:" PCT_ENCODED "|[" IUNRESERVED SUB_DELIMS ":@])"

#define IQUERY          "(?:" IPCHAR "|[" IPRIVATE "\\/\\?])*"

#define IFRAGMENT       "(?:" IPCHAR "|[\\/\\?])*"

#define IUNRESERVED     UNRESERVED UCSCHAR

#define UCSCHAR            "\\x{A0}-\\x{D7FF}"  \
                         "\\x{F900}-\\x{FDCF}"  \
                         "\\x{FDF0}-\\x{FFEF}"  \
                        "\\x{10000}-\\x{1FFFD}" \
                        "\\x{20000}-\\x{2FFFD}" \
                        "\\x{30000}-\\x{3FFFD}" \
                        "\\x{40000}-\\x{4FFFD}" \
                        "\\x{50000}-\\x{5FFFD}" \
                        "\\x{60000}-\\x{6FFFD}" \
                        "\\x{70000}-\\x{7FFFD}" \
                        "\\x{80000}-\\x{8FFFD}" \
                        "\\x{90000}-\\x{9FFFD}" \
                        "\\x{A0000}-\\x{AFFFD}" \
                        "\\x{B0000}-\\x{BFFFD}" \
                        "\\x{C0000}-\\x{CFFFD}" \
                        "\\x{D0000}-\\x{DFFFD}" \
                        "\\x{E1000}-\\x{EFFFD}" \

#define IPRIVATE          "\\x{E000}-\\x{F8FF}"  \
                         "\\x{F0000}-\\x{FFFFD}" \
                        "\\x{100000}-\\x{10FFFD}"

#define SCHEME          "[" ALPHA "][" ALPHA DIGIT "\\+\\-\\.]*"

#define PORT            "[" DIGIT "]*"

#define IP_LITERAL      "\\[(?:" IPV6ADDRESS "|" IPVFUTURE ")\\]"

#define IPVFUTURE       "v[" HEXDIG "]+\\.[" UNRESERVED SUB_DELIMS ":]+"

#define IPV6ADDRESS     "(?:"                                      "(?:" H16 ":){6}" LS32 \
                          "|"                                    "::(?:" H16 ":){5}" LS32 \
                          "|" "(?:"                     H16 ")?" "::(?:" H16 ":){4}" LS32 \
                          "|" "(?:" "(?:" H16 ":){0,1}" H16 ")?" "::(?:" H16 ":){3}" LS32 \
                          "|" "(?:" "(?:" H16 ":){0,2}" H16 ")?" "::(?:" H16 ":){2}" LS32 \
                          "|" "(?:" "(?:" H16 ":){0,3}" H16 ")?" "::"    H16 ":"     LS32 \
                          "|" "(?:" "(?:" H16 ":){0,4}" H16 ")?" "::"                LS32 \
                          "|" "(?:" "(?:" H16 ":){0,5}" H16 ")?" "::"                H16  \
                          "|" "(?:" "(?:" H16 ":){0,6}" H16 ")?" "::"                     \
                          ")"

#define H16             "[" HEXDIG "]{1,4}"

#define LS32            "(?:" H16 ":" H16 "|" IPV4ADDRESS ")"

#define IPV4ADDRESS     DEC_OCTET "(?:\\." DEC_OCTET "){3}"

#define DEC_OCTET       "(?:" "[" DIGIT "]"       \
                          "|" "[1-9][" DIGIT "]"  \
                          "|" "1[" DIGIT "]{2}"   \
                          "|" "2[0-4][" DIGIT "]" \
                          "|" "25[0-5]"           \
                          ")"

#define PCT_ENCODED     "%[" HEXDIG "]{2}"

#define UNRESERVED      ALPHA DIGIT "\\-\\._~"

#define RESERVED        GEN_DELIMS SUB_DELIMS

#define GEN_DELIMS      ":\\/\\?\\#\\[\\]@"

#define SUB_DELIMS      "!\\$&'\\(\\)\\*\\+,;="

#define ALPHA           "a-z"

#define DIGIT           "0-9"

#define HEXDIG          "0-9a-f"


#endif
