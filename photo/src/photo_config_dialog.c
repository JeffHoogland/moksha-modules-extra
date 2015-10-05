#include "Photo.h"

struct _E_Config_Dialog_Data
{
   Evas_Object *ilist_local_dirs;
   Evas_Object *button_local_load_start;
   Evas_Object *button_local_load_stop;
   Evas_Object *textblock_local_infos;

   int show_label;
   int nice_trans;

   int pictures_from;
   int pictures_set_bg_purge;
   char *pictures_viewer;
   int pictures_thumb_size;
   struct
   {
      int auto_reload;
      int popup;
   } local;

   int action_mouse_over;
   int action_mouse_left;
   int action_mouse_middle;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void         _cb_dir_list(void *data);
static void         _cb_dir_add(void *data, void *data2);
static void         _cb_dir_del(void *data, void *data2);
static void         _cb_dir_config(void *data, void *data2);
static void         _cb_dir_reload(void *data, void *data2);
static void         _cb_dir_stopload(void *data, void *data2);

/*
 * Public functions
 */

int  photo_config_dialog_show(void)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   char buf[4096];

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;
   
   snprintf(buf, sizeof(buf), "%s/e-module-photo.edj", e_module_dir_get(photo->module));
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("Photo Configuration"), "Photo", "_e_modules_photo_config_dialog", buf, 0, v, NULL);

   return 1;
}

void photo_config_dialog_hide(void)
{
   e_object_del(E_OBJECT(photo->config_dialog));
   photo->config_dialog = NULL;
}

void photo_config_dialog_refresh_local_infos(void)
{
   E_Config_Dialog_Data *cfdata;
   char buf[100];

   if (!photo->config_dialog) return;
   cfdata = photo->config_dialog->cfdata;

   snprintf(buf, sizeof(buf), "%d pictures loaded, %d to thumb",
            photo_picture_local_loaded_nb_get(),
            photo_picture_local_tothumb_nb_get());
   e_widget_textblock_plain_set(cfdata->textblock_local_infos, buf);
}

void photo_config_dialog_refresh_local_dirs(void)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l;
   Evas_Object *ilist_dirs;
   int pos;

   if (!photo->config_dialog) return;
   cfdata = photo->config_dialog->cfdata;

   ilist_dirs = cfdata->ilist_local_dirs;

   pos = e_widget_ilist_selected_get(ilist_dirs);
   e_widget_ilist_clear(ilist_dirs);

   for(l=photo->config->local.dirs; l; l=eina_list_next(l))
     {
        Evas_Object *ic;
        Picture_Local_Dir *dir;
        char buf[1024];
        
        dir = eina_list_data_get(l);

        ic = e_icon_add(evas_object_evas_get(ilist_dirs));
        switch(dir->state)
          {
          case PICTURE_LOCAL_DIR_NOT_LOADED:
             photo_util_icon_set(ic, PHOTO_THEME_ICON_DIR_NOT_LOADED);
             break;
          case PICTURE_LOCAL_DIR_LOADING:
             photo_util_icon_set(ic, PHOTO_THEME_ICON_DIR_LOADING);
             break;
          case PICTURE_LOCAL_DIR_LOADED:
             photo_util_icon_set(ic, PHOTO_THEME_ICON_DIR_LOADED);
             break;
          }

        if (!dir->recursive)
          snprintf(buf, sizeof(buf), "%s", dir->path);
        else
          snprintf(buf, sizeof(buf), "%s [recursive]", dir->path);

        e_widget_ilist_append(ilist_dirs, ic, buf, _cb_dir_list, cfdata, NULL);
     }

   if (!eina_list_count(photo->config->local.dirs))
     e_widget_size_min_set(ilist_dirs, 165, 120);
   else
     {
        int wmw, wmh;
        e_widget_size_min_get(ilist_dirs, &wmw, &wmh);
        e_widget_size_min_set(ilist_dirs, wmw, 120);
     }
   
   e_widget_ilist_go(ilist_dirs);
   e_widget_ilist_selected_set(ilist_dirs, pos);
}

void photo_config_dialog_refresh_local_load(void)
{
   E_Config_Dialog_Data *cfdata;

   if (!photo->config_dialog) return;
   cfdata = photo->config_dialog->cfdata;

   if (photo_picture_local_load_state_get())
     {
        e_widget_disabled_set(cfdata->button_local_load_start, 1);
        e_widget_disabled_set(cfdata->button_local_load_stop, 0);
     }
   else
     {
        e_widget_disabled_set(cfdata->button_local_load_start, 0);
        e_widget_disabled_set(cfdata->button_local_load_stop, 1);
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

   photo->config_dialog = cfd;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   cfd->cfdata = cfdata;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   free(cfdata->pictures_viewer);
   photo->config_dialog = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   cfdata->show_label = photo->config->show_label;
   cfdata->nice_trans = photo->config->nice_trans;

   cfdata->pictures_from = photo->config->pictures_from;
   cfdata->pictures_set_bg_purge = photo->config->pictures_set_bg_purge;
   cfdata->pictures_viewer = strdup(photo->config->pictures_viewer);
   cfdata->pictures_thumb_size = photo->config->pictures_thumb_size;
   cfdata->local.auto_reload = photo->config->local.auto_reload;
   cfdata->local.popup = photo->config->local.popup;

   cfdata->action_mouse_over = photo->config->action_mouse_over;
   cfdata->action_mouse_left = photo->config->action_mouse_left;
   cfdata->action_mouse_middle = photo->config->action_mouse_middle;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ob;
   E_Radio_Group *rg;
   int wmw, wmh;
   
   o = e_widget_table_add(evas, 0);

   of = e_widget_frametable_add(evas, D_("Picture directories"), 0);

   ob = e_widget_textblock_add(evas);
   cfdata->textblock_local_infos = ob;
   photo_config_dialog_refresh_local_infos();
   e_widget_size_min_get(ob, &wmw, &wmh);
   e_widget_size_min_set(ob, wmw, 40);
   e_widget_frametable_object_append(of, ob, 0, 0, 3, 1, 1, 1, 1, 1);

   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_selector_set(ob, 1);
   cfdata->ilist_local_dirs = ob;
   photo_config_dialog_refresh_local_dirs();
   e_widget_frametable_object_append(of, ob, 0, 1, 3, 1, 1, 1, 1, 1);

   ob = e_widget_button_add(evas, D_("Add"), NULL, _cb_dir_add, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_button_add(evas, D_("Delete"), NULL, _cb_dir_del, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_button_add(evas, D_("Configure"), NULL, _cb_dir_config, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 2, 2, 1, 1, 1, 1, 1, 1);

   ob = e_widget_button_add(evas, D_("Reload picture directories"), NULL, _cb_dir_reload, cfdata, NULL);
   cfdata->button_local_load_start = ob;
   e_widget_frametable_object_append(of, ob, 0, 3, 3, 1, 1, 1, 1, 1);
   ob = e_widget_button_add(evas, D_("Stop loading"), NULL, _cb_dir_stopload, cfdata, NULL);
   cfdata->button_local_load_stop = ob;
   e_widget_frametable_object_append(of, ob, 0, 4, 3, 1, 1, 1, 1, 1);
   photo_config_dialog_refresh_local_load();

   e_widget_table_object_append(o, of, 0, 0, 1, 1, 1, 1, 1, 1);

   of = e_widget_frametable_add(evas, D_("Appearance"), 0);

   ob = e_widget_check_add(evas, D_("Always show label"), &(cfdata->show_label));
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, D_("Nice transitions"), &(cfdata->nice_trans));
   e_widget_frametable_object_append(of, ob, 0, 1, 2, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 0, 1, 1, 1, 1, 1, 1, 1);


   of = e_widget_frametable_add(evas, D_("Default Mouse actions"), 0);

   ob = e_widget_label_add(evas, D_("Over"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   CONFIG_DIALOG_ACTION_CHOICE(of, 0, 2, cfdata->action_mouse_over, ITEM_ACTION_NO);

   ob = e_widget_label_add(evas, D_("Left"));
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, D_("click"));
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);
   CONFIG_DIALOG_ACTION_CHOICE(of, 1, 2, cfdata->action_mouse_left, ITEM_ACTION_NO);

   ob = e_widget_label_add(evas, D_("Middle"));
   e_widget_frametable_object_append(of, ob, 2, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, D_("click"));
   e_widget_frametable_object_append(of, ob, 2, 1, 1, 1, 1, 1, 1, 1);
   CONFIG_DIALOG_ACTION_CHOICE(of, 2, 2, cfdata->action_mouse_middle, ITEM_ACTION_NO);
   CONFIG_DIALOG_ACTION_CHOICE_LABELS(of, 3, 2, D_("No"));


   e_widget_table_object_append(o, of, 1, 0, 1, 1, 1, 1, 1, 1);

   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if ( (photo->config->show_label != cfdata->show_label ) ||
        (photo->config->action_mouse_over != cfdata->action_mouse_over) ||
        (photo->config->action_mouse_left != cfdata->action_mouse_left) ||
        (photo->config->action_mouse_middle != cfdata->action_mouse_middle) )
     {
        photo->config->show_label = cfdata->show_label;

        photo->config->action_mouse_over = cfdata->action_mouse_over;
        photo->config->action_mouse_left = cfdata->action_mouse_left;
        photo->config->action_mouse_middle = cfdata->action_mouse_middle;

        photo_item_label_mode_set(NULL);
     }

   photo->config->nice_trans = cfdata->nice_trans;

   photo_config_save();
   return 1;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ob;
   E_Radio_Group *rg;

   o = e_widget_table_add(evas, 0);

   of = e_widget_frametable_add(evas, D_("Popups"), 0);

   ob = e_widget_label_add(evas, D_("Picture loader popup"));
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 1, 1, 1, 1, 1);
   rg = e_widget_radio_group_new(&(cfdata->local.popup));
   ob = e_widget_radio_add(evas, D_("Never"), PICTURE_LOCAL_POPUP_NEVER, rg);
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, D_("Summary"), PICTURE_LOCAL_POPUP_SUM, rg);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, D_("Verbose"), PICTURE_LOCAL_POPUP_ALWAYS, rg);
   e_widget_frametable_object_append(of, ob, 2, 1, 1, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 0, 0, 1, 1, 1, 1, 1, 1);

   of = e_widget_frametable_add(evas, D_("Miscellaneous"), 0);

   ob = e_widget_check_add(evas, D_("Remove generated backgrounds"),
                           &(cfdata->pictures_set_bg_purge));
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 1, 1, 1, 1, 1);

   ob = e_widget_label_add(evas, D_("Pictures viewer"));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_entry_add(evas, &(cfdata->pictures_viewer), NULL, NULL, NULL);
   e_widget_size_min_set(ob, 80, 25);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);

   ob = e_widget_label_add(evas, D_("Images Quality"));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f pix"),
                            (float)PICTURE_THUMB_SIZE_MIN,
                            (float)PICTURE_THUMB_SIZE_MAX,
                            100.0, 0,
                            NULL, &(cfdata->pictures_thumb_size), 80);
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 0, 1, 0);

   ob = e_widget_check_add(evas, D_("Automaticly reload directories"),
                           &(cfdata->local.auto_reload));
   e_widget_frametable_object_append(of, ob, 0, 3, 2, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, D_("when add / modify directory list"));
   e_widget_frametable_object_append(of, ob, 0, 4, 2, 1, 1, 1, 1, 1);


   e_widget_table_object_append(o, of, 0, 1, 1, 1, 1, 1, 1, 1);

   return o;
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   photo->config->pictures_set_bg_purge = cfdata->pictures_set_bg_purge;
   if (photo->config->pictures_viewer)
     eina_stringshare_del(photo->config->pictures_viewer);
   photo->config->pictures_viewer = eina_stringshare_add(cfdata->pictures_viewer);
   photo->config->pictures_thumb_size = cfdata->pictures_thumb_size;

   photo->config->local.auto_reload = cfdata->local.auto_reload;
   photo->config->local.popup = cfdata->local.popup;

   photo_config_save();
   return 1;
}



static void
_cb_dir_list(void *data)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
}

static void
_cb_dir_add(void *data, void *data2)
{
   if (photo->config_dialog_adddir) return;

   photo_config_dialog_dir_show(NULL);
}

static void
_cb_dir_del(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   Picture_Local_Dir *dir;

   cfdata = data;
   dir = eina_list_nth(photo->config->local.dirs,
                       e_widget_ilist_selected_get(cfdata->ilist_local_dirs));
   if (!dir || (dir->state == PICTURE_LOCAL_DIR_LOADING))
     return;

   photo->config->local.dirs = eina_list_remove(photo->config->local.dirs, dir);
   photo_picture_local_dir_free(dir, 1);

   photo_config_save();

   if (photo->config->local.auto_reload)
     photo_picture_local_load_start();
   else
     photo_config_dialog_refresh_local_dirs();
}

static void
_cb_dir_config(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   Picture_Local_Dir *dir;

   cfdata = data;
   dir = eina_list_nth(photo->config->local.dirs,
                       e_widget_ilist_selected_get(cfdata->ilist_local_dirs));

   if (!dir ||
       (dir->state == PICTURE_LOCAL_DIR_LOADING) ||
       dir->config_dialog)
     return;

   photo_config_dialog_dir_show(dir);
}

static void
_cb_dir_reload(void *data, void *data2)
{
   photo_picture_local_load_start();
}

static void
_cb_dir_stopload(void *data, void *data2)
{
   photo_picture_local_load_stop();
}
