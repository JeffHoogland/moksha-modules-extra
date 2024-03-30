ACLOCAL_AMFLAGS = -I m4
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
		       config.sub configure depcomp install-sh ltmain.sh \
		       missing module.desktop config.rpath mkinstalldirs

SUBDIRS = src

if HAVE_PO

SUBDIRS += po

endif

EDJE_FLAGS = -v \
	     -id $(top_srcdir)/images

filesdir =  $(datadir)
files_DATA = module.desktop e-module-trash.edj

EXTRA_DIST = module.desktop.in \
	     $(wildcard images/*.png) \
	     $(wildcard images/*.xcf) \
	     e-module-trash.edc

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	rm -rf e-module-trash.edj module.desktop *~

uninstall:
	rm -rf $(DESTDIR)$(datadir)
	 
