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

filesdir = $(datadir)
files_DATA = images/cloud.png \
	     images/drop-s.png \
	     images/drop-m.png \
	     images/drop-l.png \
	     module.desktop \
	     e-module-rain.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-rain.spec.in \
	     e-module-rain.edc \
	     images/cloud.png \
	     images/drop-s.png \
	     images/drop-m.png \
	     images/drop-l.png \
	     images/module_icon.png

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf e-module-rain.edj module.desktop e_modules-rain.spec *~

uninstall:
	 rm -rf $(DESTDIR)$(datadir)
