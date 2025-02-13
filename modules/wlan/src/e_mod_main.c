#include <e.h>
#include "e_mod_main.h"

typedef struct _Instance Instance;
typedef struct _Wlan Wlan;

struct _Instance
{
  E_Gadcon_Client *gcc;
  Evas_Object *wlan_obj;
  Wlan *wlan;
  Ecore_Timer *check_timer;
  Config_Item *ci;
};

struct _Wlan
{
  Instance *inst;
  Evas_Object *wlan_obj;
};

/* Func Protos for Gadcon */
static E_Gadcon_Client *_gc_init (E_Gadcon * gc, const char *name,
				  const char *id, const char *style);
static void _gc_shutdown (E_Gadcon_Client * gcc);
static void _gc_orient (E_Gadcon_Client * gcc, E_Gadcon_Orient orient);
static const char *_gc_label (const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon (const E_Gadcon_Client_Class *client_class, Evas * evas);
static const char *_gc_id_new (const E_Gadcon_Client_Class *client_class);

/* Func Protos for Module */
static void _wlan_cb_mouse_down (void *data, Evas * e, Evas_Object * obj,
				void *event_info);
static void _wlan_cb_mouse_in (void *data, Evas * e, Evas_Object * obj,
			      void *event_info);
static void _wlan_cb_mouse_out (void *data, Evas * e, Evas_Object * obj,
			       void *event_info);
static void _wlan_menu_cb_configure (void *data, E_Menu * m, E_Menu_Item * mi);
static void _wlan_menu_cb_post (void *data, E_Menu * m);
static Config_Item *_wlan_config_item_get (const char *id);
static Wlan *_wlan_new (Evas * evas);
static void _wlan_free (Wlan * wlan);
static Eina_Bool _wlan_cb_check (void *data);
static void _wlan_update_qual (void *data, double value);
static void _wlan_update_level (void *data, double value);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *wlan_config = NULL;

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "wlan", 
 {
    _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, NULL
 },
   E_GADCON_CLIENT_STYLE_PLAIN
};

static E_Gadcon_Client *
_gc_init (E_Gadcon * gc, const char *name, const char *id, const char *style)
{
  E_Gadcon_Client *gcc;
  Evas_Object *o;
  Instance *inst;
  Wlan *wlan;

  inst = E_NEW (Instance, 1);

  inst->ci = _wlan_config_item_get (id);

  wlan = _wlan_new (gc->evas);
  wlan->inst = inst;
  inst->wlan = wlan;

  o = wlan->wlan_obj;
  gcc = e_gadcon_client_new (gc, name, id, style, o);
  gcc->data = inst;
  inst->gcc = gcc;
  inst->wlan_obj = o;

  evas_object_event_callback_add (o, EVAS_CALLBACK_MOUSE_DOWN,
				  _wlan_cb_mouse_down, inst);
  evas_object_event_callback_add (o, EVAS_CALLBACK_MOUSE_IN, _wlan_cb_mouse_in,
				  inst);
  evas_object_event_callback_add (o, EVAS_CALLBACK_MOUSE_OUT,
				  _wlan_cb_mouse_out, inst);

  if (inst->ci->always_text)
    edje_object_signal_emit (inst->wlan_obj, "label_active", "");

  _wlan_cb_check (inst);

  inst->check_timer = ecore_timer_add (inst->ci->poll_time, _wlan_cb_check, inst);
  wlan_config->instances = eina_list_append (wlan_config->instances, inst);
  return gcc;
}

static void
_gc_orient (E_Gadcon_Client * gcc, E_Gadcon_Orient orient)
{
  e_gadcon_client_aspect_set (gcc, 16, 16);
  e_gadcon_client_min_size_set (gcc, 16, 16);
}

static const char *
_gc_label (const E_Gadcon_Client_Class *client_class)
{
  return D_ ("Wlan");
}

static Evas_Object *
_gc_icon (const E_Gadcon_Client_Class *client_class, Evas * evas)
{
  Evas_Object *o;
  char buf[PATH_MAX];

  o = edje_object_add (evas);
  snprintf (buf, sizeof (buf), "%s/e-module-wlan.edj",
	    e_module_dir_get (wlan_config->module));
  edje_object_file_set (o, buf, "icon");
  return o;
}

static const char *
_gc_id_new (const E_Gadcon_Client_Class *client_class)
{
   Config_Item *ci;

   ci = _wlan_config_item_get (NULL);
   return ci->id;
}

static void
_gc_shutdown (E_Gadcon_Client * gcc)
{
  Instance *inst;

  inst = gcc->data;
  if (inst->check_timer)
    ecore_timer_del (inst->check_timer);
  wlan_config->instances = eina_list_remove (wlan_config->instances, inst);
  _wlan_free (inst->wlan);
   E_FREE(inst);
}

static void
_wlan_cb_mouse_down (void *data, Evas * e, Evas_Object * obj, void *event_info)
{
  Instance *inst;
  Evas_Event_Mouse_Down *ev;

  inst = data;
  ev = event_info;
  if ((ev->button == 3) && (!wlan_config->menu))
    {
      E_Menu *m;
      E_Menu_Item *mi;
      int x, y, w, h;

      m = e_menu_new ();
      mi = e_menu_item_new (m);
      e_menu_item_label_set (mi, D_ ("Settings"));
      e_util_menu_item_theme_icon_set(mi, "preferences-system");
      e_menu_item_callback_set (mi, _wlan_menu_cb_configure, inst);

      m = e_gadcon_client_util_menu_items_append (inst->gcc, m, 0);
      e_menu_post_deactivate_callback_set (m, _wlan_menu_cb_post, inst);
      wlan_config->menu = m;
      e_gadcon_canvas_zone_geometry_get (inst->gcc->gadcon, &x, &y, &w, &h);
      e_menu_activate_mouse (m,
			     e_util_zone_current_get (e_manager_current_get
						      ()), x + ev->output.x,
			     y + ev->output.y, 1, 1,
			     E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
      evas_event_feed_mouse_up (inst->gcc->gadcon->evas, ev->button,
				EVAS_BUTTON_NONE, ev->timestamp, NULL);
    }
}

static void
_wlan_menu_cb_post (void *data, E_Menu * m)
{
   if (!wlan_config->menu) return;
   e_object_del (E_OBJECT (wlan_config->menu));
   wlan_config->menu = NULL;
}

static void
_wlan_menu_cb_configure (void *data, E_Menu * m, E_Menu_Item * mi)
{
  Instance *inst;

  inst = data;
  _config_wlan_module (inst->ci);
}

void
_wlan_config_updated (Config_Item *ci)
{
  Eina_List *l;

  if (!wlan_config) return;
  for (l = wlan_config->instances; l; l = l->next)
    {
      Instance *inst;

      inst = l->data;
      if (inst->ci != ci) continue;

      if (inst->check_timer)
	ecore_timer_del (inst->check_timer);
      inst->check_timer =
	      ecore_timer_add (inst->ci->poll_time, _wlan_cb_check, inst);
      if (inst->ci->always_text)
	edje_object_signal_emit (inst->wlan_obj, "label_active", "");
      else
	edje_object_signal_emit (inst->wlan_obj, "label_passive", "");
    }
}

static Config_Item *
_wlan_config_item_get (const char *id)
{
   Eina_List *l;
   Config_Item *ci;
   char buf[128];

   if (!id)
     {
	int  num = 0;

	/* Create id */
	if (wlan_config->items)
	  {
	     const char *p;

	     ci = eina_list_last (wlan_config->items)->data;
	     p = strrchr (ci->id, '.');
	     if (p) num = atoi (p + 1) + 1;
	  }
	snprintf (buf, sizeof (buf), "%s.%d", _gc_class.name, num);
	id = buf;
     }
   else
     {
	for (l = wlan_config->items; l; l = l->next)
	  {
	     ci = l->data;
	     if (!ci->id) continue;
	     if (!strcmp (ci->id, id)) 
	       {
		  if (!ci->device)
		    ci->device = eina_stringshare_add ("wlan0");
		  return ci;
	       }
	  }
     }
   ci = E_NEW (Config_Item, 1);
   ci->id = eina_stringshare_add (id);
   ci->device = eina_stringshare_add ("wlan0");  
   ci->poll_time = 1.0;
   ci->always_text = 0;
   ci->show_percent = 1;

   wlan_config->items = eina_list_append (wlan_config->items, ci);
   return ci;
}

EAPI E_Module_Api e_modapi = 
{
  E_MODULE_API_VERSION, "Wlan"
};

EAPI void *
e_modapi_init (E_Module * m)
{
  char buf[PATH_MAX];

  snprintf (buf, sizeof (buf), "%s/locale", e_module_dir_get (m));
  bindtextdomain (PACKAGE, buf);
  bind_textdomain_codeset (PACKAGE, "UTF-8");

  conf_item_edd = E_CONFIG_DD_NEW ("Wlan_Config_Item", Config_Item);
#undef T
#undef D
#define T Config_Item
#define D conf_item_edd
  E_CONFIG_VAL (D, T, id, STR);
  E_CONFIG_VAL (D, T, device, STR);  
  E_CONFIG_VAL (D, T, poll_time, DOUBLE);
  E_CONFIG_VAL (D, T, always_text, INT);
  E_CONFIG_VAL (D, T, show_percent, INT);

  conf_edd = E_CONFIG_DD_NEW ("Wlan_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
  E_CONFIG_LIST (D, T, items, conf_item_edd);

  wlan_config = e_config_domain_load ("module.wlan", conf_edd);
  if (!wlan_config)
    {
      Config_Item *ci;

      wlan_config = E_NEW (Config, 1);

      ci = E_NEW (Config_Item, 1);
      ci->id = eina_stringshare_add ("0");
      ci->device = eina_stringshare_add ("wlan0");      
      ci->poll_time = 1.0;
      ci->always_text = 0;
      ci->show_percent = 1;
      wlan_config->items = eina_list_append (wlan_config->items, ci);
    }
  wlan_config->module = m;
  e_gadcon_provider_register (&_gc_class);
  return m;
}

EAPI int
e_modapi_shutdown (E_Module * m)
{
  wlan_config->module = NULL;
  e_gadcon_provider_unregister (&_gc_class);

  if (wlan_config->config_dialog)
    e_object_del (E_OBJECT (wlan_config->config_dialog));
  if (wlan_config->menu)
    {
      e_menu_post_deactivate_callback_set (wlan_config->menu, NULL, NULL);
      e_object_del (E_OBJECT (wlan_config->menu));
      wlan_config->menu = NULL;
    }
  while (wlan_config->items)
    {
      Config_Item *ci;

      ci = wlan_config->items->data;
      wlan_config->items =
	eina_list_remove_list (wlan_config->items, wlan_config->items);
      if (ci->id)
	eina_stringshare_del (ci->id);
      if (ci->device)
	eina_stringshare_del (ci->device);
       E_FREE(ci);
    }
   E_FREE(wlan_config);
   E_CONFIG_DD_FREE (conf_item_edd);
   E_CONFIG_DD_FREE (conf_edd);
  return 1;
}

EAPI int
e_modapi_save (E_Module * m)
{
  e_config_domain_save ("module.wlan", conf_edd, wlan_config);
  return 1;
}

static Wlan *
_wlan_new (Evas * evas)
{
  Wlan *wlan;
  char buf[PATH_MAX];

  wlan = E_NEW (Wlan, 1);

  wlan->wlan_obj = edje_object_add (evas);
  snprintf (buf, sizeof (buf), "%s/wlan.edj",
	    e_module_dir_get (wlan_config->module));
  if (!e_theme_edje_object_set
      (wlan->wlan_obj, "base/theme/modules/wlan", "modules/wlan/main"))
    edje_object_file_set (wlan->wlan_obj, buf, "modules/wlan/main");
  evas_object_show (wlan->wlan_obj);

  return wlan;
}

static void
_wlan_free (Wlan * m)
{
   evas_object_del (m->wlan_obj);
   E_FREE(m);
}

static void
_wlan_cb_mouse_in (void *data, Evas * e, Evas_Object * obj, void *event_info)
{
  Instance *inst;

  inst = data;
  edje_object_signal_emit (inst->wlan_obj, "label_active", "");
}

static void
_wlan_cb_mouse_out (void *data, Evas * e, Evas_Object * obj, void *event_info)
{
  Instance *inst;

  inst = data;
  if (!inst->ci->always_text)
    edje_object_signal_emit (inst->wlan_obj, "label_passive", "");
}

static Eina_Bool
_wlan_cb_check (void *data)
{
   Instance *inst;

   unsigned int dummy;
   char iface[64];
   char buf[256];
   FILE *stat;
   int found_dev = 0;
   int wlan_status = 0;
   int wlan_link = 0;
   int wlan_level = 0;
   int wlan_noise = 0;
   char omsg[100];
   char in_str[100];

   stat = fopen("/proc/net/wireless", "r");
   if (!stat) return EINA_TRUE;

   inst = data;
   
   while (fgets(buf, 256, stat))
     {
        int i = 0;

        /* remove : */
        for (; buf[i] != 0; i++)
           if (buf[i] == ':' || buf[i] == '.')
              buf[i] = ' ';

        if (sscanf(buf, "%s %u %u %u %u %u %u %u %u %u %u",
                   iface, &wlan_status, &wlan_link, &wlan_level, &wlan_noise, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy) < 11)
           continue;

      if (!inst->ci->device)
	continue;

      if (!strcmp (iface, inst->ci->device))
	{
	  found_dev = 1;
	  break;
	}
     }
   fclose(stat);

   if (!found_dev) return EINA_TRUE;

   snprintf(in_str, sizeof(in_str), "LNK: %d%%", wlan_link);

   double link_send = ((double)wlan_link / (double)100.0);
   double level_send = ((double)wlan_level / (double)100.0);

  _wlan_update_qual(inst, link_send);
  _wlan_update_level (inst, level_send);
  
   snprintf(omsg,sizeof(omsg),"Qual: %d%%", wlan_link);
   edje_object_part_text_set(inst->wlan_obj, "qual_label", omsg);

   snprintf(omsg,sizeof(omsg),"Lvl: %d%%", wlan_level);
   edje_object_part_text_set(inst->wlan_obj, "level_label", omsg);

  return EINA_TRUE;
}

static void 
_wlan_update_qual(void *data, double value) 
{
   Instance *inst;
   Edje_Message_Float *val;
   
   inst = data;
   val = malloc(sizeof(Edje_Message_Float));
   val->val = value;
   edje_object_message_send(inst->wlan_obj, EDJE_MESSAGE_FLOAT,1, val);
   free(val);
}

static void 
_wlan_update_level(void *data, double value) 
{
   Instance *inst;
   Edje_Message_Float *val;

   inst = data;
   val = malloc(sizeof(Edje_Message_Float));
   val->val = value;   
   edje_object_message_send(inst->wlan_obj, EDJE_MESSAGE_FLOAT,2, val);
   free(val);
}

