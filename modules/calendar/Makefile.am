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
files_DATA = e-module-calendar.edj module.desktop calendar.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-calendar.spec.in \
	     e-module-calendar.edc \
	     calendar.edc \
	     images/arrow_left.png \
	     images/arrow_right.png \
	     images/arrow2_left.png \
	     images/arrow2_right.png \
	     images/calendar.png \
	     images/module_icon.png

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	rm -rf calendar.edj e-module-calendar.edj module.desktop e_modules-calendar.spec *~

uninstall:
	rm -rf $(DESTDIR)$(datadir)
