#include <e.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include "e_mod_main.h"

typedef struct _Instance Instance;
typedef struct _Slideshow Slideshow;

struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object *slide_obj;
   Slideshow *slide;
   Ecore_Timer *check_timer;
   Eina_List *bg_list;
   const char *display;
   int index, bg_id, bg_count;
   Config_Item *ci;
};

struct _Slideshow
{
   Instance *inst;
   Evas_Object *slide_obj, *bg_obj, *img_obj;
};

static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static void _slide_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _slide_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi);
static void _slide_menu_cb_post(void *data, E_Menu *m);
static Config_Item *_slide_config_item_get(const char *id);
static Slideshow *_slide_new(Evas *evas);
static void _slide_free(Slideshow *ss);
static Eina_Bool _slide_cb_check(void *data);
static void _slide_get_bg_count(void *data);
static void _slide_set_bg(void *data, const char *bg);
static void _slide_set_preview(void *data);
static void _slide_get_bg_subdirs(void *data, char *local_path);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *slide_config = NULL;

static const E_Gadcon_Client_Class _gc_class =
{
   GADCON_CLIENT_CLASS_VERSION, "slideshow", 
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
   Slideshow *slide;

   inst = E_NEW(Instance, 1);

   inst->ci = _slide_config_item_get(id);

   slide = _slide_new(gc->evas);
   slide->inst = inst;
   inst->slide = slide;

   o = slide->slide_obj;
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->slide_obj = o;

   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, 
                                  _slide_cb_mouse_down, inst);
   slide_config->instances = eina_list_append(slide_config->instances, inst);

   if (!inst->ci->disable_timer)
     inst->check_timer = ecore_timer_add(inst->ci->poll_time, _slide_cb_check, inst);
   else
     {
	_slide_get_bg_count(inst);
	if(inst->ci->random_order)
	  {
             srand(time(NULL) + inst->gcc->id * 100);
	     inst->index = (rand() % (inst->bg_count+1));
	  }
	else
	  inst->index = 0;

	_slide_set_preview(inst);
     }
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;
   Slideshow *slide;
   char *item;

   inst = gcc->data;
   slide = inst->slide;

   EINA_LIST_FREE(inst->bg_list, item)
     free(item);

   if (inst->display) eina_stringshare_del(inst->display);
   if (inst->check_timer) ecore_timer_del(inst->check_timer);

   slide_config->instances = eina_list_remove(slide_config->instances, inst);

   evas_object_event_callback_del(slide->slide_obj, EVAS_CALLBACK_MOUSE_DOWN, 
                                  _slide_cb_mouse_down);

   _slide_free(slide);
   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class)
{
   return D_("Slideshow");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas)
{
   Evas_Object *o;
   char buf[PATH_MAX];

   o = edje_object_add(evas);
   snprintf(buf, sizeof (buf), "%s/e-module-slideshow.edj", 
	    e_module_dir_get (slide_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   Config_Item *ci;

   ci = _slide_config_item_get(NULL);
   return ci->id;
}

static void
_slide_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;
   if ((ev->button == 3) && (!slide_config->menu))
     {
	E_Menu *m;
	E_Menu_Item *mi;
	int x, y, w, h;

	m = e_menu_new();
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, D_("Settings"));
	e_util_menu_item_theme_icon_set(mi, "preferences-system");
	e_menu_item_callback_set(mi, _slide_menu_cb_configure, inst);

	m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
	e_menu_post_deactivate_callback_set(m, _slide_menu_cb_post, inst);
	slide_config->menu = m;

	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, &w, &h);
	e_menu_activate_mouse(m,
			       e_util_zone_current_get(e_manager_current_get()), 
			       x + ev->output.x, y + ev->output.y, 1, 1,
			       E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
     }
   else if (ev->button == 2)
     {
	if (inst->ci->disable_timer) return;
	if (inst->check_timer)
	  ecore_timer_del(inst->check_timer);
	else
	  inst->check_timer = ecore_timer_add(inst->ci->poll_time, 
                                              _slide_cb_check, inst);
     }
   else if (ev->button == 1)
     _slide_cb_check(inst);
}

static void
_slide_menu_cb_post(void *data, E_Menu *m)
{
   if (!slide_config->menu) return;
   e_object_del(E_OBJECT(slide_config->menu));
   slide_config->menu = NULL;
}

static void
_slide_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Instance *inst;

   inst = data;
   _config_slideshow_module(inst->ci);
}

void
_slide_config_updated(Config_Item *ci)
{
   Eina_List *l;
   Instance *inst;

   if (!slide_config) return;
   EINA_LIST_FOREACH(slide_config->instances, l, inst) 
     {
	if (inst->ci != ci) continue;
	if (inst->check_timer) ecore_timer_del(inst->check_timer);
	if ((inst->ci->disable_timer) || (inst->ci->poll_time == 0))
	  break;
	inst->check_timer = ecore_timer_add(inst->ci->poll_time, 
                                            _slide_cb_check, inst);
     }
}

static Config_Item *
_slide_config_item_get(const char *id)
{
   Eina_List *l;
   Config_Item *ci;
   char buf[PATH_MAX];

   if (!id)
     {
	int  num = 0;

	/* Create id */
	if (slide_config->items)
	  {
	     const char *p;

	     ci = eina_list_last(slide_config->items)->data;
	     p = strrchr(ci->id, '.');
	     if (p) num = atoi(p + 1) + 1;
	  }
	snprintf(buf, sizeof(buf), "%s.%d", _gc_class.name, num);
	id = buf;
     }
   else
     {
        EINA_LIST_FOREACH(slide_config->items, l, ci) 
          {
	     if (!ci->id) continue;
	     if (!strcmp(ci->id, id)) return ci;
	 }
     }

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->poll_time = 60.0;
   ci->disable_timer = 0;
   ci->all_desks = 0;
   snprintf(buf, sizeof (buf), "%s/.e/e/backgrounds", e_user_homedir_get());
   ci->dir = eina_stringshare_add(buf);

   slide_config->items = eina_list_append(slide_config->items, ci);
   return ci;
}

EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION, "Slideshow"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   conf_item_edd = E_CONFIG_DD_NEW("Slideshow_Config_Item", Config_Item);
#undef T
#undef D
#define T Config_Item
#define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, dir, STR);
   E_CONFIG_VAL(D, T, poll_time, DOUBLE);
   E_CONFIG_VAL(D, T, disable_timer, INT);
   E_CONFIG_VAL(D, T, random_order, INT);
   E_CONFIG_VAL(D, T, all_desks, INT);

   conf_edd = E_CONFIG_DD_NEW("Slideshow_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);

   slide_config = e_config_domain_load("module.slideshow", conf_edd);
   if (!slide_config)
     {
	Config_Item *ci;

	snprintf(buf, sizeof (buf), "%s/.e/e/backgrounds", e_user_homedir_get());
	slide_config = E_NEW(Config, 1);
	ci = E_NEW(Config_Item, 1);

	ci->id = eina_stringshare_add("0");
	ci->dir = eina_stringshare_add(buf);
	ci->poll_time = 60.0;
	ci->disable_timer = 0;
	ci->random_order = 0;
	ci->all_desks = 0;
	slide_config->items = eina_list_append(slide_config->items, ci);
     }
   slide_config->module = m;
   e_gadcon_provider_register(&_gc_class);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   slide_config->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);

   if (slide_config->config_dialog)
     e_object_del(E_OBJECT(slide_config->config_dialog));
   if (slide_config->menu)
     {
	e_menu_post_deactivate_callback_set(slide_config->menu, NULL, NULL);
	e_object_del(E_OBJECT(slide_config->menu));
	slide_config->menu = NULL;
     }
   while (slide_config->items)
     {
	Config_Item *ci;

	ci = slide_config->items->data;
	slide_config->items = eina_list_remove_list(slide_config->items, slide_config->items);
	if (ci->id) eina_stringshare_del(ci->id);
	if (ci->dir) eina_stringshare_del(ci->dir);
	E_FREE(ci);
     }
   E_FREE(slide_config);
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   e_config_domain_save("module.slideshow", conf_edd, slide_config);
   return 1;
}

static Slideshow *
_slide_new(Evas *evas)
{
   Slideshow *ss;
   char buf[PATH_MAX];

   ss = E_NEW(Slideshow, 1);
   snprintf(buf, sizeof (buf), "%s/slideshow.edj", 
            e_module_dir_get(slide_config->module));
   ss->img_obj = e_livethumb_add(evas);
   e_livethumb_vsize_set(ss->img_obj, 120, 120);
   evas_object_show(ss->img_obj);

   ss->slide_obj = edje_object_add(evas);
   if (!e_theme_edje_object_set
       (ss->slide_obj, "base/theme/modules/slideshow", "modules/slideshow/main"))
     edje_object_file_set(ss->slide_obj, buf, "modules/slideshow/main");
   evas_object_show(ss->slide_obj);

   edje_object_part_swallow(ss->slide_obj, "item", ss->img_obj);
   return ss;
}

static void
_slide_free(Slideshow *ss)
{
   evas_object_del(ss->img_obj);
   evas_object_del(ss->bg_obj);
   evas_object_del(ss->slide_obj);
   E_FREE(ss);
}

static Eina_Bool 
_slide_cb_check(void *data)
{
   Instance *inst;
   char *bg;

   inst = data;
   _slide_get_bg_count(inst);

   if (inst->index > inst->bg_count) inst->index = 0;
   if (inst->index <= inst->bg_count)
     {
        bg = eina_list_nth(inst->bg_list, inst->index);
	if (!bg)
	  {
	     inst->index = 0;
             bg = eina_list_nth(inst->bg_list, inst->index);
	  }
	if (bg)
	  {
	     _slide_set_bg(inst, bg);
	     if(inst->ci->random_order)
	       {
                  srand(time(NULL) + inst->gcc->id * 100);
		  inst->index = (rand() % (inst->bg_count+1));
	       }
	     else
	       inst->index++;
	     _slide_set_preview(inst);
	  }
     }
   return EINA_TRUE;
}

static void
_slide_get_bg_subdirs(void *data, char *local_path)
{
   Eina_List *dir_list;
   char full_path[PATH_MAX];
   char item_full_path[PATH_MAX];
   char item_local_path[PATH_MAX];
   char *item;
   Instance *inst;

   inst = data;
   if (!inst->ci->dir) return;

   snprintf(full_path, sizeof(full_path), "%s/%s", inst->ci->dir, local_path);
   dir_list = ecore_file_ls(full_path);

   EINA_LIST_FREE(dir_list, item)
     {
	snprintf(item_full_path, sizeof(item_full_path), "%s/%s", full_path, item);
	snprintf(item_local_path, sizeof(item_local_path), "%s/%s", local_path, item);

	if(ecore_file_is_dir(item_full_path))
	  _slide_get_bg_subdirs(inst, item_local_path);
	else
          inst->bg_list = eina_list_append(inst->bg_list, strdup(item_local_path));

	free(item);
     }
}

static void
_slide_get_bg_count(void *data)
{
   Instance *inst;
   char *item;
   Eina_List *dir_list;
   char item_full_path[PATH_MAX];

   inst = data;
   if (!inst->ci->dir) return;

   inst->bg_count = 0;
   EINA_LIST_FREE(inst->bg_list, item)
     free(item);

   dir_list = ecore_file_ls(inst->ci->dir);
   EINA_LIST_FREE(dir_list, item)
     {
	snprintf(item_full_path, sizeof(item_full_path), "%s/%s", inst->ci->dir, item);

	if(ecore_file_is_dir(item_full_path))
	  _slide_get_bg_subdirs(inst, item);
	else
          inst->bg_list = eina_list_append(inst->bg_list, strdup(item));
	free(item);
     }

   inst->bg_count = eina_list_count(inst->bg_list);
}

static void
_slide_set_bg(void *data, const char *bg)
{
   Instance *inst;
   E_Container *con;
   E_Gadcon *g;
   E_Desk *d;
   E_Zone *z;
   int i;
   char buf[PATH_MAX];

   inst = data;
   if (!(g = inst->gcc->gadcon)) return;
   snprintf (buf, sizeof (buf), "%s/%s", inst->ci->dir, bg);

   if (inst->ci->all_desks == 0) 
     {
	con = e_container_current_get(e_manager_current_get());
	z = e_zone_current_get(con);
	d = e_desk_current_get(z);
	e_bg_del(con->num, z->num, d->x, d->y);
	e_bg_add(con->num, z->num, d->x, d->y, buf);
     }
   else if (inst->ci->all_desks == 1) 
     {
	while (e_config->desktop_backgrounds) 
	  {
	     E_Config_Desktop_Background *cfbg;

	     cfbg = e_config->desktop_backgrounds->data;
	     e_bg_del(cfbg->container, cfbg->zone, cfbg->desk_x, cfbg->desk_y);
	  }
	e_bg_default_set(buf);
     }
   else if (inst->ci->all_desks == 2)
     {
	z = e_gadcon_zone_get(g);
        for (i = 0; i < z->desk_x_count * z->desk_y_count; i++)
          {
            e_bg_del(z->container->num, z->num, z->desks[i]->x, z->desks[i]->y);
            e_bg_add(z->container->num, z->num, z->desks[i]->x, z->desks[i]->y, buf);
	  }
     }   

   e_bg_update();
}

static void
_slide_set_preview(void *data)
{
   Instance *inst;
   Slideshow *ss;
   char buf[PATH_MAX];
   char *bg;

   inst = data;
   ss = inst->slide;

   bg = eina_list_nth(inst->bg_list, inst->index);
   snprintf(buf, sizeof (buf), "%s/%s", inst->ci->dir, bg);
   if (!e_util_edje_collection_exists (buf, "e/desktop/background")) return;
   if (ss->bg_obj) evas_object_del(ss->bg_obj);
   ss->bg_obj = edje_object_add(e_livethumb_evas_get (ss->img_obj));
   edje_object_file_set(ss->bg_obj, buf, "e/desktop/background");
   e_livethumb_thumb_set(ss->img_obj, ss->bg_obj);
}
