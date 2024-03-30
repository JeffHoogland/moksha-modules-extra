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
files_DATA = module.desktop e-module-cpu.edj cpu.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-cpu.spec.in \
	     e-module-cpu.edc \
	     cpu.edc \
	     images/cpu.png \
	     images/module_icon.png

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf cpu.edj e-module-cpu.edj module.desktop e_modules-cpu.spec *~

uninstall:
	rm -rf $(DESTDIR)$(datadir)
