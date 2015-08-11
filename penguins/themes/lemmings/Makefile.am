THEME = lemmings
EDC = $(THEME).edc
EDJ = $(THEME).edj
SUFFIXES = .edc .edj
MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = $(EDJ)

pkgdatadir = $(datadir)
themedir = $(pkgdatadir)/themes
theme_DATA = $(EDJ)

all: $(EDJ)

.edc.edj:
	$(EDJE_CC) -v -id $(top_srcdir)/themes/$(THEME) -fd $(top_srcdir)/themes/$(THEME) $< $@

EXTRA_DIST = $(EDC) \
	     $(wildcard *.png)

dist-hook:
	rm -rf `find $(distdir) -name CVS`
