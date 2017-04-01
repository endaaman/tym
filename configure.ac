#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tym], [0.1.0-alpha])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/tym.c])
AC_CONFIG_HEADERS([config.h])

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(TYM, [gtk+-3.0 vte-2.91 lua ])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
