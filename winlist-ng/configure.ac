dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(winlist-ng, 0.1.0, hannes.janetzek@gmail.com)
AC_PREREQ(2.52)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(1.6)
AC_CONFIG_SRCDIR(configure.ac)
AM_CONFIG_HEADER(config.h)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AM_ENABLE_SHARED
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

release=$(pkg-config --variable=release enlightenment)
MODULE_ARCH="$host_os-$host_cpu-$release"
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")

AC_PATH_PROGS(EDJE_CC, edje_cc)

PKG_CHECK_MODULES(E, [enlightenment])

datadir=$(pkg-config --variable=modules enlightenment)/${PACKAGE}
AC_ARG_ENABLE(homedir-install,
  AS_HELP_STRING([--enable-homedir-install], [Install module in homedir]),
  [ datadir="${HOME}/.e/e/modules/${PACKAGE}" ]
)

AC_OUTPUT([
Makefile
data/Makefile
data/themes/Makefile
src/Makefile
module.desktop
$po_makefile_in
],[
])

