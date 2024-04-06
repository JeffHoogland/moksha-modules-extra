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

Eina_Bool
photo_util_image_size(const char *path, int *w, int *h)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((path || w || h), EINA_FALSE);
   Eina_Bool ret = EINA_TRUE;
   Ecore_Evas *ee = ecore_evas_buffer_new(100, 100);
   Evas *evas = ecore_evas_get(ee);
   Evas_Object *img;
   int err;

   img = evas_object_image_add(evas);
   evas_object_image_file_set(img, path, NULL);
   err = evas_object_image_load_error_get(img);
   if (err == EVAS_LOAD_ERROR_NONE)
      evas_object_image_size_get(img, w, h);
   else
     {
         DPICL(("Picture load error: %d", err));
         ret = EINA_FALSE;
      }
   evas_object_del(img);
   return ret;
}

/*
 * Private functions
 *
 */
