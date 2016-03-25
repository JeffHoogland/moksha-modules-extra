MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I. \
	   -I$(top_srcdir) \
	   -I$(includedir) \
	   @E_CFLAGS@

pkgdir = $(datadir)/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = e_mod_main.h \
		    news_config_dialog_category.h \
		    news_config_dialog_feed.h \
		    news_config_dialog_feeds.h \
		    news_config_dialog.h \
		    news_config_dialog_item_content.h \
		    news_config_dialog_item.h \
		    news_config_dialog_langs.h \
		    news_config.h \
		    news_feed.h \
		    News.h \
		    News_includes.h \
		    news_item.h \
		    news_menu.h \
		    news_parse.h \
		    news_popup.h \
		    news_theme.h \
		    news_utils.h \
		    news_viewer.h \
		    e_mod_main.c \
		    news_config.c \
		    news_config_dialog.c \
		    news_config_dialog_category.c \
		    news_config_dialog_feed.c \
		    news_config_dialog_feeds.c \
		    news_config_dialog_item.c \
		    news_config_dialog_item_content.c \
		    news_config_dialog_langs.c \
		    news_feed.c \
		    news_item.c \
		    news_menu.c \
		    news_parse.c \
		    news_popup.c \
		    news_theme.c \
		    news_utils.c \
		    news_viewer.c

module_la_LIBADD = @E_LIBS@
module_la_LDFLAGS = -module -avoid-version -s
module_la_DEPENDENCIES = $(top_builddir)/config.h

clean-local:
	rm -rf *~
