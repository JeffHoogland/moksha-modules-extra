ACLOCAL_AMFLAGS = -I m4
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
		       config.sub configure depcomp install-sh ltmain.sh \
		       missing module.desktop config.rpath mkinstalldirs

SUBDIRS = src

EDJE_FLAGS = -v \
	     -id $(top_srcdir)/images

filesdir = $(datadir)
files_DATA = module.desktop e-module-diskio.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-diskio.spec.in \
	     e-module-diskio.edc \
	     images/module_icon.png \
	     images/module_icon_r.png \
	     images/module_icon_w.png

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf e-module-diskio.edj module.desktop e_modules-diskio.spec *~

uninstall:
	 rm -rf $(DESTDIR)$(datadir)
