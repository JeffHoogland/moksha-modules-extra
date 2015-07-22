MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. \
	   -I$(top_srcdir) \
	   -I$(includedir) \
	   @E_CFLAGS@ 

pkgdir = $(datadir)/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = e_mod_main.c \
		    e_mod_main.h \
		    ng_config.c \
		    ng_item.c \
		    ng_launcher.c \
		    ng_taskbar.c \
		    ng_config_instances.c \
		    ng_box.c \
		    ng_border_menu.c \
		    ng_gadcon.c

module_la_LIBADD = @E_LIBS@
module_la_LDFLAGS = -module -avoid-version
module_la_DEPENDENCIES = $(top_builddir)/config.h

clean-local:
	 rm -rf *~


