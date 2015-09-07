dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(configure.ac)
AC_PREREQ(2.59)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(emprint, 0.1.0)
AC_CONFIG_HEADERS(config.h)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_PROG_CC
AC_HEADER_STDC
AC_HEADER_TIME
AM_ENABLE_SHARED
AM_PROG_LIBTOOL
AC_C___ATTRIBUTE__

dnl Set PACKAGE_DATA_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}", [PACKAGE_DATA_DIR])
else
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}", [PACKAGE_DATA_DIR])
fi

dnl Set PACKAGE_BIN_DIR in config.h.
if test "x${datadir}" = 'x${prefix}/bin'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${ac_default_prefix}/bin", [Installation directory for user executables])
  else
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${prefix}/bin", [Installation directory for user executables])
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${bindir}", [Installation directory for user executables])
fi

PKG_PROG_PKG_CONFIG

EFL_CHECK_PATH_MAX

PKG_CHECK_MODULES(EINA, eina)
PKG_CHECK_MODULES(ECORE, 
ecore
ecore-file
ecore-x
ecore-evas
ecore-input
)
PKG_CHECK_MODULES(EVAS, evas)

AC_OUTPUT([
Makefile
src/Makefile
src/bin/Makefile
data/Makefile
data/images/Makefile
], [
])
