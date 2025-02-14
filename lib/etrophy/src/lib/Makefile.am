MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVIL_CFLAGS@ \
@ELEMENTARY_CFLAGS@

lib_LTLIBRARIES = libetrophy.la

includes_HEADERS = Etrophy.h
includesdir = $(includedir)/etrophy-@VMAJ@

base_sources = \
    etrophy_layouts.c \
    etrophy.c

libetrophy_la_SOURCES = $(base_sources)

libetrophy_la_LIBADD = @ELEMENTARY_LIBS@ @EVIL_LIBS@ -lm
libetrophy_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@
