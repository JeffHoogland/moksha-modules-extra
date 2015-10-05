#include "Photo.h"


/*
 * Public functions
 */

void photo_util_edje_set(Evas_Object *obj, char *key)
{
   if (!photo->theme)
     e_theme_edje_object_set(obj, PHOTO_THEME_IN_E, key);
   else
     edje_object_file_set(obj, photo->theme, key);
}

void photo_util_icon_set(Evas_Object *ic, char *key)
{
   if (!photo->theme)
     e_util_edje_icon_set(ic, key);
   else
     e_icon_file_edje_set(ic, photo->theme, key);
}

void photo_util_menu_icon_set(E_Menu_Item *mi, char *key)
{
   if (!photo->theme)
     e_util_menu_item_theme_icon_set(mi, key);
   else
     e_menu_item_icon_edje_set(mi, photo->theme, key);
}

/*
 * Private functions
 *
 */
