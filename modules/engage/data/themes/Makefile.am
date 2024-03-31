MAINTAINERCLEANFILES = Makefile.in

EDJE_FLAGS = -v -id $(top_srcdir)/data/themes -fd $(top_srcdir)/data/themes

filesdir = $(datadir)
files_DATA = \
engage.edj


EXTRA_DIST = $(files_DATA)

%.edj:  %.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@

clean-local:
	rm -f *.edj


