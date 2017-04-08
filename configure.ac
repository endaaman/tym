#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tym], [0.1.1])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/tym.c])
AC_CONFIG_HEADERS([config.h])

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

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(TYM, [gtk+-3.0 vte-2.91 lua53 ])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT


