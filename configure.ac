#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_define([tym_major_version],[0])
m4_define([tym_minor_version],[2])
m4_define([tym_micro_version],[4])
m4_define([tym_version],[tym_major_version().tym_minor_version().tym_micro_version()])

AC_PREREQ([2.69])
AC_INIT([tym], [tym_version()])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/tym.c])
AC_CONFIG_HEADERS([conf.h:conf.h.in])
AC_CONFIG_FILES([
  Makefile
  src/Makefile
])

AC_ARG_ENABLE(debug,
[  --enable-debug          turn on debugging [[default=no]]],
[\
case "${enableval}" in
 yes) enable_debug=yes ;;
 no)  enable_debug=no ;;
 *)   AC_MSG_ERROR(bad value for --enable-debug) ;;
esac],
enable_debug=no)
if test x"${enable_debug}" = x"yes"; then
  AC_DEFINE(DEBUG, 1, [Define to 1 if you want to debug])
fi

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(TYM, [gtk+-3.0 vte-2.91 lua53 ])

AC_OUTPUT
