m4_define([tym_major_version],[3])
m4_define([tym_minor_version],[5])
m4_define([tym_micro_version],[2])
m4_define([tym_version],[tym_major_version().tym_minor_version().tym_micro_version()])

AC_PREREQ([2.69])
AC_INIT([tym], [tym_version()], [], [tym], [https://github.com/endaaman/tym])
AM_INIT_AUTOMAKE([foreign])
DATE="`date '+%Y-%m-%d'`"
AC_SUBST(DATE)
AC_CONFIG_SRCDIR([src/tym.c])
AC_CONFIG_HEADERS([app-config.h], [])
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  include/Makefile
  include/common.h
  tym-daemon.service
  tym.1
])

AC_PROG_CC
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(TYM, [gtk+-3.0 vte-2.91 libpcre2-8])

AC_ARG_ENABLE(luajit,
  [AC_HELP_STRING([--enable-luajit], [use LuaJIT instead of the official Lua interpreter(default=no)])],
  [\
  case "${enableval}" in
    yes) enable_luajit=yes ;;
    no)  enable_luajit=no ;;
    *) AC_MSG_ERROR(bad value for --enable-luajit) ;;
  esac],
  [enable_luajit=no]
)
if test x"${enable_luajit}" = x"yes"; then
  PKG_CHECK_MODULES(LUA, [luajit])
  AC_DEFINE([USES_LUAJIT], 1, [Define to 1 to enable LuaJIT specific code])
else
  PKG_CHECK_MODULES(LUA, [lua], [], [
    PKG_CHECK_MODULES(LUA, [lua5.3])
  ])
fi

AC_ARG_ENABLE(debug,
  [AC_HELP_STRING([--enable-debug],[turn on debugging(default=no)])],
  [\
  case "${enableval}" in
   yes) enable_debug=yes ;;
   no)  enable_debug=no ;;
   *)   AC_MSG_ERROR(bad value for --enable-debug) ;;
  esac],
  [enable_debug=no]
)
if test x"${enable_debug}" = x"yes"; then
  AC_DEFINE(DEBUG, 1, [Define to 1 if you want to debug])
fi
AM_CONDITIONAL([DEBUG], [test "$enable_debug" = yes])

# --enable-old-vte
AC_ARG_ENABLE(old-vte,
  [AC_HELP_STRING([--enable-old-vte], [use old VTE API(default=no)])],
  [\
  case "${enableval}" in
    yes) enable_old_vte=yes ;;
    no)  enable_old_vte=no ;;
    *) AC_MSG_ERROR(bad value for --enable-old-vte) ;;
  esac],
  [enable_old_vte=no]
)

if test x"${enable_old_vte}" = x"yes"; then
  AC_DEFINE([TYM_USE_OLD_VTE], 1, [Define to 1 if using old VTE API])
fi

AC_OUTPUT
