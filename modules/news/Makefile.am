ACLOCAL_AMFLAGS = -I m4
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = src data

if HAVE_PO

SUBDIRS += po

endif

filesdir = $(datadir)
files_DATA = module.desktop

EXTRA_DIST = module.desktop.in e_modules-news.spec.in

clean-local:
	 rm -rf module.desktop e_modules-news.spec *~

uninstall:
	 rm -rf $(DESTDIR)$(datadir)
