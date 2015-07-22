dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(places, 0.5.0, dave@gurumeditation.it)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.ac)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX

AM_INIT_AUTOMAKE(1.8)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_HEADERS(config.h)

AC_PROG_CC
AC_HEADER_STDC
AC_C_CONST
AC_C___ATTRIBUTE__

define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

m4_ifdef([AM_GNU_GETTEXT_VERSION], [
AM_GNU_GETTEXT_VERSION([0.14])
])

m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
AM_CONDITIONAL([HAVE_PO], [true])
],[
AM_CONDITIONAL([HAVE_PO], [false])
])
AC_SUBST(LTLIBINTL)

# check for Enlightenment
PKG_CHECK_MODULES(E, [enlightenment < 0.17.99])
release=$(pkg-config --variable=release enlightenment)
MODULE_ARCH="$host_os-$host_cpu-$release"
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")

# Find edje_cc
PKG_CHECK_MODULES(EDJE, [edje >= 0.5.0])
AC_ARG_WITH(edje-cc,
  AC_HELP_STRING([--with-edje-cc=PATH], [specify a specific path to edje_cc]),
  [
    v=$withval;
    EDJE_CC=$v
  ],[
    EDJE_CC=$(pkg-config --variable=prefix edje)/bin/edje_cc
  ]
)
AC_SUBST(EDJE_CC)
AC_MSG_CHECKING([Which edje_cc to use])
AC_MSG_RESULT(${EDJE_CC})


# check for udisks, using e_dbus v1 (default enabled) (DEPRECATED)
PKG_CHECK_MODULES([E_UDISKS], [dbus-1 edbus eukit >= 1.0.999],
                  [have_udisks="yes"], [have_udisks="no"])
want_udisks=yes
AC_ARG_ENABLE([udisks],
   AS_HELP_STRING([--enable-udisks],[enable udisks support @<:@default=enabled@:>@]), 
   [want_udisks=$enableval])
if test "x$want_udisks" = "xno";then
   have_udisks=no
fi

# check for edbus v2 (default enabled)
PKG_CHECK_MODULES([EDBUS2], [edbus2 >= 1.7.99],
                  [have_edbus2="yes"], [have_edbus2="no"])
want_edbus2=yes
AC_ARG_ENABLE([edbus2],
   AS_HELP_STRING([--enable-edbus2],[enable UDisks with edbus2 support @<:@default=enabled@:>@]), 
   [want_edbus2=$enableval])
if test "x$want_edbus2" = "xno";then
   have_edbus2=no
fi

# check for eeze (default disabled)
PKG_CHECK_MODULES([EEZE], [eeze >= 1.1.99],
                  [have_eeze="yes"], [have_eeze="no"])
want_eeze=no
AC_ARG_ENABLE([eeze],
   AS_HELP_STRING([--enable-eeze],[enable eeze support @<:@default=disabled@:>@]), 
   [want_eeze=$enableval])
if test "x$want_eeze" != "xyes";then
   have_eeze=no
fi

# do not use both e_dbus and eldbus
if test "x$have_edbus2" = "xyes";then
   have_udisks=no
fi

AM_CONDITIONAL([HAVE_EDBUS2], [test "x$have_edbus2" = "xyes"])
test "x$have_edbus2" = "xyes" && AC_DEFINE_UNQUOTED([HAVE_EDBUS2], [1], [enable edbus2 support])

AM_CONDITIONAL([HAVE_UDISKS], [test "x$have_udisks" = "xyes"])
test "x$have_udisks" = "xyes" && AC_DEFINE_UNQUOTED([HAVE_UDISKS], [1], [enable Udisks support])

AM_CONDITIONAL([HAVE_EEZE], [test "x$have_eeze" = "xyes"])
test "x$have_eeze" = "xyes" && AC_DEFINE_UNQUOTED([HAVE_EEZE], [1], [enable eeze support])


# homedir install
datadir=$(pkg-config --variable=modules enlightenment)/${PACKAGE}
AC_ARG_ENABLE(homedir-install,
  AS_HELP_STRING([--enable-homedir-install], [Install module in homedir]),
  [ datadir="${HOME}/.e/e/modules/${PACKAGE}" ]
)


AC_OUTPUT([
Makefile
src/Makefile
module.desktop
e_modules-places.spec
$po_makefile_in
], [
])


# report
txt_strip() {
  echo "[$]@" | sed -e 's/^[[ \t]]*\([[^ \t]]*\)[[ \t]]*$/\1/g'
}

echo
cat << SUMMARY_EOF
Summary:
 * project.........: $PACKAGE $VERSION
 * prefix..........: $(txt_strip $prefix)
 * CFLAGS..........: $(txt_strip $CFLAGS)
 * LDFLAGS.........: $(txt_strip $LDFLAGS)
SUMMARY_EOF
echo
cat << DEVICE_EOF
Device Backends:
 * build udisks (e_dbus v1)....: $have_udisks (deprecated)
 * build udisks (edbus  v2)....: $have_edbus2 (suggested)
 * build eeze..................: $have_eeze (experimental)
DEVICE_EOF
echo
