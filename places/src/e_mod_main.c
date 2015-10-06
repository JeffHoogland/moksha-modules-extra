#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <e.h>
#include <E_DBus.h>
#include "e_mod_main.h"
#include "e_mod_places.h"

/* Local Function Prototypes */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static void _gc_id_del(const E_Gadcon_Client_Class *client_class, const char *id);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);

static void _places_conf_new(void);
static void _places_conf_free(void);
static Eina_Bool _places_conf_timer(void *data);
static Config_Item *_places_conf_item_get(const char *id);
static void _places_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _places_cb_menu_post(void *data, E_Menu *menu);
static void _places_cb_menu_configure(void *data, E_Menu *mn, E_Menu_Item *mi);

/* Local Variables */
static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;
static E_Int_Menu_Augmentation *maug = NULL;
Eina_List *instances = NULL;
Config *places_conf = NULL;
extern Eina_List *volumes;

static const E_Gadcon_Client_Class _gc_class =
{
   GADCON_CLIENT_CLASS_VERSION, "Places",
     {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon,
          _gc_id_new, _gc_id_del, e_gadcon_site_is_not_toolbar},
   E_GADCON_CLIENT_STYLE_INSET
};

EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Places"};

/* Module Functions */
EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[PATH_MAX];

   /* Set up module's message catalogue */
   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   /* Location of theme to load for this module */
   snprintf(buf, sizeof(buf), "%s/e-module-places.edj", m->dir);

   /* Display this Modules config info in the main Config Panel */
   e_configure_registry_category_add("fileman", 80, D_("Files"),
                                     NULL, "system-file-manager");
   e_configure_registry_item_add("fileman/places", 80, D_("Places"),
                                 NULL, buf, e_int_config_places_module);

   /* Define EET Data Storage */
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
   E_CONFIG_VAL(D, T, fm, STR);
   E_CONFIG_VAL(D, T, auto_mount, UCHAR);
   E_CONFIG_VAL(D, T, boot_mount, UCHAR);
   E_CONFIG_VAL(D, T, auto_open, UCHAR);
   E_CONFIG_LIST(D, T, conf_items, conf_item_edd);
   E_CONFIG_VAL(D, T, show_menu, UCHAR);
   E_CONFIG_VAL(D, T, hide_header, UCHAR);
   E_CONFIG_VAL(D, T, show_home, UCHAR);
   E_CONFIG_VAL(D, T, show_desk, UCHAR);
   E_CONFIG_VAL(D, T, show_trash, UCHAR);
   E_CONFIG_VAL(D, T, show_root, UCHAR);
   E_CONFIG_VAL(D, T, show_temp, UCHAR);
   E_CONFIG_VAL(D, T, show_bookm, UCHAR);

   /* Tell E to find any existing module data. First run ? */
   places_conf = e_config_domain_load("module.places", conf_edd);
   if (places_conf)
     {
        if ((places_conf->version >> 16) < MOD_CONFIG_FILE_EPOCH)
          {
             /* config too old */
             _places_conf_free();
             ecore_timer_add(1.0, _places_conf_timer,
			     "Places Module Configuration data needed "
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
        else if (places_conf->version > MOD_CONFIG_FILE_VERSION)
          {
             /* config too new...wtf ? */
             _places_conf_free();
             ecore_timer_add(1.0, _places_conf_timer,
                 "Your Places Module configuration is NEWER "
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
   if (!places_conf) _places_conf_new();

   places_conf->module = m;
   e_gadcon_provider_register(&_gc_class);
   places_init();

   places_menu_augmentation();

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   places_shutdown();

   e_configure_registry_item_del("fileman/places");
   e_configure_registry_category_del("fileman");

   /* Kill the config dialog */
   if (places_conf->cfd) e_object_del(E_OBJECT(places_conf->cfd));
   places_conf->cfd = NULL;

   /* Remove the main menu item */
   if (maug)
     {
        e_int_menus_menu_augmentation_del("main/1", maug);
        maug = NULL;
     }

   /* Tell E the module is now unloaded. Gets removed from shelves, etc. */
   places_conf->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);

   _places_conf_free();

   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   e_config_domain_save("module.places", conf_edd, places_conf);
   return 1;
}

void
places_menu_augmentation(void)
{
   if (places_conf->show_menu && (!maug))
     maug = e_int_menus_menu_augmentation_add("main/1", places_augmentation,
                                              NULL, NULL, NULL);
   else if ((!places_conf->show_menu) && maug)
     {
        e_int_menus_menu_augmentation_del("main/1", maug);
        maug = NULL;
     }
}

/* Gadcon Functions */
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Instance *inst = NULL;

   inst = E_NEW(Instance, 1);
   inst->conf_item = _places_conf_item_get(id);

   inst->o_box = e_box_add(gc->evas);
   e_box_homogenous_set(inst->o_box, 0);
   e_box_orientation_set(inst->o_box, 0);
   e_box_align_set(inst->o_box, 0.0, 0.0);

   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->o_box);
   inst->gcc->data = inst;

   evas_object_event_callback_add(inst->o_box, EVAS_CALLBACK_MOUSE_DOWN,
                                  _places_cb_mouse_down, inst);

   instances = eina_list_append(instances, inst);
   places_fill_box(inst->o_box);

   return inst->gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst = NULL;

   if (!(inst = gcc->data)) return;
   instances = eina_list_remove(instances, inst);

   if (inst->menu)
     {
        e_menu_post_deactivate_callback_set(inst->menu, NULL, NULL);
        e_object_del(E_OBJECT(inst->menu));
        inst->menu = NULL;
     }

   if (inst->o_box)
     {
        evas_object_event_callback_del(inst->o_box, EVAS_CALLBACK_MOUSE_DOWN,
                                       _places_cb_mouse_down);
        places_empty_box(inst->o_box);
        evas_object_del(inst->o_box);
     }

   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   Instance *inst;
   Volume *v;
   Eina_List *l;
   int count = 1;

   EINA_LIST_FOREACH(places_volume_list_get(), l, v)
      if (v->valid) count++;

   inst = gcc->data;
   switch (orient)
     {
      case E_GADCON_ORIENT_HORIZ:
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
        // TODO get sizes from the theme
        e_gadcon_client_aspect_set(gcc, 100 * count, 60);
        e_gadcon_client_min_size_set(gcc, 100 * count, 60);
        e_box_orientation_set(inst->o_box, 1);
        break;
      case E_GADCON_ORIENT_FLOAT:
      case E_GADCON_ORIENT_VERT:
      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_LB:
      case E_GADCON_ORIENT_CORNER_RB:
        // TODO get sizes from the theme
        e_gadcon_client_aspect_set(gcc, 200, 50 * count);
        e_gadcon_client_min_size_set(gcc, 200, 50 * count);
        e_box_orientation_set(inst->o_box, 0);
        break;
      default:
        break;
     }

   places_fill_box(inst->o_box);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class)
{
   return D_("Places");
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   Config_Item *ci = NULL;

   ci = _places_conf_item_get(NULL);
   return ci->id;
}

static void
_gc_id_del(const E_Gadcon_Client_Class *client_class, const char *id)
{
   // Config_Item *ci = NULL;

   // if (!(ci = _places_conf_item_get(id))) return;

   // if (ci->id) eina_stringshare_del(ci->id);

   // places_conf->conf_items = eina_list_remove(places_conf->conf_items, ci);
   // E_FREE(ci);
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas)
{
   Evas_Object *o = NULL;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/e-module-places.edj", places_conf->module->dir);
   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

/* Conf functions */
static void
_places_conf_new(void)
{
   places_conf = E_NEW(Config, 1);
   places_conf->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) if ((places_conf->version & 0xffff) < v) {
#define IFMODCFGEND }

   /* setup defaults */
   IFMODCFG(0x008d);
   places_conf->auto_mount = 0;
   places_conf->auto_open = 0;
   places_conf->show_home = 1;
   places_conf->hide_header = 0;
   places_conf->show_desk = 1;
   places_conf->show_trash = 0;
   places_conf->show_root = 0;
   places_conf->show_temp = 0;
   places_conf->show_bookm = 1;
   _places_conf_item_get(NULL);
   IFMODCFGEND;

   IFMODCFG(0x008e);
   places_conf->fm = NULL;
   IFMODCFGEND;

   /* update the version */
   places_conf->version = MOD_CONFIG_FILE_VERSION;

   /* setup limits on the config properties here (if needed) */

   /* save the config to disk */
   e_config_save_queue();
}

static void
_places_conf_free(void)
{
   while (places_conf->conf_items)
     {
        Config_Item *ci = NULL;

        ci = places_conf->conf_items->data;
        places_conf->conf_items =
          eina_list_remove_list(places_conf->conf_items,
                                places_conf->conf_items);
        if (ci->id) eina_stringshare_del(ci->id);
        E_FREE(ci);
     }

   if (places_conf->fm) eina_stringshare_del(places_conf->fm);
   E_FREE(places_conf);
}

static Eina_Bool
_places_conf_timer(void *data)
{
   e_util_dialog_internal(D_("Places Configuration Updated"), data);
   return EINA_FALSE;
}

static Config_Item *
_places_conf_item_get(const char *id)
{
   Config_Item *ci;

   GADCON_CLIENT_CONFIG_GET(Config_Item, places_conf->conf_items, _gc_class, id);

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->switch2 = 0;
   places_conf->conf_items = eina_list_append(places_conf->conf_items, ci);
   return ci;
}

static void
_places_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event)
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
        e_menu_item_callback_set(mi, _places_cb_menu_configure, NULL);

        /* Each Gadget Client has a utility menu from the Container */
        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
        e_menu_post_deactivate_callback_set(m, _places_cb_menu_post, inst);
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

static void
_places_cb_menu_post(void *data, E_Menu *menu)
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   if (!inst->menu) return;
   e_object_del(E_OBJECT(inst->menu));
   inst->menu = NULL;
}

static void
_places_cb_menu_configure(void *data, E_Menu *mn, E_Menu_Item *mi)
{
   if (!places_conf) return;
   if (places_conf->cfd) return;
   e_int_config_places_module(mn->zone->container, NULL);
}
