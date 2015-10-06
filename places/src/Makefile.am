MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. \
	   -I$(top_srcdir) \
	   -I$(includedir) \
	   @E_CFLAGS@ @E_UDISKS_CFLAGS@ @EEZE_CFLAGS@ -Wall

pkgdir = $(datadir)/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = e_mod_main.h \
                    e_mod_main.c \
                    e_mod_config.c \
                    e_mod_places.h \
                    e_mod_places.c \
                    e_mod_udisks.h \
                    e_mod_udisks.c \
                    e_mod_eeze.h \
                    e_mod_eeze.c

module_la_LIBADD = @E_LIBS@ @E_UDISKS_LIBS@ @EEZE_LIBS@
module_la_LDFLAGS = -module -avoid-version -Wall
module_la_DEPENDENCIES = $(top_builddir)/config.h

clean-local:
	rm -rf *~
