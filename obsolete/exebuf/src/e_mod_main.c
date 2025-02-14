#include <e.h>
#include "e_mod_main.h"

static void _exebuf_conf_new(void);
static void _exebuf_conf_free(void);
static Eina_Bool _exebuf_conf_timer(void *data);
static void _exebuf_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _exebuf_cb_menu_post(void *data, E_Menu *menu);
static void _exebuf_cb_menu_configure(void *data, E_Menu *mn, E_Menu_Item *mi);
static void _exebuf_action_exebuf_cb(E_Object *obj, const char *params);
static Eina_Bool _exebuf_run_defer_cb(void *data);
static void _exebuf_run_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void _exebuf_menu_add(void *data, E_Menu *m);

/* Local Variables */
static E_Config_DD *conf_edd = NULL;
static E_Action *act = NULL;
static E_Int_Menu_Augmentation *maug = NULL;

Config *exebuf_conf = NULL;

/* We set the version and the name, check e_mod_main.h for more details */
EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Exebuf"};

/*
 * Module Functions
 */

/* Function called when the module is initialized */
EAPI void *
e_modapi_init(E_Module *m) 
{
   char buf[4096];
   
   /* Location of message catalogs for localization */
   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   /* /\* Location of theme to load for this module *\/
    * snprintf(buf, sizeof(buf), "%s/e-module-exebuf.edj", m->dir);
    * 
    * 
    * /\* Display this Modules config info in the main Config Panel *\/
    * 
    * /\* starts with a category, create it if not already exists *\/
    * e_configure_registry_category_add("advanced", 80, "Advanced", 
    *                                   NULL, "preferences-advanced");
    * /\* add right-side item *\/
    * e_configure_registry_item_add("advanced/exebuf", 110, D_("Exebuf"), 
    *                               NULL, buf, e_int_config_exebuf_module); */

   /* Define EET Data Storage for the config file */
   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, max_exe_list, INT);
   E_CONFIG_VAL(D, T, max_eap_list, INT);
   E_CONFIG_VAL(D, T, max_hist_list, INT);
   E_CONFIG_VAL(D, T, scroll_animate, INT);
   E_CONFIG_VAL(D, T, scroll_speed, DOUBLE);
   E_CONFIG_VAL(D, T, pos_align_x, DOUBLE);
   E_CONFIG_VAL(D, T, pos_align_y, DOUBLE);
   E_CONFIG_VAL(D, T, pos_size_w, DOUBLE);
   E_CONFIG_VAL(D, T, pos_size_h, DOUBLE);
   E_CONFIG_VAL(D, T, pos_min_w, INT);
   E_CONFIG_VAL(D, T, pos_min_h, INT);
   E_CONFIG_VAL(D, T, pos_max_w, INT);
   E_CONFIG_VAL(D, T, pos_max_h, INT);
   E_CONFIG_VAL(D, T, term_cmd, STR);

   /* Tell E to find any existing module data. First run ? */
   exebuf_conf = e_config_domain_load("module.exebuf", conf_edd);
   if (exebuf_conf) 
     {
        /* Check config version */
        if ((exebuf_conf->version >> 16) < MOD_CONFIG_FILE_EPOCH) 
          {
             /* config too old */
	    _exebuf_conf_free();
	     ecore_timer_add(1.0, _exebuf_conf_timer,
			     D_("Exebuf Module Configuration data needed "
			     "upgrading. Your old configuration<br> has been"
			     " wiped and a new set of defaults initialized. "
			     "This<br>will happen regularly during "
			     "development, so don't report a<br>bug. "
			     "This simply means the module needs "
			     "new configuration<br>data by default for "
			     "usable functionality that your old<br>"
			     "configuration simply lacks. This new set of "
			     "defaults will fix<br>that by adding it in. "
			     "You can re-configure things now to your<br>"
			     "liking. Sorry for the inconvenience.<br>"));
          }

        /* Ardvarks */
        else if (exebuf_conf->version > MOD_CONFIG_FILE_VERSION) 
          {
             /* config too new...wtf ? */
             _exebuf_conf_free();
	     ecore_timer_add(1.0, _exebuf_conf_timer, 
			     D_("Your Exebuf Module configuration is NEWER "
			     "than the module version. This is "
			     "very<br>strange. This should not happen unless"
			     " you downgraded<br>the module or "
			     "copied the configuration from a place where"
			     "<br>a newer version of the module "
			     "was running. This is bad and<br>as a "
			     "precaution your configuration has been now "
			     "restored to<br>defaults. Sorry for the "
			     "inconvenience.<br>"));
          }
     }

   /* if we don't have a config yet, or it got erased above, 
    * then create a default one */
   if (!exebuf_conf) _exebuf_conf_new();

   /* create a link from the modules config to the module
    * this is not written */
   exebuf_conf->module = m;

   e_exebuf_init();
   /* add module supplied action */
   act = e_action_add("exebuf");
   if (act)
     {
	act->func.go = _exebuf_action_exebuf_cb;
	e_action_predef_name_set(D_("Launch"), D_("Run Command Dialog"), "exebuf",
				 NULL, NULL, 0);
     }
   maug = e_int_menus_menu_augmentation_add_sorted
     ("main/1", D_("Run Command"), _exebuf_menu_add, NULL, NULL, NULL);

   e_configure_registry_category_add("advanced", 80, D_("Advanced"), NULL, "preferences-advanced");
   e_configure_registry_item_add("advanced/run_command", 40, D_("Run Command"), NULL, "system-run", exebuf_config_dialog);

   /* Give E the module */
   return m;
}

/*
 * Function to unload the module
 */
EAPI int 
e_modapi_shutdown(E_Module *m) 
{
   /* remove module-supplied menu additions */
   if (maug)
     {
	e_int_menus_menu_augmentation_del("main/1", maug);
	maug = NULL;
     }
   /* remove module-supplied action */
   if (act)
     {
	e_action_predef_name_del(D_("Launch"), D_("Run Command Dialog"));
	e_action_del("exebuf");
	act = NULL;
     }
   e_exebuf_shutdown();

   e_configure_registry_item_del("advanced/run_command");
   e_configure_registry_category_del("advanced");

   /* Kill the config dialog */
   if (exebuf_conf->cfd) e_object_del(E_OBJECT(exebuf_conf->cfd));
   exebuf_conf->cfd = NULL;

   /* Tell E the module is now unloaded. Gets removed from shelves, etc. */
   exebuf_conf->module = NULL;

   /* Cleanup the main config structure */
   _exebuf_conf_free();

   /* Clean EET */
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

/*
 * Function to Save the modules config
 */ 
EAPI int 
e_modapi_save(E_Module *m) 
{
   e_config_domain_save("module.exebuf", conf_edd, exebuf_conf);
   return 1;
}

/* new module needs a new config :), or config too old and we need one anyway */
static void 
_exebuf_conf_new(void) 
{
   char buf[128];

   exebuf_conf = E_NEW(Config, 1);
   exebuf_conf->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) if ((exebuf_conf->version & 0xffff) < v) {
#define IFMODCFGEND }

   /* setup defaults */
   IFMODCFG(0x008d);
   exebuf_conf->max_exe_list = 20;
   exebuf_conf->max_eap_list = 20;
   exebuf_conf->max_hist_list = 20;
   exebuf_conf->scroll_animate = 1;
   exebuf_conf->scroll_speed = 0.1000000000000000055511151;
   exebuf_conf->pos_align_x = 0.5000000000000000000000000;
   exebuf_conf->pos_align_y = 0.5000000000000000000000000;
   exebuf_conf->pos_size_w = 0.7500000000000000000000000;
   exebuf_conf->pos_size_h = 0.2500000000000000000000000;
   exebuf_conf->pos_min_w = 200;
   exebuf_conf->pos_min_h = 160;
   exebuf_conf->pos_max_w = 400;
   exebuf_conf->pos_max_h = 320;
   exebuf_conf->term_cmd = eina_stringshare_add("xterm -hold -e");
   IFMODCFGEND;

   /* update the version */
   exebuf_conf->version = MOD_CONFIG_FILE_VERSION;

   /* setup limits on the config properties here (if needed) */

   /* save the config to disk */
   e_config_save_queue();
}

/* This is called when we need to cleanup the actual configuration,
 * for example when our configuration is too old */
static void 
_exebuf_conf_free(void)
{
   /* cleanup any stringshares here */

   if (exebuf_conf->term_cmd) eina_stringshare_del(exebuf_conf->term_cmd);

   E_FREE(exebuf_conf);
}

/* timer for the config oops dialog (old configuration needs update) */
static Eina_Bool 
_exebuf_conf_timer(void *data) 
{
   e_util_dialog_internal(D_("Exebuf Configuration Updated"), data);

   return EINA_FALSE;
}

/* action callback */
static void
_exebuf_action_exebuf_cb(E_Object *obj, const char *params)
{
   E_Zone *zone = NULL;
   
   if (obj)
     {
	if (obj->type == E_MANAGER_TYPE)
	  zone = e_util_zone_current_get((E_Manager *)obj);
	else if (obj->type == E_CONTAINER_TYPE)
	  zone = e_util_zone_current_get(((E_Container *)obj)->manager);
	else if (obj->type == E_ZONE_TYPE)
	  zone = e_util_zone_current_get(((E_Zone *)obj)->container->manager);
	else
	  zone = e_util_zone_current_get(e_manager_current_get());
     }
   if (!zone) zone = e_util_zone_current_get(e_manager_current_get());
   if (zone) e_exebuf_show(zone);
}

/* menu item callback(s) */
static Eina_Bool
_exebuf_run_defer_cb(void *data)
{
   E_Zone *zone;
   
   zone = data;
   if (zone) e_exebuf_show(zone);
   return EINA_FALSE;
}

static void 
_exebuf_run_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   ecore_idle_enterer_add(_exebuf_run_defer_cb, m->zone);
}

/* menu item add hook */
static void
_exebuf_menu_add(void *data, E_Menu *m)
{
   E_Menu_Item *mi;
   
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, D_("Run Command"));
   e_util_menu_item_theme_icon_set(mi, "system-run");
   e_menu_item_callback_set(mi, _exebuf_run_cb, NULL);
}
