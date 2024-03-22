/*  Copyright (C) 2008-2012 Davide Andreoli (see AUTHORS)
 *
 *  This file is part of trash.
 *  trash is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  trash is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with trash.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <e.h>
#include "e_mod_main.h"
#include "Efreet.h"
#include "Efreet_Trash.h"

//~ #define FILE_MANAGER "pcmanfm"
// #define FILE_MANAGER "konqueror"
// #define FILE_MANAGER "nautilus --no-desktop"

static void _trash_dnd_cb_enter(void *data, const char *type, void *event_info);
static void _trash_dnd_cb_move(void *data, const char *type, void *event_info);
static void _trash_dnd_cb_leave(void *data, const char *type, void *event_info);
static void _trash_dnd_cb_drop(void *data, const char *type, void *event_info);

static void _trash_dialog_cb_ok(void *data);
static void _trash_dialog_cb_cancel(void *data);

static void _trash_button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _trash_cb_menu_post(void *data, E_Menu *menu);
static void _trash_cb_obj_moveresize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _trash_monitor_cb(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path);
static void _trash_cb_menu_empty(void *data, E_Menu *m, E_Menu_Item *mi);
static void _trash_cb_menu_show(void *data, E_Menu *m, E_Menu_Item *mi);
static void _trash_cb_menu_empty_ok(void *data);
static void _trash_cb_menu_configure(void *data, E_Menu *mn, E_Menu_Item *mi);
static void _trash_conf_new(void);
static Config_Item *_trash_conf_item_get(const char *id);
static void _trash_conf_free(void);

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static Eina_Bool _gc_in_site(E_Gadcon_Site site);


typedef struct _Instance Instance;
struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object     *o_trash;
   E_Drop_Handler  *drop_handler;
   E_Menu          *menu;
}; 


static const char *icon;
static Ecore_File_Monitor *monitor;
static Eina_List *instances;
static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;
Config *trash_conf = NULL;


/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class =
{
   GADCON_CLIENT_CLASS_VERSION,"trash",
     {
        _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon,
        _gc_id_new, NULL, _gc_in_site
     },
   E_GADCON_CLIENT_STYLE_PLAIN
};

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Instance *inst;
   E_Gadcon_Client *gcc;
   Evas_Object *o;
   Evas_Coord x, y, w, h;
   const char *drop[] = { "text/uri-list"};
   inst = E_NEW(Instance, 1);

   o = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(o, "base/theme/modules/trash", 
                                "modules/trash/main"))
     edje_object_file_set(o, icon, "modules/trash/main");

   if (!efreet_trash_is_empty())
      edje_object_signal_emit(o, "set_full", "e");

   evas_object_show(o);

   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   instances = eina_list_append(instances, inst);

   inst->gcc = gcc;
   inst->o_trash = o;

   /* Enable xdnd */
   evas_object_geometry_get(o, &x, &y, &w, &h);
   //e_drop_xdnd_register_set(gc->dnd_win, 1);
   inst->drop_handler = e_drop_handler_add(E_OBJECT(inst->gcc), inst,
                                           _trash_dnd_cb_enter, _trash_dnd_cb_move,
                                           _trash_dnd_cb_leave, _trash_dnd_cb_drop,
                                           drop, 1, x, y, w,  h);

   evas_object_event_callback_add(inst->o_trash, EVAS_CALLBACK_MOUSE_DOWN,
                                  _trash_button_cb_mouse_down, inst);
   evas_object_event_callback_add(inst->o_trash, EVAS_CALLBACK_MOVE,
                                  _trash_cb_obj_moveresize, inst);
   evas_object_event_callback_add(inst->o_trash, EVAS_CALLBACK_RESIZE,
                                  _trash_cb_obj_moveresize, inst);

   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   inst = gcc->data;
   if (inst->o_trash) evas_object_del(inst->o_trash);
   if (inst->drop_handler) e_drop_handler_del(inst->drop_handler);

   instances = eina_list_remove(instances, inst);
   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient __UNUSED__)
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   return D_("Trash");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class __UNUSED__, Evas *evas)
{
   Evas_Object *o;

   o = edje_object_add(evas);
   edje_object_file_set(o, icon, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   return _gadcon_class.name;
}

static Eina_Bool
_gc_in_site(E_Gadcon_Site site __UNUSED__)
{
   //return (site != E_GADCON_SITE_DESKTOP);
   return EINA_TRUE;
}


/* e module setup */
EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "Trash"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[4096];
   
   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   snprintf(buf, sizeof(buf), "%s/e-module-trash.edj", e_module_dir_get(m));
   icon = eina_stringshare_add(buf);

   //~ instances = NULL;
   e_configure_registry_category_add("advanced", 80, D_("Advanced"), 
                                     NULL, "preferences-advanced");
                                     
   e_configure_registry_item_add("advanced/trash", 110, D_("Trash"), 
                                 NULL, buf, e_int_config_trash_module);
                                 
   conf_item_edd = E_CONFIG_DD_NEW("Config_Item", Config_Item);
   #undef T
   #undef D
   #define T Config_Item
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, switch2, INT);

   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   //~ E_CONFIG_VAL(D, T, switch1, UCHAR); /* our var from header */
   E_CONFIG_VAL(D, T, fileman, STR); /* our var from header */
   E_CONFIG_LIST(D, T, conf_items, conf_item_edd); /* the list */

   trash_conf = e_config_domain_load("module.trash", conf_edd);
   if (trash_conf) 
     {
        /* Check config version */
        if ((trash_conf->version >> 16) < MOD_CONFIG_FILE_EPOCH) 
          {
             /* config too old */
            _trash_conf_free();
          }

        /* Ardvarks */
        else if (trash_conf->version > MOD_CONFIG_FILE_VERSION) 
          {
             /* config too new...wtf ? */
             _trash_conf_free();
          }
     }

   if (!trash_conf) _trash_conf_new();
   trash_conf->module = m;

   e_gadcon_provider_register(&_gadcon_class);
   snprintf(buf, sizeof(buf), "%s/files", efreet_trash_dir_get(NULL));
   monitor = ecore_file_monitor_add(buf, _trash_monitor_cb, NULL);

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   Instance *inst;
   e_configure_registry_item_del("advanced/trash");
   e_configure_registry_category_del("advanced");
   if (trash_conf->cfd) e_object_del(E_OBJECT(trash_conf->cfd));
   trash_conf->cfd = NULL;
   trash_conf->module = NULL;
   
   while (trash_conf->conf_items) 
     {
        Config_Item *ci = NULL;

        /* Grab an item from the list */
        ci = trash_conf->conf_items->data;

        /* remove it */
        trash_conf->conf_items = 
          eina_list_remove_list(trash_conf->conf_items, 
                                trash_conf->conf_items);

        /* cleanup stringshares */
        if (ci->id) eina_stringshare_del(ci->id);

        /* keep the planet green */
        E_FREE(ci);
     }

   if (monitor) ecore_file_monitor_del(monitor);
   e_gadcon_provider_unregister(&_gadcon_class);
   if (icon) eina_stringshare_del(icon);
   if (trash_conf->fileman) eina_stringshare_del(trash_conf->fileman);
   if (!instances) return 1;
  
   E_FREE(trash_conf);
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
  
   EINA_LIST_FREE(instances, inst)
     {
        if (inst->o_trash) evas_object_del(inst->o_trash);
        E_FREE(inst);
     }
   instances = NULL;
   return 1;
}

EAPI int
e_modapi_save(E_Module *m __UNUSED__)
{
   e_config_domain_save("module.trash", conf_edd, trash_conf);
   return 1;
}


/* callbacks */
static void
_trash_monitor_cb(void *data __UNUSED__, Ecore_File_Monitor *em __UNUSED__,
                  Ecore_File_Event event __UNUSED__, const char *path __UNUSED__)
{
   Instance *inst;
   Eina_List *l;
   int empty;

   empty = efreet_trash_is_empty();

   EINA_LIST_FOREACH(instances, l, inst)
     {
        if (!inst->o_trash) continue;
        if (empty)
          edje_object_signal_emit(inst->o_trash, "set_empty", "e");
        else
          edje_object_signal_emit(inst->o_trash, "set_full", "e");
     }
}

static void
_trash_button_cb_mouse_down(void *data, Evas *e __UNUSED__, 
                           Evas_Object *obj __UNUSED__, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;
   if ((ev->button == 1))
     {
        _trash_cb_menu_show(NULL, NULL, NULL);
     }
   else if ((ev->button == 3) && (!inst->menu))
     {
        E_Zone *zone;
        E_Menu *m;
        E_Menu_Item *mi;
        int x, y;

        zone = e_util_zone_current_get(e_manager_current_get());

        m = e_menu_new();

        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Empty Trash"));
        e_menu_item_icon_edje_set(mi, icon, "icon");
        e_menu_item_callback_set(mi, _trash_cb_menu_empty, inst);

        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Show trash"));
        e_menu_item_icon_edje_set(mi, icon, "icon");
        e_menu_item_callback_set(mi, _trash_cb_menu_show, inst);
        
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Settings"));
        e_util_menu_item_theme_icon_set(mi, "preferences-system");
        e_menu_item_callback_set(mi, _trash_cb_menu_configure, NULL);

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
        e_menu_post_deactivate_callback_set(m, _trash_cb_menu_post, inst);
        inst->menu = m;

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
        e_menu_activate_mouse(m, zone, x + ev->output.x, y + ev->output.y,
                              1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void
_trash_cb_menu_post(void *data, E_Menu *menu __UNUSED__)
{
   Instance *inst = data;

   if (inst && inst->menu)
     {
        e_object_del(E_OBJECT(inst->menu));
        inst->menu = NULL;
     }
}

static void
_trash_cb_obj_moveresize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
                         void *event_info __UNUSED__)
{
   Instance *inst = data;
   Evas_Coord x, y, w, h;

   if (!inst) return;

   e_gadcon_client_viewport_geometry_get(inst->gcc, &x, &y, &w, &h);
   e_drop_handler_geometry_set(inst->drop_handler, x, y, w, h);
}

static void
_trash_cb_menu_empty(void *data __UNUSED__, E_Menu *m __UNUSED__, E_Menu_Item *mi __UNUSED__)
{
   e_confirm_dialog_show("", NULL,
      D_("<b>Remove all the files and folders from the trash?</><br>"
         "All the elements currently in the trash will be irreparably lost!!!"),
      D_("Cancel"), D_("Empty trash"), 
      NULL, _trash_cb_menu_empty_ok, NULL, NULL, NULL, NULL);
}

static void 
_trash_cb_menu_empty_ok(void *data __UNUSED__)
{
   efreet_trash_empty_trash();
}

static void
_trash_cb_menu_show(void *data __UNUSED__, E_Menu *m __UNUSED__, E_Menu_Item *mi __UNUSED__)
{
   char buf[4096];
   E_Zone *zone;
  
   zone = e_util_zone_current_get (e_manager_current_get ());

   snprintf(buf, sizeof(buf), "%s trash:///", trash_conf->fileman);
   e_exec(zone, NULL, buf, NULL, NULL);
}

static void
_trash_dnd_cb_enter(void *data, const char *type __UNUSED__, void *event_info __UNUSED__)
{
   Instance *inst = data;
   edje_object_signal_emit(inst->o_trash, "drag_start", "e");
}

static void
_trash_dnd_cb_move(void *data __UNUSED__, const char *type __UNUSED__, void *event_info __UNUSED__)
{
   //~ E_Event_Dnd_Move *ev;
   //~ Instance *inst;

   //~ ev = event_info;
   //~ inst = data;

   //~ printf("_trash_cb_move ");
}

static void
_trash_dnd_cb_leave(void *data, const char *type __UNUSED__, void *event_info __UNUSED__)
{
   //~ E_Event_Dnd_Leave *ev;
   Instance *inst;

   //~ ev = event_info;
   inst = data;

   //printf("_trash_cb_leave\n");
   edje_object_signal_emit(inst->o_trash, "drag_end", "e");
}

static void
_trash_dnd_cb_drop(void *data, const char *type, void *event_info)
{
   Instance *inst;
   Eina_List *l, *nonlocals = NULL;
   E_Event_Dnd_Drop *ev;
   int res;

   inst = data;
   ev = event_info;

   //printf("_trash_cb_drop [type: %s][x:%d y:%d]\n", type,ev->x, ev->y);

   if (!type) return;
   if (strcmp(type, "text/uri-list")) return;

   //printf("RECEIVED (x:%d y:%d) NUMS:%d\n", eina_list_count(ev->data));

   for (l = ev->data; l; l = l->next)
     {
        Efreet_Uri *uri;
        if ((uri = efreet_uri_decode(l->data)))
          {
             res = efreet_trash_delete_uri(uri, 0);
             if (res == -1)
                nonlocals = eina_list_append(nonlocals, uri);
             else
                efreet_uri_free(uri);
          }
     }

   if (nonlocals)
     {
        printf("ALERT [%d]!!\n", eina_list_count(nonlocals));
        e_confirm_dialog_show(D_("Alert"), NULL, 
                     D_("Some files can't be moved to trash <br>"
                     "because they are not on the local filesystem.<br><br>"
                     "The files will be deleted FOREVER!!!"),
                     D_("Cancel"), D_("Delete Files"), 
                     NULL, //void (*bt1_func)(void *data)
                     _trash_dialog_cb_ok,     //void (*bt2_func)(void *data)
                     NULL, //void *data
                     nonlocals, //void *data2
                     _trash_dialog_cb_cancel, //void (*del_func)(void *data)
                     nonlocals      //void * del_data
                     );
     }

   edje_object_signal_emit(inst->o_trash, "drag_end", "e");
}

static void 
_trash_dialog_cb_ok(void *data)
{
   Eina_List *l;
   int errors = 0;

   //printf("DELETE %d\n", eina_list_count(data));

   for (l = data; l; l = l->next)
     {
        Efreet_Uri *uri;
        uri = l->data;
        //printf("**URI: %s\n", uri->path);
        if (!efreet_trash_delete_uri(uri, 1))
           errors++;
     }

   if (errors)
     {
        //TODO ALERT
     }
}

static void 
_trash_dialog_cb_cancel(void *data)
{
   Efreet_Uri *uri;
   Eina_List *l = data;

   EINA_LIST_FREE(l, uri)
     efreet_uri_free(uri);
}

static void 
_trash_cb_menu_configure(void *data __UNUSED__, E_Menu *mn, E_Menu_Item *mi __UNUSED__)
{
   if (!trash_conf) return;
   if (trash_conf->cfd) return;
   e_int_config_trash_module(mn->zone->container, NULL);
}

static void 
_trash_conf_new(void) 
{
   trash_conf = E_NEW(Config, 1);
   trash_conf->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) if ((trash_conf->version & 0xffff) < v) {
#define IFMODCFGEND }

   /* setup defaults */
   IFMODCFG(0x008d);
   //~ trash_conf->switch1 = 1;
   trash_conf->fileman = eina_stringshare_add("Thunar");

   _trash_conf_item_get(NULL);
   IFMODCFGEND;
   trash_conf->version = MOD_CONFIG_FILE_VERSION;
   e_config_save_queue();
}

static Config_Item *
_trash_conf_item_get(const char *id) 
{
   Config_Item *ci;

   //GADCON_CLIENT_CONFIG_GET(Config_Item, trash_conf->conf_items, _gc_class, id);

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->switch2 = 0;
   trash_conf->conf_items = eina_list_append(trash_conf->conf_items, ci);
   return ci;
}

static void 
_trash_conf_free(void) 
{
   /* cleanup any stringshares here */
   while (trash_conf->conf_items) 
     {
        Config_Item *ci = NULL;

        ci = trash_conf->conf_items->data;
        trash_conf->conf_items = 
          eina_list_remove_list(trash_conf->conf_items, 
                                trash_conf->conf_items);
        /* EPA */
        if (ci->id) eina_stringshare_del(ci->id);
        E_FREE(ci);
     }

   E_FREE(trash_conf);
}
