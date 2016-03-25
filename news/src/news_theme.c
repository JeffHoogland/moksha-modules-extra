#include "News.h"

static char _theme[4096];

/*
 * Public functions
 */

const char *
news_theme_file_get(char *category)
{
   if (category && e_theme_category_find(category))
     return NULL;
   snprintf(_theme, sizeof(_theme), "%s/e-module-news.edj", e_module_dir_get(news->module));
   return _theme;
}

void
news_theme_edje_set(Evas_Object *obj, char *key)
{
   if (!e_theme_edje_object_set(obj, NEWS_THEME_IN_E, key))
     edje_object_file_set(obj, news_theme_file_get(NULL), key);
}

void
news_theme_icon_set(Evas_Object *ic, char *key)
{
   if (!e_util_edje_icon_set(ic, key))
      e_icon_file_edje_set(ic, news_theme_file_get(NULL), key);
}

void
news_theme_menu_icon_set(E_Menu_Item *mi, char *key)
{
   if (!e_util_menu_item_theme_icon_set(mi, key))
     e_menu_item_icon_edje_set(mi, news_theme_file_get(NULL), key);
}

/*
 * Private functions
 */
