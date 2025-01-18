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
static void _diskio_cb_menu_configure(void *data, E_Menu *mn, E_Menu_Item *mi);
static Eina_Bool  _diskio_set(void *data);

/* Local Structures */
typedef struct _Instance Instance;
struct _Instance 
{
   E_Gadcon_Client *gcc;
   Evas_Object *o_diskio;
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
   char buf[PATH_MAX];

   /* Set up module locales*/
   bindtextdomain(LOCALEDOMAIN, LOCALEDIR);
   bind_textdomain_codeset(LOCALEDOMAIN, "UTF-8");

   snprintf(buf, sizeof(buf), "%s/e-module-diskio.edj", m->dir);

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

   diskio_conf = e_config_domain_load("module.diskio", conf_edd);
   if (diskio_conf) 
     {
        if ((diskio_conf->version >> 16) < MOD_CONFIG_FILE_EPOCH) 
          {
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
        else if (diskio_conf->version > MOD_CONFIG_FILE_VERSION) 
          {
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
   if (!diskio_conf) _diskio_conf_new();
   diskio_conf->module = m;
   e_gadcon_provider_register(&_gc_class);

   return m;
}

EAPI int 
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   if (diskio_conf->cfd) e_object_del(E_OBJECT(diskio_conf->cfd));
   diskio_conf->cfd = NULL;

   diskio_conf->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);

   while (diskio_conf->conf_items) 
     {
        Config_Item *ci = NULL;

        ci = diskio_conf->conf_items->data;
        diskio_conf->conf_items = 
          eina_list_remove_list(diskio_conf->conf_items, 
                                diskio_conf->conf_items);
        if (ci->id) eina_stringshare_del(ci->id);
        E_FREE(ci);
     }

   E_FREE(diskio_conf);

   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int 
e_modapi_save(E_Module *m __UNUSED__)
{
   e_config_domain_save("module.diskio", conf_edd, diskio_conf);
   return 1;
}

/* Local Functions */

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   Instance *inst = NULL;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/e-module-diskio.edj", 
            diskio_conf->module->dir);

   inst = E_NEW(Instance, 1);
   inst->conf_item = _diskio_conf_item_get(id);
   if (inst->conf_item->disk) inst->conf_item->disk = eina_stringshare_add(inst->conf_item->disk);

   inst->o_diskio = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(inst->o_diskio, "base/theme/modules/diskio", 
                                "modules/diskio/main"))
     edje_object_file_set(inst->o_diskio, buf, "modules/diskio/main");

   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->o_diskio);
   inst->gcc->data = inst;

   evas_object_event_callback_add(inst->o_diskio, EVAS_CALLBACK_MOUSE_DOWN, 
                                  _diskio_cb_mouse_down, inst);

   instances = eina_list_append(instances, inst);

   inst->timer = ecore_timer_add(0.1, _diskio_set, inst);

   return inst->gcc;
}

/* Set disk i/o */
static Eina_Bool
_diskio_set(void *data)
{
   FILE *statfile;
   Instance *inst = NULL;

   char buffer[128];
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

   if (fgets(buffer, sizeof(buffer), statfile)!=NULL)
       sscanf(buffer, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &dummy0, 
       &dummy1, &bytes_r_new, &dummy2, &dummy3, &dummy4, &bytes_w_new, &dummy5, &dummy6, &dummy7, &dummy8);

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

static void 
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst = NULL;

   if (!(inst = gcc->data)) return;
   instances = eina_list_remove(instances, inst);

   if (inst->timer) ecore_timer_del(inst->timer);

   if (inst->o_diskio) 
     {
        evas_object_event_callback_del(inst->o_diskio, EVAS_CALLBACK_MOUSE_DOWN, 
                                       _diskio_cb_mouse_down);
        evas_object_del(inst->o_diskio);
     }
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
   return D_("DiskIO");
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   Config_Item *ci = NULL;

   ci = _diskio_conf_item_get(NULL);
   return ci->id;
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class __UNUSED__, Evas *evas)
{
   Evas_Object *o = NULL;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/e-module-diskio.edj", diskio_conf->module->dir);

   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static void 
_diskio_conf_new(void)
{
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
   while (diskio_conf->conf_items) 
     {
        Config_Item *ci = NULL;

        ci = diskio_conf->conf_items->data;
        diskio_conf->conf_items = 
          eina_list_remove_list(diskio_conf->conf_items, 
                                diskio_conf->conf_items);
        if (ci->id) eina_stringshare_del(ci->id);
        if (ci->disk) eina_stringshare_del(ci->disk);
        E_FREE(ci);
     }

   E_FREE(diskio_conf);
}

static Eina_Bool 
_diskio_conf_timer(void *data)
{
   e_util_dialog_internal( D_("DiskIO Configuration Updated"), data);
   return EINA_FALSE;
}

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

static void 
_diskio_cb_mouse_down(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event)
{
   Instance *inst = NULL;
   Evas_Event_Mouse_Down *ev;
   E_Zone *zone = NULL;
   E_Menu_Item *mi = NULL;
   int x, y;

   if (!(inst = data)) return;
   ev = event;

   if (ev->button == 3)
     {
        E_Menu *m;

        zone = e_util_zone_current_get(e_manager_current_get());

        m = e_menu_new();
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Settings"));
        e_util_menu_item_theme_icon_set(mi, "preferences-system");
        e_menu_item_callback_set(mi, _diskio_cb_menu_configure, inst);

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, 
                                          NULL, NULL);

        e_menu_activate_mouse(m, zone, (x + ev->output.x), 
                              (y + ev->output.y), 1, 1, 
                              E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button, 
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void 
_diskio_cb_menu_configure(void *data, E_Menu *mn, E_Menu_Item *mi __UNUSED__)
{
   Instance *inst = NULL;

   inst = data;
   if (!inst) return;
   if (!diskio_conf) return;
   if (diskio_conf->cfd) return;
   e_int_config_diskio_module(mn->zone->container, inst->conf_item);
}
