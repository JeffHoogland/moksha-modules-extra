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

filesdir =  $(datadir)
files_DATA = module.desktop e-module-slideshow.edj slideshow.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-slideshow.spec.in \
	     e-module-slideshow.edc \
	     slideshow.edc \
	     images/module_icon.png

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf e-module-slideshow.edj slideshow.edj module.desktop e_modules-slideshow.spec *~

uninstall:
	 rm -rf $(DESTDIR)$(datadir)
