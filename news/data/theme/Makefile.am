ACLOCAL_AMFLAGS = -I m4
MAINTAINERCLEANFILES = Makefile.in

EDJE_FLAGS = -v \
	-id $(top_srcdir)/data/theme/images

filesdir = $(datadir)
files_DATA = e-module-news.edj

EXTRA_DIST = \
module.edc \
module_feed.edc \
module_feed_parts.edc \
module_feed_programs.edc \
module_feedone.edc \
module_feedone_parts.edc \
module_feedone_programs.edc \
module_icon.edc \
module_item.edc \
module_item_parts.edc \
module_popw.edc \
module_popw_parts.edc \
module_popw_programs.edc \
images/feed_background_new.png \
images/feed_border.png \
images/icon_article_read.png \
images/icon_article_unread.png \
images/icon_logo.png \
images/icon_setasread.png \
images/icon_update.png \
images/popup_warn_background.png \
images/popup_warn_disable_clicked.png \
images/popup_warn_disable.png \
images/popup_warn_logo_error.png \
images/popup_warn_logo_info.png

e-module-news.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/theme/module.edc \
	$(top_builddir)/data/theme/e-module-news.edj

clean-local:
	rm -f e-module-news.edj
