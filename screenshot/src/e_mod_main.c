#include <e.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_X.h>
#include <limits.h>
#include "config.h"
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#include "e_mod_main.h"

static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static void _cfg_free(void);
static Eina_Bool _cfg_timer(void *data);
static void _cfg_new(void);
static void _cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _cb_menu_post(void *data, E_Menu *menu);
static void _cb_menu_cfg(void *data, E_Menu *menu, E_Menu_Item *mi);
static void _cb_normal(void *data, E_Menu *menu, E_Menu_Item *mi);
static void _cb_window(void *data, E_Menu *menu, E_Menu_Item *mi);
static void _cb_region(void *data, E_Menu *menu, E_Menu_Item *mi);
static void _cb_start_shot(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _cb_exec_shot(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _cb_dialog_ok(void *data, char *text);
static void _cb_send_msg(void *data);
static void _cb_do_shot(void);
static void _cb_take_shot(E_Object *obj, const char *params);
static Eina_Bool _cb_timer(void *data);

typedef struct _Instance Instance;
struct _Instance 
{
   E_Gadcon_Client *gcc;
   Evas_Object *o_base;
   E_Menu *menu, *menu_mode;
   Ecore_Timer *timer;
   int counter;
};

static Eina_List *instances = NULL;
static E_Config_DD *conf_edd = NULL;
static E_Action *act = NULL;
Config *ss_cfg = NULL;
static int aspect_width = 1;
static int aspect_height = 1;

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "screenshot", 
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
      e_gadcon_site_is_not_toolbar
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Screenshot"};

/* module functions */
EAPI void *
e_modapi_init(E_Module *m) 
{
   char buf[PATH_MAX];

#if HAVE_LOCALE_H
   setlocale(LC_ALL, "");
#endif
   snprintf(buf, sizeof(buf), "%s/locale", m->dir);
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   snprintf(buf, sizeof(buf), "%s/e-module-screenshot.edj", m->dir);

   /* register config dialog for panel */
   e_configure_registry_category_add("extensions", 90, "Screenshot", 
				     NULL, "preferences-extensions");
   e_configure_registry_item_add("extensions/screenshot", 20, D_("Screenshot"), 
				 NULL, buf, 
				 e_int_config_screenshot_module);

   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, mode, INT);
   E_CONFIG_VAL(D, T, quality, INT);
   E_CONFIG_VAL(D, T, thumb_size, INT);
   E_CONFIG_VAL(D, T, delay, DOUBLE);
   E_CONFIG_VAL(D, T, prompt, UCHAR);
   E_CONFIG_VAL(D, T, use_app, UCHAR);
   E_CONFIG_VAL(D, T, use_bell, UCHAR);
   E_CONFIG_VAL(D, T, use_thumb, UCHAR);
   E_CONFIG_VAL(D, T, location, STR);
   E_CONFIG_VAL(D, T, filename, STR);
   E_CONFIG_VAL(D, T, app, STR);

   ss_cfg = e_config_domain_load("module.screenshot", conf_edd);
   if (ss_cfg) 
     {
	if ((ss_cfg->version >> 16) < MOD_CONFIG_FILE_EPOCH) 
	  {
	     /* Config Too Old */
	     _cfg_free();
	     ecore_timer_add(1.0, _cfg_timer,
			     D_("Screenshot Module Configuration data needed "
			     "upgrading. Your old configuration<br> has been"
			     " wiped and a new set of defaults initialized. "
			     "This<br>will happen regularly during "
			     "development, so don't report a<br>bug. "
			     "This simply means Screenshot module needs "
			     "new configuration<br>data by default for "
			     "usable functionality that your old<br>"
			     "configuration simply lacks. This new set of "
			     "defaults will fix<br>that by adding it in. "
			     "You can re-configure things now to your<br>"
			     "liking. Sorry for the inconvenience.<br>"));
	  }
	else if (ss_cfg->version > MOD_CONFIG_FILE_VERSION) 
	  {
	     /* Config Too New */
	     _cfg_free();
	     ecore_timer_add(1.0, _cfg_timer, 
			     D_("Your Screenshot Module configuration is NEWER "
			     "than the Screenshot Module version. This is "
			     "very<br>strange. This should not happen unless"
			     " you downgraded<br>the Screenshot Module or "
			     "copied the configuration from a place where"
			     "<br>a newer version of the Screenshot Module "
			     "was running. This is bad and<br>as a "
			     "precaution your configuration has been now "
			     "restored to<br>defaults. Sorry for the "
			     "inconvenience.<br>"));
	  }
     }

   if (!ss_cfg) _cfg_new();

   ss_cfg->mod_dir = eina_stringshare_add(m->dir);

   /* register actions for keybindings */
   act = e_action_add("screenshot");
   if (act) 
     {
	act->func.go = _cb_take_shot;
	e_action_predef_name_set("Screenshot", D_("Take Screenshot"), 
				 "screenshot", NULL, NULL, 0);	
     }
   
   e_gadcon_provider_register(&_gc_class);
   return m;
}

EAPI int 
e_modapi_shutdown(E_Module *m) 
{
   if (ss_cfg->cfd) e_object_del(E_OBJECT(ss_cfg->cfd));

   if (act) 
     {
	e_action_predef_name_del("Screenshot", D_("Take Screenshot"));
	e_action_del("screenshot");
	act = NULL;
     }

   e_configure_registry_item_del("extensions/screenshot");
   e_configure_registry_category_del("screenshot");

   e_gadcon_provider_unregister(&_gc_class);
   _cfg_free();
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int 
e_modapi_save(E_Module *m) 
{
   e_config_domain_save("module.screenshot", conf_edd, ss_cfg);
   return 1;
}

/* Gadcon Functions */
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   Instance *inst = NULL;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/e-module-screenshot.edj", ss_cfg->mod_dir);

   inst = E_NEW(Instance, 1);

   inst->o_base = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(inst->o_base, "base/theme/modules/screenshot", 
				"modules/screenshot/main"))
     edje_object_file_set(inst->o_base, buf, "modules/screenshot/main");

   // respect theme aspect, but ignore Evas_Aspect_Control
   evas_object_size_hint_aspect_get(inst->o_base, NULL, &aspect_width, 
                                    &aspect_height);

   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->o_base);
   inst->gcc->data = inst;

   edje_object_signal_callback_add(inst->o_base, "e,action,screenshot,start", 
				   "*", _cb_start_shot, inst);
   edje_object_signal_callback_add(inst->o_base, "e,action,screenshot,exec", 
				   "*", _cb_exec_shot, inst);
   evas_object_event_callback_add(inst->o_base, EVAS_CALLBACK_MOUSE_DOWN, 
				  _cb_mouse_down, inst);

   edje_object_part_text_set(inst->o_base, "e.text.counter", "");

   instances = eina_list_append(instances, inst);
   return inst->gcc;
}

static void 
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst = NULL;

   if (!(inst = gcc->data)) return;
   instances = eina_list_remove(instances, inst);
   if (inst->timer) ecore_timer_del(inst->timer);
   if (inst->menu) 
     {
	e_menu_post_deactivate_callback_set(inst->menu, NULL, NULL);
	e_object_del(E_OBJECT(inst->menu));
	inst->menu = NULL;
     }
   if (inst->o_base) 
     {
	evas_object_event_callback_del(inst->o_base, EVAS_CALLBACK_MOUSE_DOWN,
				       _cb_mouse_down);
	evas_object_del(inst->o_base);
     }
   E_FREE(inst);
}

static void 
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient) 
{
   e_gadcon_client_aspect_set(gcc, aspect_width, aspect_height);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class) 
{
   return D_("Screenshot");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas) 
{
   Evas_Object *o = NULL;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/e-module-screenshot.edj", ss_cfg->mod_dir);
   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class) 
{
   static char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s.%d", _gc_class.name, 
            eina_list_count(instances) + 1);
   return buf;
}

/* private module functions */
static void 
_cfg_free(void) 
{
   if (ss_cfg->mod_dir) eina_stringshare_del(ss_cfg->mod_dir);
   if (ss_cfg->location) eina_stringshare_del(ss_cfg->location);
   if (ss_cfg->filename) eina_stringshare_del(ss_cfg->filename);
   if (ss_cfg->app) eina_stringshare_del(ss_cfg->app);
   E_FREE(ss_cfg);
}

static Eina_Bool 
_cfg_timer(void *data) 
{
   e_util_dialog_internal(D_("Screenshot Configuration Updated"), data);
   return EINA_FALSE;
}

static void 
_cfg_new(void) 
{
   char buf[PATH_MAX];

   ss_cfg = E_NEW(Config, 1);
   ss_cfg->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) \
   if ((ss_cfg->version & 0xffff) < v) {
#define IFMODCFGEND }

   IFMODCFG(0x008d);
   ss_cfg->mode = 0;
   ss_cfg->quality = 75;
   ss_cfg->thumb_size = 50;
   ss_cfg->delay = 60.0;
   ss_cfg->prompt = 0;
   ss_cfg->use_app = 0;
   ss_cfg->use_bell = 1;
   ss_cfg->use_thumb = 0;
   snprintf(buf, sizeof(buf), "%s/Desktop", e_user_homedir_get());
   ss_cfg->location = eina_stringshare_add(buf);
   ss_cfg->filename = NULL;
   ss_cfg->app = NULL;
   IFMODCFGEND;

   ss_cfg->version = MOD_CONFIG_FILE_VERSION;

   E_CONFIG_LIMIT(ss_cfg->mode, 0, 2);
   E_CONFIG_LIMIT(ss_cfg->quality, 1, 100);
   E_CONFIG_LIMIT(ss_cfg->delay, 0.0, 60.0);
   E_CONFIG_LIMIT(ss_cfg->thumb_size, 10, 100);
   e_config_save_queue();
}

static void 
_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info) 
{
   Instance *inst = NULL;
   Evas_Event_Mouse_Down *ev;

   if (!(inst = data)) return;
   ev = event_info;
   if ((ev->button == 3) && (!inst->menu)) 
     {
        E_Menu *m = NULL, *mo = NULL;
        E_Menu_Item *mi = NULL;
        E_Zone *zone = NULL;
        int x = 0, y = 0;

	zone = e_util_zone_current_get(e_manager_current_get());

	m = e_menu_new();
	mo = e_menu_new();
	inst->menu_mode = mo;

	mi = e_menu_item_new(mo);
	e_menu_item_label_set(mi, D_("Whole Screen"));
	e_menu_item_radio_group_set(mi, 1);
	e_menu_item_radio_set(mi, 1);
	if (ss_cfg->mode == 0) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cb_normal, inst);

	mi = e_menu_item_new(mo);
	e_menu_item_label_set(mi, D_("Select Window"));
	e_menu_item_radio_group_set(mi, 1);
	e_menu_item_radio_set(mi, 1);
	if (ss_cfg->mode == 1) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cb_window, inst);

	mi = e_menu_item_new(mo);
	e_menu_item_label_set(mi, D_("Select Region"));
	e_menu_item_radio_group_set(mi, 1);
	e_menu_item_radio_set(mi, 1);
	if (ss_cfg->mode == 2) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cb_region, inst);

	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, D_("Capture Mode"));
	e_menu_item_submenu_set(mi, inst->menu_mode);

	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, D_("Settings"));
	e_util_menu_item_theme_icon_set(mi, "configure");
	e_menu_item_callback_set(mi, _cb_menu_cfg, inst);

	m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
	e_menu_post_deactivate_callback_set(m, _cb_menu_post, inst);
        inst->menu = m;

	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
	e_menu_activate_mouse(m, zone, x + ev->output.x, y + ev->output.y, 
			      1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
     }
}

static void 
_cb_menu_post(void *data, E_Menu *menu) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   if (!inst->menu) return;
   if (inst->menu_mode) e_object_del(E_OBJECT(inst->menu_mode));
   inst->menu_mode = NULL;
   e_object_del(E_OBJECT(inst->menu));
   inst->menu = NULL;
}

static void 
_cb_menu_cfg(void *data, E_Menu *menu, E_Menu_Item *mi)
{
   Instance *inst = NULL;
   E_Container *con;

   if (!(inst = data)) return;

   /* kill/cancel any existing countdown timer */
   if (inst->timer) 
     {
        ecore_timer_del(inst->timer);
        inst->timer = NULL;
        inst->counter = 0;
        edje_object_part_text_set(inst->o_base, "e.text.counter", "");
     }

   con = e_container_current_get(e_manager_current_get());
   e_int_config_screenshot_module(con, NULL);
}

static void 
_cb_normal(void *data, E_Menu *menu, E_Menu_Item *mi) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   ss_cfg->mode = 0;
   e_config_save_queue();
}

static void 
_cb_window(void *data, E_Menu *menu, E_Menu_Item *mi) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   ss_cfg->mode = 1;
   e_config_save_queue();
}

static void 
_cb_region(void *data, E_Menu *menu, E_Menu_Item *mi) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   ss_cfg->mode = 2;
   e_config_save_queue();
}

static void 
_cb_start_shot(void *data, Evas_Object *obj, const char *emission, const char *source) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   if (ss_cfg->prompt)
     {
	e_entry_dialog_show(D_("Screenshot Module"), "enlightenment", 
			    D_("Enter a new filename for this screenshot"),
			    NULL, NULL, NULL, _cb_dialog_ok, NULL, inst);
     }
   else 
     inst->timer = ecore_timer_add(1.0, _cb_timer, inst);
}

static void 
_cb_exec_shot(void *data, Evas_Object *obj, const char *emission, const char *source) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;

   _cb_do_shot();
   edje_object_signal_emit(inst->o_base, "e,action,screenshot,stop", "");
   edje_object_part_text_set(inst->o_base, "e.text.counter", "");
}

static void 
_cb_dialog_ok(void *data, char *text) 
{
   Instance *inst = NULL;
   char buf[PATH_MAX];
   char *t = NULL;

   if (!(inst = data)) return;
   if (!text) return;
   t = ecore_file_dir_get(text);
   if (!strcmp(t, text)) 
     {
	snprintf(buf, sizeof(buf), "%s/%s", ss_cfg->location, 
		 ecore_file_file_get(text));
     }
   else
     snprintf(buf, sizeof(buf), "%s", text);

   if (ss_cfg->filename) eina_stringshare_del(ss_cfg->filename);
   ss_cfg->filename = eina_stringshare_add(buf);

   inst->timer = ecore_timer_add(1.0, _cb_timer, inst);
}

static void 
_cb_send_msg(void *data) 
{
   Instance *inst = NULL;
   Edje_Message_Int_Set *msg = NULL;

   if (!(inst = data)) return;
//   if (ss_cfg->delay <= 0.0) return;
   msg = malloc(sizeof(Edje_Message_Int_Set) + 1 * sizeof(int));
   msg->count = 1;
   msg->val[0] = ss_cfg->delay;
   edje_object_message_send(inst->o_base, EDJE_MESSAGE_INT_SET, 1, msg);
   free(msg);
   msg = NULL;
}

static void 
_cb_do_shot(void) 
{
   Ecore_Exe *exe;
   char *tmp, buf[PATH_MAX];

   tmp = strdup("");
   if (ss_cfg->use_bell) 
     {
	snprintf(buf, sizeof(buf), "--beep ");
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if (ss_cfg->quality > 0) 
     {
	snprintf(buf, sizeof(buf), "--quality %d ", ss_cfg->quality);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   switch (ss_cfg->mode) 
     {
      case 0:
	break;
      case 1:
	snprintf(buf, sizeof(buf), "--window ");
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
	break;
      case 2:
	snprintf(buf, sizeof(buf), "--region ");
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
	break;
     }

   if ((ss_cfg->use_app) && (ss_cfg->app)) 
     {
	snprintf(buf, sizeof(buf), "--app %s ", ss_cfg->app);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if ((ss_cfg->use_thumb) && (ss_cfg->thumb_size > 0)) 
     {
	snprintf(buf, sizeof(buf), "--thumb-geom %d ", ss_cfg->thumb_size);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if ((ss_cfg->prompt) && (ss_cfg->filename))
     {
	snprintf(buf, sizeof(buf), "%s", ss_cfg->filename);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }
   else 
     {
	if ((ss_cfg->location) && (ss_cfg->filename)) 
	  {
	     snprintf(buf, sizeof(buf), "%s/%s", ss_cfg->location, 
		      ss_cfg->filename);
	     tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	     strcat(tmp, buf);	     
	  }
	else if (ss_cfg->location) 
	  {
	     snprintf(buf, sizeof(buf), "%s", ss_cfg->location);
	     tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	     strcat(tmp, buf);
	  }
     }

   if (!ecore_file_app_installed("emprint")) return;
   snprintf(buf, sizeof(buf), "emprint %s", tmp);
   exe = ecore_exe_run(buf, NULL);
   if (exe) ecore_exe_free(exe);
}

static void 
_cb_take_shot(E_Object *obj, const char *params) 
{
   Ecore_Exe *exe;
   char *tmp, buf[PATH_MAX];

   tmp = strdup("");
   if (ss_cfg->delay > 0) 
     {
	snprintf(buf, sizeof(buf), "--delay %i ", (int)ss_cfg->delay);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if (ss_cfg->use_bell) 
     {
	snprintf(buf, sizeof(buf), "--beep ");
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if (ss_cfg->quality > 0) 
     {
	snprintf(buf, sizeof(buf), "--quality %d ", ss_cfg->quality);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   switch (ss_cfg->mode) 
     {
      case 0:
	break;
      case 1:
	snprintf(buf, sizeof(buf), "--window ");
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
	break;
      case 2:
	snprintf(buf, sizeof(buf), "--region ");
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
	break;
     }

   if ((ss_cfg->use_app) && (ss_cfg->app)) 
     {
	snprintf(buf, sizeof(buf), "--app %s ", ss_cfg->app);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if ((ss_cfg->use_thumb) && (ss_cfg->thumb_size > 0)) 
     {
	snprintf(buf, sizeof(buf), "--thumb-geom %d ", ss_cfg->thumb_size);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if ((ss_cfg->prompt) && (ss_cfg->filename))
     {
	snprintf(buf, sizeof(buf), "%s", ss_cfg->filename);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }
   else 
     {
	if ((ss_cfg->location) && (ss_cfg->filename)) 
	  {
	     snprintf(buf, sizeof(buf), "%s/%s", ss_cfg->location, 
		      ss_cfg->filename);
	     tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	     strcat(tmp, buf);	     
	  }
	else if (ss_cfg->location) 
	  {
	     snprintf(buf, sizeof(buf), "%s", ss_cfg->location);
	     tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	     strcat(tmp, buf);
	  }
     }

   snprintf(buf, sizeof(buf), "emprint %s", tmp);
   exe = ecore_exe_run(buf, NULL);
   if (exe) ecore_exe_free(exe);
}

static Eina_Bool 
_cb_timer(void *data) 
{
   Instance *inst = NULL;
   char buf[256];

   if (!(inst = data)) return EINA_FALSE;
   if ((ss_cfg->delay - inst->counter) == 0) 
     {
        inst->timer = NULL;
        inst->counter = 0;
        _cb_send_msg(inst);
        return EINA_FALSE;
     }
   snprintf(buf, sizeof(buf), "%2.0f", (ss_cfg->delay - inst->counter));
   edje_object_part_text_set(inst->o_base, "e.text.counter", buf);
   inst->counter++;
   return EINA_TRUE;
}
