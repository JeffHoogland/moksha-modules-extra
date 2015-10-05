MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. \
	   -I$(top_srcdir) \
	   -I$(includedir) \
	   @E_CFLAGS@ 

pkgdir = $(datadir)/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = e_mod_main.c \
		    e_mod_main.h \
		    e_mod_keybindings.c \
		    e_mod_keybindings.h \
		    ng_config.c \
		    ng_item.c \
		    ng_box.h \
		    ng_box.c \
		    ng_winlist.c \
		    ng_winlist.h

module_la_LIBADD = @E_LIBS@
module_la_LDFLAGS = -module -avoid-version
module_la_DEPENDENCIES = $(top_builddir)/config.h

clean-local:
	 rm -rf *~
