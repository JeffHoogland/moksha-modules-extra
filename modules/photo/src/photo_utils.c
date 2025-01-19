#include "Photo.h"

char * _xdg_pictures_dir();

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

char*
photo_util_pictures_dir()
{
   // If this dir was a compile option and exits use it
   if (strlen(PHOTO_LOCAL_DIR) && ecore_file_is_dir(PHOTO_LOCAL_DIR))
      return strdup(PHOTO_LOCAL_DIR);
   // Otherwise try XDG_PICTURES_DIR
   char *util_dir = _xdg_pictures_dir();
   if (util_dir && ecore_file_is_dir(util_dir))
      return util_dir;
   // XDG variable is not set so return something
   char tmp_path[PATH_MAX];
   e_user_homedir_concat(tmp_path, sizeof(tmp_path), "Pictures");
   if (ecore_file_is_dir(tmp_path))
      return strdup(tmp_path);
   // No ~/Pictures folder exist so use reasonable defaults
   if (ecore_file_is_dir("/usr/share/backgrounds/bodhi"))
      return strdup("/usr/share/backgrounds/bodhi");
   else if (ecore_file_is_dir("/usr/share/backgrounds/"))
      return strdup("/usr/share/backgrounds/");
   else
      return strdup("/tmp");
}

/*
 * Private functions
 *
 */

char *
_xdg_pictures_dir()
{
   char *env_dir = getenv("XDG_PICTURES_DIR");
   if (env_dir && env_dir[0] != '\0')
      // Environment variable is set
       return strdup(env_dir);
   // Expand the path to the configuration file
   char config_path[PATH_MAX];
   e_user_homedir_concat(config_path, sizeof(config_path), ".config/user-dirs.dirs");
   // Open the configuration file
   FILE *file = fopen(config_path, "r");
   if (!file)
     {
        DPICL(("Failed to open configuration file"));
        return NULL;
     }

   // Read the file line by line
   char line[PATH_MAX];
   while (fgets(line, sizeof(line), file))
     {
        // Look for the XDG_PICTURES_DIR entry
        if (strncmp(line, "XDG_PICTURES_DIR", 16) == 0)
          {
             fclose(file);

             // Extract the path (it is usually in the form XDG_PICTURES_DIR="$HOME/Pictures")
             char *start = strchr(line, '"');
             if (start)
               {
                 char *end = strchr(start + 1, '"');
                 if (end)
                   {
                      *end = '\0'; // Null-terminate the string
                      char *resolved_path = strdup(start + 1);

                      // Expand $HOME if present
                      if (strncmp(resolved_path, "$HOME", 5) == 0)
                        {
                           char *home = getenv("HOME");
                            if (home)
                             {
                                size_t len = strlen(home) + strlen(resolved_path) - 5 + 1;
                                char *full_path = malloc(len);
                                snprintf(full_path, len, "%s%s", home, resolved_path + 5);
                                free(resolved_path);
                                DPICL(("Picture XDG_PICTURES_DIR: %s", full_path));
                                return full_path;
                             }
                        }
                      // $HOME is not in string
                      DPICL(("Picture XDG_PICTURES_DIR: %s", resolved_path));
                      return resolved_path;
                   }
               }
          }
     }
   fclose(file);
   return NULL;
}
