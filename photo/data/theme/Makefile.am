MAINTAINERCLEANFILES = Makefile.in

EDJE_FLAGS = -v \
	-id $(top_srcdir)/data/theme/images

EXTRA_DIST = \
photo.edc \
photo_item.edc \
photo_item_parts.edc \
photo_item_programs.edc \
photo_popi.edc \
photo_popi_parts.edc \
photo_popi_programs.edc \
photo_popw.edc \
photo_popw_parts.edc \
photo_popw_programs.edc \
photo_icon.edc \
$(wildcard images/*.png)

filesdir = $(datadir)
files_DATA = photo.edj

photo.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/theme/photo.edc \
	$(top_builddir)/data/theme/photo.edj

clean-local:
	rm -f photo.edj
