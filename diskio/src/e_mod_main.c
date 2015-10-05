#include <e.h>
#include "e_mod_main.h"

/* Local Function Prototypes */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);

static void _diskio_conf_new(void);
static void _diskio_conf_free(void);
static Eina_Bool _diskio_conf_timer(void *data);
static Config_Item *_diskio_conf_item_get(const char *id);
static void _diskio_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _diskio_cb_menu_post(void *data, E_Menu *menu);
static void _diskio_cb_menu_configure(void *data, E_Menu *mn, E_Menu_Item *mi);
static Eina_Bool  _diskio_set(void *data);

/* Local Structures */
typedef struct _Instance Instance;
struct _Instance 
{
   /* pointer to this gadget's container */
   E_Gadcon_Client *gcc;

   /* evas_object used to display */
   Evas_Object *o_diskio;

   /* popup anyone ? */
   E_Menu *menu;

   /* Config_Item structure. Every gadget should have one :) */
   Config_Item *conf_item;

   /* diskstat update timer */
   Ecore_Timer *timer;
   unsigned long bytes_r;
   unsigned long bytes_w;
};

/* Local Variables */
static Eina_List *instances = NULL;
static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;
Config *diskio_conf = NULL;

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "diskio", 
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, 
      _gc_id_new, NULL, NULL
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "DiskIO"};

/*
 * Module Functions
 */
EAPI void *
e_modapi_init(E_Module *m) 
{
   char buf[4096];

   /* Location of theme to load for this module */
   snprintf(buf, sizeof(buf), "%s/e-module-diskio.edj", m->dir);

   /* Define EET Data Storage */

   conf_item_edd = E_CONFIG_DD_NEW("Config_Item", Config_Item);
   #undef T
   #undef D
   #define T Config_Item
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, disk, STR);

   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_LIST(D, T, conf_items, conf_item_edd); /* the list */

   /* Tell E to find any existing module data. First run ? */
   diskio_conf = e_config_domain_load("module.diskio", conf_edd);
   if (diskio_conf) 
     {
        /* Check config version */
        if ((diskio_conf->version >> 16) < MOD_CONFIG_FILE_EPOCH) 
          {
             /* config too old */
             _diskio_conf_free();
			 ecore_timer_add(1.0, _diskio_conf_timer,
							 "DiskIO Module Configuration data needed "
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
							 "liking. Sorry for the inconvenience.<br>");
          }

        /* Ardvarks */
        else if (diskio_conf->version > MOD_CONFIG_FILE_VERSION) 
          {
             /* config too new...wtf ? */
             _diskio_conf_free();
			 ecore_timer_add(1.0, _diskio_conf_timer, 
							 "Your DiskIO Module configuration is NEWER "
							 "than the module version. This is "
							 "very<br>strange. This should not happen unless"
							 " you downgraded<br>the module or "
							 "copied the configuration from a place where"
							 "<br>a newer version of the module "
							 "was running. This is bad and<br>as a "
							 "precaution your configuration has been now "
							 "restored to<br>defaults. Sorry for the "
							 "inconvenience.<br>");
          }
     }

   /* if we don't have a config yet, or it got erased above, 
    * then create a default one */
   if (!diskio_conf) _diskio_conf_new();

   /* create a link from the modules config to the module
    * this is not written */
   diskio_conf->module = m;

   /* Tell any gadget containers (shelves, etc) that we provide a module
    * for the user to enjoy */
   e_gadcon_provider_register(&_gc_class);

   /* Give E the module */
   return m;
}

/*
 * Function to unload the module
 */
EAPI int 
e_modapi_shutdown(E_Module *m) 
{
   /* Kill the config dialog */
   if (diskio_conf->cfd) e_object_del(E_OBJECT(diskio_conf->cfd));
   diskio_conf->cfd = NULL;

   /* Tell E the module is now unloaded. Gets removed from shelves, etc. */
   diskio_conf->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);

   /* Cleanup our item list */
   while (diskio_conf->conf_items) 
     {
        Config_Item *ci = NULL;

        /* Grab an item from the list */
        ci = diskio_conf->conf_items->data;
        /* remove it */
        diskio_conf->conf_items = 
          eina_list_remove_list(diskio_conf->conf_items, 
                                diskio_conf->conf_items);
        /* cleanup stringshares !! ) */
        if (ci->id) eina_stringshare_del(ci->id);
        /* keep the planet green */
        E_FREE(ci);
     }

   /* Cleanup the main config structure */
   E_FREE(diskio_conf);

   /* Clean EET */
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

/*
 * Function to Save the modules config
 */ 
EAPI int 
e_modapi_save(E_Module *m) 
{
   e_config_domain_save("module.diskio", conf_edd, diskio_conf);
   return 1;
}

/* Local Functions */

/* Called when Gadget_Container says go */
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   Instance *inst = NULL;
   char buf[4096];

   /* theme file */
   snprintf(buf, sizeof(buf), "%s/e-module-diskio.edj", 
            diskio_conf->module->dir);

   /* New visual instance, any config ? */
   inst = E_NEW(Instance, 1);
   inst->conf_item = _diskio_conf_item_get(id);
   if (inst->conf_item->disk) inst->conf_item->disk = eina_stringshare_add(inst->conf_item->disk);

   /* create on-screen object */
   inst->o_diskio = edje_object_add(gc->evas);
   /* we have a theme ? */
   if (!e_theme_edje_object_set(inst->o_diskio, "base/theme/modules/diskio", 
                                "modules/diskio/main"))
     edje_object_file_set(inst->o_diskio, buf, "modules/diskio/main");

   /* Start loading our module on screen via container */
   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->o_diskio);
   inst->gcc->data = inst;

   /* hook a mouse down. we want/have a popup menu, right ? */
   evas_object_event_callback_add(inst->o_diskio, EVAS_CALLBACK_MOUSE_DOWN, 
                                  _diskio_cb_mouse_down, inst);

   /* add to list of running instances so we can cleanup later */
   instances = eina_list_append(instances, inst);

   inst->timer = ecore_timer_add(0.1, _diskio_set, inst);

   /* return the Gadget_Container Client */
   return inst->gcc;
}

/* Set disk i/o */
static Eina_Bool
_diskio_set(void *data)
{
   FILE *statfile;
   Instance *inst = NULL;

   char buffer[128];
   char dummy[16];
   char path[128];
   unsigned long dummy0, dummy1, dummy2, dummy3, dummy4, dummy5,
				 dummy6, dummy7, dummy8, bytes_r_new=0, bytes_w_new=0;

   inst = data;
   if (!inst) return EINA_TRUE;
   if (!inst->conf_item) return EINA_TRUE;

   edje_object_part_text_set(inst->o_diskio, "e.text.display_name",
                             inst->conf_item->disk);

   snprintf (path, sizeof (path), "/sys/block/%s/stat", inst->conf_item->disk);
   statfile = fopen(path, "r");
   if (!statfile)
	 { 
        edje_object_signal_emit(inst->o_diskio, "read,off", "");
        edje_object_signal_emit(inst->o_diskio, "write,off", "");
		return EINA_TRUE;
	 }

   fgets(buffer, sizeof(buffer), statfile);
   sscanf(buffer, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &dummy0, &dummy1, &bytes_r_new, &dummy2, &dummy3, &dummy4, &bytes_w_new, &dummy5, &dummy6, &dummy7, &dummy8);

   fclose (statfile);

   if (!bytes_r_new && !bytes_w_new)
     {
        edje_object_signal_emit(inst->o_diskio, "read,off", "");
        edje_object_signal_emit(inst->o_diskio, "write,off", "");
	    return EINA_TRUE;
	 }

   if (inst->bytes_r != bytes_r_new)
     {
        edje_object_signal_emit(inst->o_diskio, "read,on", "");
		inst->bytes_r = bytes_r_new;
     }
   else
     edje_object_signal_emit(inst->o_diskio, "read,off", "");

   if (inst->bytes_w != bytes_w_new)
     {
		edje_object_signal_emit(inst->o_diskio, "write,on", "");
		inst->bytes_w = bytes_w_new;
     }
   else
     edje_object_signal_emit(inst->o_diskio, "write,off", "");

   return EINA_TRUE;
}

/* Called when Gadget_Container says stop */
static void 
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst = NULL;

   if (!(inst = gcc->data)) return;
   instances = eina_list_remove(instances, inst);

	/* Delete diskstat update timer */
   if (inst->timer) ecore_timer_del(inst->timer);

   /* kill popup menu */
   if (inst->menu) 
     {
        e_menu_post_deactivate_callback_set(inst->menu, NULL, NULL);
        e_object_del(E_OBJECT(inst->menu));
        inst->menu = NULL;
     }
   /* delete the visual */
   if (inst->o_diskio) 
     {
        /* remove mouse down callback hook */
        evas_object_event_callback_del(inst->o_diskio, EVAS_CALLBACK_MOUSE_DOWN, 
                                       _diskio_cb_mouse_down);
        evas_object_del(inst->o_diskio);
     }
   E_FREE(inst);
}

/* For for when container says we are changing position */
static void 
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient) 
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

/* Gadget/Module label */
static const char *
_gc_label(const E_Gadcon_Client_Class *client_class) 
{
   return D_("DiskIO");
}

/* so E can keep a unique instance per-container */
static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class) 
{
   Config_Item *ci = NULL;

   ci = _diskio_conf_item_get(NULL);
   return ci->id;
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas) 
{
   Evas_Object *o = NULL;
   char buf[4096];

   /* theme */
   snprintf(buf, sizeof(buf), "%s/e-module-diskio.edj", diskio_conf->module->dir);

   /* create icon object */
   o = edje_object_add(evas);

   /* load icon from theme */
   edje_object_file_set(o, buf, "icon");

   return o;
}

/* new module needs a new config :), or config too old and we need one anyway */
static void 
_diskio_conf_new(void) 
{
   Config_Item *ci = NULL;
   char buf[128];

   diskio_conf = E_NEW(Config, 1);
   diskio_conf->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) if ((diskio_conf->version & 0xffff) < v) {
#define IFMODCFGEND }

   /* setup defaults */
   IFMODCFG(0x008d);
   _diskio_conf_item_get(NULL);
   IFMODCFGEND;

   /* update the version */
   diskio_conf->version = MOD_CONFIG_FILE_VERSION;

   /* setup limits on the config properties here (if needed) */

   /* save the config to disk */
   e_config_save_queue();
}

static void 
_diskio_conf_free(void) 
{
   /* cleanup any stringshares here */
   while (diskio_conf->conf_items) 
     {
        Config_Item *ci = NULL;

        ci = diskio_conf->conf_items->data;
        diskio_conf->conf_items = 
          eina_list_remove_list(diskio_conf->conf_items, 
                                diskio_conf->conf_items);
        /* EPA */
        if (ci->id) eina_stringshare_del(ci->id);
        if (ci->disk) eina_stringshare_del(ci->disk);
        E_FREE(ci);
     }

   E_FREE(diskio_conf);
}

/* timer for the config oops dialog */
static Eina_Bool 
_diskio_conf_timer(void *data) 
{
   e_util_dialog_show( D_("DiskIO Configuration Updated"), data);
   return EINA_FALSE;
}

/* function to search for any Config_Item struct for this Item
 * create if needed */
static Config_Item *
_diskio_conf_item_get(const char *id) 
{
   Config_Item *ci;

   GADCON_CLIENT_CONFIG_GET(Config_Item, diskio_conf->conf_items, _gc_class, id);

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->disk = eina_stringshare_add("???");
   diskio_conf->conf_items = eina_list_append(diskio_conf->conf_items, ci);
   return ci;
}

/* Pants On */
static void 
_diskio_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event) 
{
   Instance *inst = NULL;
   Evas_Event_Mouse_Down *ev;
   E_Zone *zone = NULL;
   E_Menu_Item *mi = NULL;
   int x, y;

   if (!(inst = data)) return;
   ev = event;

   if ((ev->button == 3) && (!inst->menu)) 
     {
        E_Menu *m;

        /* grab current zone */
        zone = e_util_zone_current_get(e_manager_current_get());

        /* create popup menu */
        m = e_menu_new();
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Settings"));
        e_util_menu_item_theme_icon_set(mi, "preferences-system");
        e_menu_item_callback_set(mi, _diskio_cb_menu_configure, inst);

        /* Each Gadget Client has a utility menu from the Container */
        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
        e_menu_post_deactivate_callback_set(m, _diskio_cb_menu_post, inst);
        inst->menu = m;

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, 
                                          NULL, NULL);

        /* show the menu relative to gadgets position */
        e_menu_activate_mouse(m, zone, (x + ev->output.x), 
                              (y + ev->output.y), 1, 1, 
                              E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button, 
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

/* popup menu closing, cleanup */
static void 
_diskio_cb_menu_post(void *data, E_Menu *menu) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   if (!inst->menu) return;
   e_object_del(E_OBJECT(inst->menu));
   inst->menu = NULL;
}

/* call configure from popup */
static void 
_diskio_cb_menu_configure(void *data, E_Menu *mn, E_Menu_Item *mi) 
{
   Instance *inst = NULL;

   inst = data;
   if (!inst) return;
   if (!diskio_conf) return;
   if (diskio_conf->cfd) return;
   e_int_config_diskio_module(mn->zone->container, inst->conf_item);
}
