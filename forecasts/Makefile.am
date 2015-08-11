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
files_DATA = module.desktop e-module-forecasts.edj forecasts.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-forecasts.spec.in \
	     e-module-forecasts.edc \
	     forecasts.edc \
	     $(wildcard images/*.png)

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	rm -rf forecasts.edj e-module-forecasts.edj module.desktop e_modules-forecasts.spec *~

uninstall:
	rm -rf $(DESTDIR)$(datadir)
