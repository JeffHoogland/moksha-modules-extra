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
files_DATA = e-module-alarm.edj module.desktop alarm.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-alarm.spec.in \
	     e-module-alarm.edc \
	     alarm.edc \
	     images/alarm_enabled.png \
	     images/alarm_disabled.png \
	     images/alarm_snooze.png \
	     images/module_icon.png

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	rm -rf alarm.edj e-module-alarm.edj module.desktop e_modules-alarm.spec *~

uninstall:
	rm -rf $(DESTDIR)$(datadir)
