#include "e_mod_main.h"
#include "e_share.h"
#include <string.h>

#include <Ecore.h>

#define __UNUSED__
#define _(S) S

/* actual module specifics */
typedef struct _Instance Instance;
struct _Instance
{
   E_Gadcon_Client *gcc;
   E_Menu *menu;
   Ecore_X_Window win;
   Evas_Object *o_button;
   Eina_List *handlers;
   Eina_List *shares;
};

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon * gc, const char *name, const char *id, const char *style);
static void             _gc_shutdown(E_Gadcon_Client * gcc);
static void             _gc_orient(E_Gadcon_Client * gcc, E_Gadcon_Orient orient);
static const char      *_gc_label(const E_Gadcon_Client_Class *client_class);
static                  Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas * evas);
static const char      *_gc_id_new(const E_Gadcon_Client_Class *client_class);

static void _share_button_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, Evas_Event_Mouse_Down *ev);
static Eina_Bool _share_x_selection_notify_handler(Instance *instance, int type, void *event);
static void _share_menu_post_cb(void *data, E_Menu *menu);
static void _share_menu_share_request_click_cb(Instance *inst, E_Menu *m, E_Menu_Item *mi);
static void _share_menu_share_item_click_cb(Share_Data *selected_share, E_Menu *m, E_Menu_Item *mi);
static void _share_open_link_in_browser(const Share_Data *sd);
static void _share_notify(const Share_Data *sd);
static void _share_notification_clicked_cb(void *data, unsigned int id);
static void _free_share_data(Share_Data *sd);




static E_Module *share_module = NULL;

/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class = {
   GADCON_CLIENT_CLASS_VERSION,
   "share",
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
      e_gadcon_site_is_not_toolbar
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};


static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;

   inst = E_NEW(Instance, 1);

   /*
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/e-module-share.edj", e_module_dir_get(share_module));
   
   o = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(o, "base/theme/modules/share", "modules/share/main"))
     edje_object_file_set(o, buf, "modules/share/main");
   edje_object_signal_emit(o, "passive", "");
     */

   o = e_icon_add(gc->evas);
   e_icon_fdo_icon_set(o, "emblem-shared");
   evas_object_show(o);

   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;

   inst->gcc = gcc;
   inst->win = ecore_evas_window_get(gc->ecore_evas);
   inst->o_button = o;

   e_gadcon_client_util_menu_attach(gcc);

   evas_object_event_callback_add(inst->o_button, EVAS_CALLBACK_MOUSE_DOWN, (Evas_Object_Event_Cb)_share_button_cb_mouse_down, inst);
   E_LIST_HANDLER_APPEND(inst->handlers, ECORE_X_EVENT_SELECTION_NOTIFY, _share_x_selection_notify_handler, inst);

   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   inst = gcc->data;
   E_FREE_LIST(inst->handlers, ecore_event_handler_del);
   inst->handlers = NULL;

   if (inst->menu)
     {
        e_menu_post_deactivate_callback_set(inst->menu, NULL, NULL);
        e_object_del(E_OBJECT(inst->menu));
        inst->menu = NULL;
     }

   E_FREE_LIST(inst->shares, _free_share_data);
   inst->shares = NULL;

   evas_object_del(inst->o_button);
   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   e_gadcon_client_aspect_set (gcc, 16, 16);
   e_gadcon_client_min_size_set (gcc, 16, 16);
}

static const char *
_gc_label (const E_Gadcon_Client_Class *client_class)
{
   return "Share";
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas * evas)
{
   Evas_Object *o;
   /*
   char buf[PATH_MAX];

   o = edje_object_add(evas);
   snprintf (buf, sizeof(buf), "%s/e-module-share.edj", e_module_dir_get(share_module));
   edje_object_file_set(o, buf, "icon");
   */

   o = e_icon_add(evas);
   e_icon_fdo_icon_set(o, "emblem-shared");

   return o;
}

static const char *
_gc_id_new (const E_Gadcon_Client_Class *client_class)
{
   return _gadcon_class.name;
}

static void
_share_button_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, Evas_Event_Mouse_Down *ev)
{
    Instance *inst = (Instance*)data;
    Evas_Coord x, y, w, h;
    int cx, cy;
    int dir;
    E_Menu_Item *mi;
    Eina_List *it;
    Share_Data *share;

    if (!inst) return;

    if ((ev->button == 1) && (!inst->menu))
    {
        /* Coordinates and sizing */
        evas_object_geometry_get(inst->o_button, &x, &y, &w, &h);
        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy,
                                          NULL, NULL);
        x += cx;
        y += cy;

        inst->menu = e_menu_new();

        mi = e_menu_item_new(inst->menu);
        e_menu_item_label_set(mi, _("Share Clipboard Content"));
        e_menu_item_callback_set(mi, (E_Menu_Cb)_share_menu_share_request_click_cb, inst);

        mi = e_menu_item_new(inst->menu);
        e_menu_item_separator_set(mi, EINA_TRUE);

        EINA_LIST_FOREACH(inst->shares, it, share)
        {
            mi = e_menu_item_new(inst->menu);
            e_menu_item_label_set(mi, share->name);
            e_menu_item_callback_set(mi, (E_Menu_Cb)_share_menu_share_item_click_cb, share);
        }

        e_menu_post_deactivate_callback_set(inst->menu,
                _share_menu_post_cb, inst);

        /* Proper menu orientation */
        switch (inst->gcc->gadcon->orient)
        {
            case E_GADCON_ORIENT_TOP:
            case E_GADCON_ORIENT_CORNER_TL:
            case E_GADCON_ORIENT_CORNER_TR:
                dir = E_MENU_POP_DIRECTION_DOWN;
                break;

            case E_GADCON_ORIENT_BOTTOM:
            case E_GADCON_ORIENT_CORNER_BL:
            case E_GADCON_ORIENT_CORNER_BR:
                dir = E_MENU_POP_DIRECTION_UP;
                break;

            case E_GADCON_ORIENT_LEFT:
            case E_GADCON_ORIENT_CORNER_LT:
            case E_GADCON_ORIENT_CORNER_LB:
                dir = E_MENU_POP_DIRECTION_RIGHT;
                break;

            case E_GADCON_ORIENT_RIGHT:
            case E_GADCON_ORIENT_CORNER_RT:
            case E_GADCON_ORIENT_CORNER_RB:
                dir = E_MENU_POP_DIRECTION_LEFT;
                break;

            case E_GADCON_ORIENT_FLOAT:
            case E_GADCON_ORIENT_HORIZ:
            case E_GADCON_ORIENT_VERT:
            default:
                dir = E_MENU_POP_DIRECTION_AUTO;
                break;
        }

        e_gadcon_locked_set(inst->gcc->gadcon, EINA_TRUE);

        /* We display not relatively to the gadget, but similarly to
         * the start menu - thus the need for direction etc.
         */
        e_menu_activate_mouse(inst->menu,
                e_util_zone_current_get
                (e_manager_current_get()),
                x, y, w, h, dir, ev->timestamp);
    }
}

static Eina_Bool
_share_x_selection_notify_handler(Instance *instance, int type, void *event)
{
   char *icon;
   Ecore_X_Event_Selection_Notify *ev;
   Share_Data *sd = NULL;

   if (!instance)
     return EINA_TRUE;

   ev = event;
   if ((ev->selection == ECORE_X_SELECTION_CLIPBOARD) &&
       (strcmp(ev->target, ECORE_X_SELECTION_TARGET_UTF8_STRING) == 0))
     {
        Ecore_X_Selection_Data_Text *text_data;
        
        text_data = ev->data;
      
        if ((text_data->data.content == ECORE_X_SELECTION_CONTENT_TEXT) &&
            (text_data->text))
          {
			  
	char cmd[200];
	
	icon="edit-paste";
	
   snprintf(cmd, 200, "notify-send --expire-time=2500 --icon=%s  'Content shared!' 'Sourcedrop link is in your clipboard now.'", icon);
	//~ notify("edit-paste",4000,'Content shared!', 'Sourcedrop link is in your clipboard now.');
   ecore_init();
   ecore_exe_run(cmd, NULL);
   ecore_shutdown();
   snprintf(cmd, 200,"...");
			  
              char buf[20];
              if (text_data->data.length == 0)  return EINA_TRUE;

              sd = E_NEW(Share_Data, 1);
              sd->inst = instance;
              //~ snprintf(buf, ((text_data->data.length >= sizeof(buf)) ? (sizeof(buf) - 1) : text_data->data.length), text_data->text);
              
              strncpy(buf, text_data->text, 20);
              
              asprintf(&sd->name, "%s", buf);
              asprintf(&sd->content, "%s", text_data->text);

              sourcedrop_share(sd);
          }
     }


   return ECORE_CALLBACK_PASS_ON;
}

/* Updates the X selection with the selected text of the entry */
void
_clipboard_update(const char *text, const Instance *inst)
{
  EINA_SAFETY_ON_NULL_RETURN(inst);
  EINA_SAFETY_ON_NULL_RETURN(text);

  ecore_x_selection_clipboard_set(inst->win, text, strlen(text) + 1);
   //~ e_util_dialog_internal ("Pišta",text);
}

void e_share_upload_completed(Share_Data *sd)
{
    if (!sd) return;
    ((Instance*)sd->inst)->shares = eina_list_append(((Instance*)sd->inst)->shares, sd);
    //~ _share_notify(sd);
    _clipboard_update(sd->url, sd->inst);
   
}


static void
_share_menu_share_request_click_cb(Instance *inst, E_Menu *m, E_Menu_Item *mi)
{
  
 
    if (!inst) 
   	    return;   	
    
    ecore_x_selection_clipboard_request(inst->win, ECORE_X_SELECTION_TARGET_UTF8_STRING);
   
}

static void
_share_menu_share_item_click_cb(Share_Data *selected_share, E_Menu *m, E_Menu_Item *mi)
{
	 
    _share_open_link_in_browser(selected_share);
}

static void
_share_notify(const Share_Data *sd)
{

//~ this does not work under Moksha       
    
}

static void
_share_open_link_in_browser(const Share_Data *sd)
{
    char buf[256];
    
    if (!sd)
        return;
    
     
    snprintf(buf, (sizeof(buf) - 1), "xdg-open %s", sd->url);
    e_exec(e_gadcon_client_zone_get(((Instance*)sd->inst)->gcc), NULL, buf, NULL, NULL);
}

 static void
_share_notification_clicked_cb(void *data, unsigned int id)
{ 
    //~ _share_open_link_in_browser((Share_Data*)data);
    
}

static void
_share_menu_post_cb(void *data, E_Menu *menu)
{
   Instance *inst = data;

   if (!inst) return;
   e_gadcon_locked_set(inst->gcc->gadcon, EINA_FALSE);
   inst->menu = NULL;
}

static void _free_share_data(Share_Data *sd)
{
    free(sd->name);
    free(sd->content);
    free(sd->url);
    free(sd);
}

/* module setup */
EAPI E_Module_Api e_modapi = {
  E_MODULE_API_VERSION,
  "Share"
};

EAPI void *
e_modapi_init (E_Module * m)
{
   share_module = m;
   sourcedrop_init();
   e_gadcon_provider_register(&_gadcon_class);
   return share_module;
}

EAPI int
e_modapi_shutdown (E_Module * m)
{
  sourcedrop_shutdown();
  share_module = NULL;
  e_gadcon_provider_unregister(&_gadcon_class);
  return 1;
}

EAPI int
e_modapi_save(E_Module * m)
{
  return 1;
}


