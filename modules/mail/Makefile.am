ACLOCAL_AMFLAGS = -I m4
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
		       config.sub configure depcomp install-sh ltmain.sh \
		       missing module.desktop config.rpath mkinstalldirs

SUBDIRS = src images

if HAVE_PO

SUBDIRS += po

endif

src_module_module_la_CFLAGS= $(AM_CFLAGS)
src_module_module_la_LDFLAGS = -module -avoid-version

if HAVE_ENOTIFY
src_module_module_la_CFLAGS += @ENOTIFY_CFLAGS@
#//src_module_module_la_LIBADD += @ENOTIFY_LIBS@
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
