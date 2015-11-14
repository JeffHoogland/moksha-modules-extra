MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. \
	   -I$(top_srcdir) \
	   -I$(includedir) \
	   @E_CFLAGS@

pkgdir = $(datadir)/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = e_mod_main.c \
		    e_mod_main.h \
		    e_mod_config.c \
		    e_mod_config_box.h \
		    e_mod_config_box.c \
		    pop.h pop.c \
		    mdir.h mdir.c \
		    mbox.h mbox.c

if IMAP2
module_la_SOURCES += imap2.h imap2.c
else
module_la_SOURCES += imap.h imap.c
endif

module_la_LIBADD = @E_LIBS@
module_la_LDFLAGS = -module -avoid-version
module_la_DEPENDENCIES = $(top_builddir)/config.h

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf *~
