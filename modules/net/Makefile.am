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
files_DATA = module.desktop e-module-net.edj net.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-net.spec.in \
	     net.edc \
	     e-module-net.edc \
	     $(wildcard images/*.png)

%.edj: %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	rm -rf e-module-net.edj net.edj module.desktop e_modules-net.spec *~

uninstall:
	 rm -rf $(DESTDIR)$(datadir)
