MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS_VERBOSE_ =
EDJE_FLAGS_VERBOSE_0 =
EDJE_FLAGS_VERBOSE_1 = -v
EDJE_FLAGS = $(EDJE_FLAGS_$(V)) -id $(top_srcdir)/data/theme/images -fd $(top_srcdir)/data/theme/fonts

filesdir = $(datadir)/etrophy/

files_DATA = etrophy.edj

EXTRA_DIST = \
	etrophy.edc

etrophy.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/theme/etrophy.edc \
	$(top_builddir)/data/theme/etrophy.edj

clean-local:
	rm -f *.edj
