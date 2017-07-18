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
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/version.h
])

AC_ARG_ENABLE(debug,
AS_HELP_STRING(
  [--enable-debug],
  [enable debugging, default: no]), [
  case "${enableval}" in
    yes) debug=true ;;
    no)  debug=false ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
  esac],
  [debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

TYM_MAJOR_VERSION=tym_major_version()
TYM_MINOR_VERSION=tym_minor_version()
TYM_MICRO_VERSION=tym_micro_version()

AC_SUBST(TYM_MAJOR_VERSION)
AC_SUBST(TYM_MINOR_VERSION)
AC_SUBST(TYM_MICRO_VERSION)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(TYM, [gtk+-3.0 vte-2.91 lua53 ])

AC_OUTPUT
