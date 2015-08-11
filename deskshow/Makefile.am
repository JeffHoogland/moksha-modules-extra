MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
		       config.sub configure depcomp install-sh ltmain.sh \
		       missing module.desktop config.rpath mkinstalldirs

SUBDIRS = src

EDJE_FLAGS = -v \
	     -id $(top_srcdir)/images

filesdir = $(datadir)
files_DATA = module.desktop e-module-deskshow.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-deskshow.spec.in \
	     e-module-deskshow.edc \
	     images/module_icon.png

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf e-module-deskshow.edj module.desktop e_modules-deskshow.spec *~

uninstall:
	rm -rf $(DESTDIR)$(datadir)
