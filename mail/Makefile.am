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
files_DATA = module.desktop e-module-mail.edj mail.edj

EXTRA_DIST = module.desktop.in \
	     e_modules-mail.spec.in \
	     e-module-mail.edc \
	     mail.edc \
	     images/checking.png \
	     images/newmail.png \
	     images/module_icon.png

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	rm -rf mail.edj e-module-mail.edj module.desktop e_modules-mail.spec *~

uninstall:
	rm -rf $(DESTDIR)$(datadir)
