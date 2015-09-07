MAINTAINERCLEANFILES = Makefile.in
ACLOCAL_AMFLAGS = -I m4

INCLUDES = -I. \
	 -I$(top_srcdir) \
	 @EINA_CFLAGS@ @ECORE_CFLAGS@ @EVAS_CFLAGS@

bin_PROGRAMS = emprint

emprint_SOURCES = emprint.h main.c

emprint_LDADD = @EINA_LIBS@ @ECORE_LIBS@ @EVAS_LIBS@
