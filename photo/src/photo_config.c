#include "Photo.h"

static E_Config_DD *_photo_edd = NULL;
static E_Config_DD *_photo_dir_edd = NULL;
static E_Config_DD *_photo_item_edd = NULL;


/*
 * Public functions
 */

int photo_config_init(void)
{
   Eina_List *l;
   char buf[4096];
	
   _photo_dir_edd = E_CONFIG_DD_NEW("Photo_Local_Dir", Picture_Local_Dir);
#undef T
#undef D
#define T Picture_Local_Dir
#define D _photo_dir_edd
   E_CONFIG_VAL(D, T, path, STR);
   E_CONFIG_VAL(D, T, recursive, SHORT);
   E_CONFIG_VAL(D, T, read_hidden, SHORT);
   _photo_item_edd = E_CONFIG_DD_NEW("Photo_Item", Photo_Config_Item);
#undef T
#undef D
#define T Photo_Config_Item
#define D _photo_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, timer_s, INT);
   E_CONFIG_VAL(D, T, timer_active, SHORT);
   E_CONFIG_VAL(D, T, alpha, SHORT);
   E_CONFIG_VAL(D, T, show_label, SHORT);
   E_CONFIG_VAL(D, T, action_mouse_over, SHORT);
   E_CONFIG_VAL(D, T, action_mouse_left, SHORT);
   E_CONFIG_VAL(D, T, action_mouse_middle, SHORT);
   _photo_edd = E_CONFIG_DD_NEW("Photo", Photo_Config);
#undef T
#undef D
#define T Photo_Config
#define D _photo_edd
   E_CONFIG_VAL(D, T, version, SHORT);
   E_CONFIG_VAL(D, T, show_label, SHORT);
   E_CONFIG_VAL(D, T, nice_trans, SHORT);
   E_CONFIG_VAL(D, T, pictures_from, SHORT);
   E_CONFIG_VAL(D, T, pictures_set_bg_purge, SHORT);
   E_CONFIG_VAL(D, T, pictures_viewer, STR);
   E_CONFIG_VAL(D, T, pictures_thumb_size, SHORT);
   E_CONFIG_LIST(D, T, local.dirs, _photo_dir_edd);
   E_CONFIG_VAL(D, T, local.auto_reload, SHORT);
   E_CONFIG_VAL(D, T, local.popup, SHORT);
   E_CONFIG_VAL(D, T, local.thumb_msg, SHORT);
   E_CONFIG_VAL(D, T, action_mouse_over, SHORT);
   E_CONFIG_VAL(D, T, action_mouse_left, SHORT);
   E_CONFIG_VAL(D, T, action_mouse_middle, SHORT);
   E_CONFIG_LIST(D, T, items, _photo_item_edd);
   photo->config = e_config_domain_load("module.photo", _photo_edd);

   if (photo->config)
     {
        if (photo->config->version < CONFIG_VERSION)
          {
             snprintf(buf, sizeof(buf),
                      D_("<hilight>Photo module : Configuration Upgraded</hilight><br><br>"
                        "Your configuration of photo module<br>"
                        "has been upgraded<br>"
                        "Your settings were removed<br>"
                        "Sorry for the inconvenience<br><br>"
                        "(%d -> %d)"), photo->config->version, CONFIG_VERSION);
             e_module_dialog_show(photo->module, D_("Photo Module version " MODULE_VERSION), buf);
             photo->config = NULL;
          }
        else
          {
             if (photo->config->version > CONFIG_VERSION)
               {
                  snprintf(buf, sizeof(buf),
                           D_("<hilight>Photo module : Configuration Downgraded</hilight><br><br>"
                             "Your configuration of Photo module<br>"
                             "has been downgraded<br>"
                             "Your settings were removed<br>"
                             "Sorry for the inconvenience<br><br>"
                             "(%d ->%d)"), photo->config->version, CONFIG_VERSION);
                  e_module_dialog_show(photo->module, D_("Photo Module version " MODULE_VERSION), buf);
                  photo->config = NULL;
               }
          }
     }

   if (!photo->config)
     {
        Photo_Config *c;
        c = E_NEW(Photo_Config, 1);
        c->version = CONFIG_VERSION;
        c->show_label = ITEM_SHOW_LABEL_DEFAULT;
        c->nice_trans = ITEM_NICE_TRANS_DEFAULT;
        c->pictures_from = PICTURE_FROM_DEFAULT;
        c->pictures_set_bg_purge = PICTURE_SET_BG_PURGE_DEFAULT;
        c->pictures_viewer = eina_stringshare_add(PICTURE_VIEWER_DEFAULT);
        c->pictures_thumb_size = PICTURE_THUMB_SIZE_DEFAULT;
        c->local.dirs = eina_list_append(c->local.dirs,
                                         photo_picture_local_dir_new((char *)e_module_dir_get(photo->module),
                                                                     1, 0));
        c->local.auto_reload = PICTURE_LOCAL_AUTO_RELOAD_DEFAULT;
        c->local.popup = PICTURE_LOCAL_POPUP_DEFAULT;
        c->local.thumb_msg = 1;
        c->action_mouse_over = ITEM_MOUSE_OVER_ACTION_DEFAULT;
        c->action_mouse_left = ITEM_MOUSE_LEFT_ACTION_DEFAULT;
        c->action_mouse_middle = ITEM_MOUSE_MIDDLE_ACTION_DEFAULT;
        photo->config = c;
     }

   E_CONFIG_LIMIT(photo->config->show_label, 0, 1);
   E_CONFIG_LIMIT(photo->config->nice_trans, 0, 1);
   E_CONFIG_LIMIT(photo->config->pictures_from, 0, 2);
   E_CONFIG_LIMIT(photo->config->local.auto_reload, 0, 1);
   E_CONFIG_LIMIT(photo->config->local.popup, 0, 2);
   E_CONFIG_LIMIT(photo->config->local.thumb_msg, 0, 1);
   E_CONFIG_LIMIT(photo->config->pictures_set_bg_purge, 0, 1);

   for (l=photo->config->local.dirs; l; l=eina_list_next(l))
     {
        Picture_Local_Dir *dir;
        dir = eina_list_data_get(l);

        E_CONFIG_LIMIT(dir->recursive, 0, 1);
        E_CONFIG_LIMIT(dir->read_hidden, 0, 1);
        dir->state = PICTURE_LOCAL_DIR_NOT_LOADED;
        dir->config_dialog = NULL;
     }

   for (l=photo->config->items; l; l=eina_list_next(l))
     {
        Photo_Config_Item *pic;
        pic = eina_list_data_get(l);

        E_CONFIG_LIMIT(pic->timer_s, ITEM_TIMER_S_MIN, ITEM_TIMER_S_MAX);
        E_CONFIG_LIMIT(pic->timer_active, 0, 1);
        E_CONFIG_LIMIT(pic->alpha, 0, 255);
        E_CONFIG_LIMIT(pic->show_label, ITEM_SHOW_LABEL_NO, ITEM_SHOW_LABEL_PARENT);
        E_CONFIG_LIMIT(pic->action_mouse_over, ITEM_ACTION_NO, ITEM_ACTION_PARENT);
        E_CONFIG_LIMIT(pic->action_mouse_left, ITEM_ACTION_NO, ITEM_ACTION_PARENT);
        E_CONFIG_LIMIT(pic->action_mouse_middle, ITEM_ACTION_NO, ITEM_ACTION_PARENT);
     }

   return 1;
}

int photo_config_shutdown(void)
{
   Photo_Config *c;
   Eina_List *l;

   c = photo->config;
   for (l=c->local.dirs; l; l=eina_list_next(l))
     {
        Picture_Local_Dir *d;
        d = eina_list_data_get(l);
        photo_picture_local_dir_free(d, 1);
     }
   eina_list_free(c->local.dirs);
   for (l=c->items; l; l=eina_list_next(l))
     {
        Photo_Config_Item *pic;
        pic = eina_list_data_get(l);
        photo_config_item_free(pic);
     }
   eina_list_free(c->items);

   E_FREE(photo->config);

   E_CONFIG_DD_FREE(_photo_edd);
   E_CONFIG_DD_FREE(_photo_dir_edd);
   E_CONFIG_DD_FREE(_photo_item_edd);

   return 1;
}

int photo_config_save(void)
{
   e_config_domain_save("module.photo", _photo_edd, photo->config);

   return 1;
}

Photo_Config_Item *photo_config_item_new(const char *id)
{
   Photo_Config_Item *pic;
   Eina_List *l;

   DCONF(("Item new config"));

   /* is there already an item config for this id ? */
   for (l=photo->config->items; l; l=eina_list_next(l))
     {
        pic = eina_list_data_get(l);
        if (!strcmp(pic->id, id))
          {
             DCONF(("config found ! %s", pic->id));
             return pic;
          }
     }

   DCONF(("config NOT found ! creating new one %s", id));
   /* no, create a new item config */
   pic = E_NEW(Photo_Config_Item, 1);
   pic->id = eina_stringshare_add(id);
   pic->timer_s = ITEM_TIMER_S_DEFAULT;
   pic->timer_active = ITEM_TIMER_ACTIVE_DEFAULT;
   pic->alpha = ITEM_ALPHA_DEFAULT;
   pic->show_label = ITEM_SHOW_LABEL_PARENT;
   pic->action_mouse_over = ITEM_ACTION_PARENT;
   pic->action_mouse_left = ITEM_ACTION_PARENT;
   pic->action_mouse_middle = ITEM_ACTION_PARENT;

   photo->config->items = eina_list_append(photo->config->items, pic);

   return pic;
}

void photo_config_item_free(Photo_Config_Item *pic)
{
   eina_stringshare_del(pic->id);

   photo->config->items = eina_list_remove(photo->config->items, pic);
   free(pic);
}


/*
 * Private functions
 *
 */
