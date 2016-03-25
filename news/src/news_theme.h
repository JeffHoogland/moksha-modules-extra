#ifdef E_MOD_NEWS_TYPEDEFS

#else

#ifndef NEWS_THEME_H_INCLUDED
#define NEWS_THEME_H_INCLUDED

#define NEWS_THEME_IN_E "base/theme/modules/news"

#define NEWS_THEME_CAT_ICON "modules/news/icon"

#define NEWS_THEME_ITEM "modules/news/item"
#define NEWS_THEME_FEED "modules/news/feed"
#define NEWS_THEME_FEEDONE "modules/news/feedone"
#define NEWS_THEME_ARTICLE "modules/news/article"
#define NEWS_THEME_VIEWER "modules/news/viewer"
#define NEWS_THEME_POPW "modules/news/popw"
#define NEWS_THEME_ICON_LOGO "modules/news/icon/logo"
#define NEWS_THEME_ICON_INFOS "modules/news/icon/infos"
#define NEWS_THEME_ICON_VIEWER "modules/news/icon/viewer"
#define NEWS_THEME_ICON_ARTICLE_READ "modules/news/icon/article_read"
#define NEWS_THEME_ICON_ARTICLE_UNREAD "modules/news/icon/article_unread"
#define NEWS_THEME_ICON_UPDATE "modules/news/icon/update"
#define NEWS_THEME_ICON_SETASREAD "modules/news/icon/setasread"

const char * news_theme_file_get(char *category);
void news_theme_edje_set(Evas_Object *obj, char *key);
void news_theme_icon_set(Evas_Object *ic, char *key);
void news_theme_menu_icon_set(E_Menu_Item *mi, char *key);

#endif
#endif
