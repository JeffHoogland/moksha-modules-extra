MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
		       config.sub configure depcomp install-sh ltmain.sh \
		       missing module.desktop config.rpath mkinstalldirs

SUBDIRS = src

EDJE_FLAGS = -v \
	     -id $(top_srcdir)/images

filesdir =  $(datadir)
files_DATA = module.desktop e-module-winselector.edj winselector.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-winselector.spec.in \
	     e-module-winselector.edc \
	     winselector.edc \
	     images/module_icon.png

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	rm -rf e-module-winselector.edj winselector.edj module.desktop e_modules-winselector.spec *~

uninstall:
	rm -rf $(DESTDIR)$(datadir)
