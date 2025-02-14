MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
		       config.sub configure depcomp install-sh ltmain.sh \
		       missing module.desktop config.rpath mkinstalldirs

SUBDIRS = src

EDJE_FLAGS = -v \
	     -id $(top_srcdir)/images

filesdir = $(datadir)
files_DATA = module.desktop e-module-execwatch.edj execwatch.edj

EXTRA_DIST = module.desktop.in \
	     execwatch.edc \
	     e-module-execwatch.edc \
	     e_modules-execwatch.spec.in \
	     $(wildcard images/*.png)

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf e-module-execwatch.edj execwatch.edj *~

uninstall:
	rm -rf $(DESTDIR)$(datadir)
