#include "Photo.h"

struct _E_Config_Dialog_Data
{
   Picture_Local_Dir *dir;

   char *path;
   int recursive;
   int read_hidden;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata, Picture_Local_Dir *dir);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);


/*
 * Public functions
 */

int  photo_config_dialog_dir_show(Picture_Local_Dir *dir)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   char buf[4096];

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   
   snprintf(buf, sizeof(buf), "%s/e-module-photo.edj", e_module_dir_get(photo->module));
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("Photo Directory Configuration"), "Photo", "_e_modules_photo_dir_config_dialog", buf, 0, v, dir);

   if (dir) dir->config_dialog = cfd;
   else photo->config_dialog_adddir = cfd;

   return 1;
}

void photo_config_dialog_dir_hide(Picture_Local_Dir *dir)
{
   if (dir)
     {
        e_object_del(E_OBJECT(dir->config_dialog));
        dir->config_dialog = NULL;
     }
   else
     {
        e_object_del(E_OBJECT(photo->config_dialog_adddir));
        photo->config_dialog_adddir = NULL;
     }
}


/*
 * Private functions
 *
 */

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   Picture_Local_Dir *dir;

   dir = cfd->data;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata, dir);
   cfd->cfdata = cfdata;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   free(cfdata->path);
   if (cfdata->dir)
     cfdata->dir->config_dialog = NULL;
   else
     photo->config_dialog_adddir = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata, Picture_Local_Dir *dir)
{
   cfdata->dir = dir;

   if (dir)
     {
        cfdata->path = strdup(dir->path);
        cfdata->recursive = dir->recursive;
        cfdata->read_hidden = dir->read_hidden;
     }
   else
     {
        cfdata->path = strdup("");
        cfdata->recursive = PICTURE_LOCAL_DIR_RECURSIVE_DEFAULT;
        cfdata->read_hidden = PICTURE_LOCAL_DIR_READ_HIDDEN_DEFAULT;
     }
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o;
   Evas_Object *of, *ob;
   
   o = e_widget_table_add(evas, 0);

   of = e_widget_frametable_add(evas, D_("Directory"), 0);

   ob = e_widget_entry_add(evas, &(cfdata->path), NULL, NULL, NULL);
   e_widget_size_min_set(ob, 250, 25);
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 0, 0, 1, 1, 1, 1, 1, 1);


   of = e_widget_frametable_add(evas, D_("Loading options"), 0);

   ob = e_widget_check_add(evas, D_("Recursive"), &(cfdata->recursive));
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, D_("Read hidden files"), &(cfdata->read_hidden));
   e_widget_frametable_object_append(of, ob, 0, 1, 2, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 0, 1, 1, 1, 1, 1, 1, 1);

   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   Picture_Local_Dir *dir;
   char buf[4096];

   dir = photo_picture_local_dir_new(cfdata->path,
                                     cfdata->recursive, cfdata->read_hidden);
   if (!dir)
     return 0;

   if(cfdata->dir)
     {
        if (cfdata->dir->state == PICTURE_LOCAL_DIR_LOADING)
          {
             snprintf(buf, sizeof(buf),
                      "<hilight>The directory %s is being loaded at the moment !</hilight><br>"
                      "Please wait until the loading is finished to modify its settings",
                      cfdata->dir->path);
             e_module_dialog_show(photo->module, D_("Photo Module Warning"), buf);
             return 0;
          }

        photo->config->local.dirs = eina_list_remove(photo->config->local.dirs,
                                                     cfdata->dir);
        photo_picture_local_dir_free(cfdata->dir, 0);
     }
   else
     {
        photo->config_dialog_adddir = NULL;
        dir->config_dialog = cfd;
     }

   photo->config->local.dirs = eina_list_append(photo->config->local.dirs, dir);
   cfdata->dir = dir;

   if (photo->config->local.auto_reload)
     photo_picture_local_load_start();

   if (photo->config_dialog)
     photo_config_dialog_refresh_local_dirs();

   photo_config_save();
   return 1;
}
