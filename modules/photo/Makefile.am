ACLOCAL_AMFLAGS = -I m4
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
		       config.sub configure depcomp install-sh ltmain.sh \
		       missing module.desktop config.rpath mkinstalldirs

SUBDIRS = src data

if HAVE_PO

SUBDIRS += po

endif

EDJE_FLAGS = -v \
	     -id $(top_srcdir)/data/images

filesdir = $(datadir)
files_DATA = e-module-photo.edj module.desktop

EXTRA_DIST = module.desktop.in \
	     e_modules-photo.spec.in \
	     e-module-photo.edc

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf e-module-photo.edj module.desktop e_modules-photo.spec *~

uninstall:
	 rm -rf $(DESTDIR)$(datadir)
