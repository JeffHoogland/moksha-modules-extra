MAINTAINERCLEANFILES = Makefile.in e_modules-mem.spec

INCLUDES = -I. \
	   -I$(top_srcdir) \
	   -I$(includedir) \
	   @E_CFLAGS@

pkgdir = $(datadir)/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
if BUILD_FREEBSD
module_la_SOURCES = e_mod_main.c \
		    e_mod_main.h \
		    machdep_freebsd.c \
		    e_mod_config.c
endif
if BUILD_LINUX
module_la_SOURCES = e_mod_main.c \
		    e_mod_main.h \
		    machdep_linux.c \
		    e_mod_config.c
endif

EXTRA_module_la_SOURCES	= machdep_linux.c \
			  machdep_freebsd.c

module_la_LIBADD       = @E_LIBS@ @machdep_libs@
module_la_LDFLAGS      = -module -avoid-version
module_la_DEPENDENCIES = $(top_builddir)/config.h

clean-local:
	 rm -rf *~
