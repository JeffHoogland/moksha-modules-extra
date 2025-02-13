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

filesdir = $(module_dir)/$(PACKAGE)
files_DATA = module.desktop e-module-echievements.edj images/trophy.png

EXTRA_DIST = module.desktop.in \
	     e_modules-echievements.spec.in \
	     e-module-echievements.edc \
	     images/trophy.png

e-module-echievements.edj: Makefile e-module-echievements.edc images/trophy.png

e-module-echievements.edj:  e-module-echievements.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	 rm -rf *.edj module.desktop e_modules-echievements.spec *~

uninstall-local:
	 rm -rf $(DESTDIR)$(module_dir)/$(PACKAGE)

