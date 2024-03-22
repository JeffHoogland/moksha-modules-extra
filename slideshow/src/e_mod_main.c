#include <e.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include "e_mod_main.h"

typedef struct _Instance Instance;
typedef struct _Slideshow Slideshow;

struct _Instance
{
   E_Gadcon_Client     *gcc;
   Evas_Object         *slide_obj;
   Slideshow           *slide;
   Ecore_Timer         *check_timer;
   Ecore_Timer         *check_timer_hr;
   Eina_List           *bg_list;
   const char          *display;
   char                *in_file;
   char                *tmpf;
   int                  index, bg_id, bg_count;
   Ecore_Exe           *exe;
   Ecore_Event_Handler *exe_handler;
   Ecore_End_Cb         ok;
   Config_Item         *ci;
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
static void _slide_conf_new(char *dir);
static void _slide_conf_free(void);
static Eina_Bool _slide_cb_check(void *data);
static Eina_Bool _slide_cb_check_time(void *data);
static void _slide_get_bg_count(void *data);
static void _slide_set_bg(void *data, const char *bg);
static void _slide_set_preview(void *data);
static void _slide_get_bg_subdirs(void *data, char *local_path);
static Eina_Bool  _edj_cc_exit(void *data, int type, void *event);
static void _import_edj_gen(Instance *inst);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;
static Eina_List *slideshow_handlers = NULL;

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
   _slide_get_bg_count(inst);
   if (!inst->ci->disable_sched)
     {
       _slide_cb_check_time(inst);
       inst->check_timer_hr = ecore_timer_add(60, _slide_cb_check_time, inst);
     }
   if (!inst->ci->disable_timer)
     inst->check_timer = ecore_timer_add(inst->ci->poll_time, _slide_cb_check, inst);
   else
     {
        if (inst->ci->random_order)
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
   if (inst->check_timer_hr) ecore_timer_del(inst->check_timer_hr);

   slide_config->instances = eina_list_remove(slide_config->instances, inst);

   evas_object_event_callback_del(slide->slide_obj, EVAS_CALLBACK_MOUSE_DOWN, 
                                  _slide_cb_mouse_down);
   _slide_free(slide);
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
   return D_("Slideshow");
}

static Evas_Object *
_gc_icon( const E_Gadcon_Client_Class *client_class __UNUSED__, Evas *evas)
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
_gc_id_new(__UNUSED__ const E_Gadcon_Client_Class *client_class)
{
   Config_Item *ci;

   ci = _slide_config_item_get(NULL);
   return ci->id;
}

static void
_slide_cb_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
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
_slide_menu_cb_post(void *data __UNUSED__, E_Menu *m __UNUSED__)
{
   if (!slide_config->menu) return;
   e_object_del(E_OBJECT(slide_config->menu));
   slide_config->menu = NULL;
}

static void
_slide_menu_cb_configure(void *data,E_Menu *m __UNUSED__, E_Menu_Item *mi __UNUSED__)
{
   Instance *inst;

   inst = data;

   if (!slide_config) return;
   if (slide_config->config_dialog) return;
   _config_slideshow_module(inst->ci);
}

void
_slide_config_updated(Config_Item *ci)
{
   Eina_List *l;
   Instance *inst;
   char *bg;

   if (!slide_config) return;

   EINA_LIST_FOREACH(slide_config->instances, l, inst) 
     {
        int no_match = 0;
        
        if (ci && inst->ci != ci) continue;
        if (inst->check_timer) ecore_timer_del(inst->check_timer);
        if (inst->check_timer_hr) ecore_timer_del(inst->check_timer_hr);
        
        if (!inst->ci->disable_sched)
          {
            EINA_LIST_FOREACH(inst->bg_list, l, bg)
              {
                if (!strcmp(bg, inst->ci->file_day) || 
                    !strcmp(bg, inst->ci->file_night))
                  no_match++;
              }
            if (no_match != 2)
              {
                 e_util_dialog_show(D_("Warning"), 
                                    D_("Check Day/Night bg file names. "
                                      "They do not match!"));
                 return;
              }
          }

        if (!inst->ci->disable_timer)
           inst->check_timer = ecore_timer_add(inst->ci->poll_time,
                                              _slide_cb_check, inst);
        if (!inst->ci->disable_sched)
          {
            inst->check_timer_hr = ecore_timer_add(60, 
                                         _slide_cb_check_time, inst);
            _slide_cb_check_time(inst);
          }
     }
}

static Eina_Bool
_slideshow_update(void *d __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   _slide_config_updated(NULL);
   return ECORE_CALLBACK_PASS_ON;
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
   ci->hours = 6.0;
   ci->minutes = 0.0;
   ci->disable_timer = 0;
   ci->disable_sched = 1;
   ci->all_desks = 0;
   snprintf(buf, sizeof (buf), "%s/.e/e/backgrounds", e_user_homedir_get());
   ci->dir = eina_stringshare_add(buf);
   ci->file_day = eina_stringshare_add("");
   ci->file_night = eina_stringshare_add("");

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

   snprintf(buf, sizeof (buf), "%s/.e/e/backgrounds", e_user_homedir_get());
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   conf_item_edd = E_CONFIG_DD_NEW("Slideshow_Config_Item", Config_Item);
#undef T
#undef D
#define T Config_Item
#define D conf_item_edd

   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, dir, STR);
   E_CONFIG_VAL(D, T, file_day, STR);
   E_CONFIG_VAL(D, T, file_night, STR);
   E_CONFIG_VAL(D, T, poll_time, DOUBLE);
   E_CONFIG_VAL(D, T, hours, DOUBLE);
   E_CONFIG_VAL(D, T, minutes, DOUBLE);
   E_CONFIG_VAL(D, T, disable_timer, INT);
   E_CONFIG_VAL(D, T, disable_sched, INT);
   E_CONFIG_VAL(D, T, random_order, INT);
   E_CONFIG_VAL(D, T, all_desks, INT);
   conf_edd = E_CONFIG_DD_NEW("Slideshow_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_LIST(D, T, items, conf_item_edd);

   slide_config = e_config_domain_load("module.slideshow", conf_edd);
   
   if (slide_config) {
     /* Check config version */
     if (!e_util_module_config_check("Slideshow", slide_config->version, MOD_CONFIG_FILE_VERSION))
       _slide_conf_free();
   }

   /* If we don't have a config yet, or it got erased above,
   * then create a default one */
   if (!slide_config)
      _slide_conf_new(buf);

   E_LIST_HANDLER_APPEND(slideshow_handlers, E_EVENT_SYS_RESUME, _slideshow_update, NULL);
   slide_config->module = m;
   e_gadcon_provider_register(&_gc_class);
   return m;
}

EAPI int
e_modapi_shutdown(__UNUSED__ E_Module *m)
{
   slide_config->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);

   E_FREE_LIST(slideshow_handlers, ecore_event_handler_del);

   /* Kill the config dialog */
   if (slide_config->config_dialog)
     e_object_del(E_OBJECT(slide_config->config_dialog));

   if (slide_config->menu)
     {
        e_menu_post_deactivate_callback_set(slide_config->menu, NULL, NULL);
        e_object_del(E_OBJECT(slide_config->menu));
        slide_config->menu = NULL;
     }
  
   _slide_conf_free();
   
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(__UNUSED__ E_Module *m)
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

static void
_slide_conf_new(char *dir)
{
   Config_Item *ci;

   slide_config = E_NEW(Config, 1);
   /* update the version */
   slide_config->version = MOD_CONFIG_FILE_VERSION;
   
   ci = E_NEW(Config_Item, 1);

   ci->id = eina_stringshare_add("0");
   ci->dir = eina_stringshare_add(dir);
   ci->file_day = eina_stringshare_add("");
   ci->file_night = eina_stringshare_add("");
   ci->poll_time = 60.0;
   ci->hours = 6.0;
   ci->minutes = 0.0;
   ci->disable_timer = 0;
   ci->disable_sched = 1;
   ci->random_order = 0;
   ci->all_desks = 0;
   slide_config->items = eina_list_append(slide_config->items, ci);
   
   /* save the config to disk */
   e_config_save_queue();
}

static void
_slide_conf_free(void)
{
   Config_Item *ci;

   EINA_LIST_FREE(slide_config->items, ci)
     {
        eina_stringshare_del(ci->id);
        eina_stringshare_del(ci->dir);
        eina_stringshare_del(ci->file_day);
        eina_stringshare_del(ci->file_night);
        free(ci);
      }

   slide_config->module = NULL;
   E_FREE(slide_config);
}

static Eina_Bool
_slide_cb_check_time(void *data)
{
   Instance *inst; 
   double now, set_time;
   time_t rawtime;
   struct tm * timeinfo;

   inst = data;

   if (inst->ci->file_day[0] == '\0' || inst->ci->file_night[0] == '\0')
     {
       e_util_dialog_show(D_("Warning"), 
                          D_("Day/Night file names are not defined!"));
       return EINA_FALSE;
     }

   time(&rawtime);
   timeinfo = localtime( &rawtime );

   set_time = inst->ci->hours * 3600 + inst->ci->minutes * 60;
   now = timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60;
   
   if (now >= set_time && now < set_time + 12 * 3600)
      _slide_set_bg(inst, inst->ci->file_day);
   else 
      _slide_set_bg(inst, inst->ci->file_night);

   return EINA_TRUE;
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
             if (inst->ci->random_order)
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
   int ret;
   Instance *inst;

   inst = data;
   if (!inst->ci->dir) return;

   snprintf(full_path, sizeof(full_path), "%s/%s", inst->ci->dir, local_path);
   dir_list = ecore_file_ls(full_path);

   EINA_LIST_FREE(dir_list, item)
     {
       ret = snprintf(item_full_path, sizeof(item_full_path), "%s/%s", full_path, item);
       if (ret < 0) abort();
       ret = snprintf(item_local_path, sizeof(item_local_path), "%s/%s", local_path, item);
       if (ret < 0) abort();
       if (ecore_file_is_dir(item_full_path))
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

        if (ecore_file_is_dir(item_full_path))
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
   if (!eina_str_has_extension(bg, ".edj"))
     {
         inst->in_file = strdup(buf);
         _import_edj_gen(inst);
         snprintf (buf, sizeof (buf), "%s.edj", ecore_file_strip_ext(buf));
     }

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

static void
_import_free(Instance *inst)
{
   EINA_SAFETY_ON_NULL_RETURN(inst);
   ecore_event_handler_del(inst->exe_handler);
   inst->exe_handler = NULL;
   inst->exe = NULL;
   if (inst->tmpf) unlink(inst->tmpf);
   unlink(inst->tmpf);
   free(inst->tmpf);
   ecore_file_remove(inst->in_file);
   free(inst->in_file);
   _slide_cb_check(inst);
}

static Eina_Bool
_edj_cc_exit(void *data, int type __UNUSED__, void *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(data,  ECORE_CALLBACK_DONE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, ECORE_CALLBACK_DONE);

   Instance *inst;
   Ecore_Exe_Event_Del *ev;
   int r = 1;

   ev = event;
   inst = data;
   if (ecore_exe_data_get(ev->exe) != inst) return ECORE_CALLBACK_PASS_ON;

   if (ev->exit_code != 0)
     {
        e_util_dialog_show(D_("Picture Import Error"),
                           D_("Moksha was unable to import the picture<br>"
                             "due to conversion errors."));
        r = 0;
     }

    if (r && inst->ok) inst->ok(inst, NULL);
      _import_free(inst);

   return ECORE_CALLBACK_DONE;
}

static void
_import_edj_gen(Instance *inst)
{
   Eina_Bool anim = EINA_FALSE;
   int fd, num = 1;
   int w = 0, h = 0;
   const char *in_file, *file;
   char buf[PATH_MAX], cmd[PATH_MAX + PATH_MAX + 40], tmpn[PATH_MAX], ipart[PATH_MAX], enc[128];
   char *imgdir = NULL, *fstrip;
   FILE *f;
   size_t len, off;

   in_file = inst->in_file;
   file = ecore_file_file_get(in_file);
   fstrip = ecore_file_strip_ext(file);
   if (!fstrip) return;
   if (inst->ci->dir)
     len = snprintf(buf, sizeof(buf), "%s/%s.edj", inst->ci->dir, fstrip);
   else
     len = e_user_dir_snprintf(buf, sizeof(buf), "backgrounds/%s.edj", fstrip);

   if (len >= sizeof(buf))
     {
        free(fstrip);
        return;
     }
   off = len - (sizeof(".edj") - 1);
   for (num = 1; ecore_file_exists(buf) && num < 100; num++)
     snprintf(buf + off, sizeof(buf) - off, "-%d.edj", num);
   free(fstrip);

   if (num == 100)
     {
        printf("Couldn't come up with another filename for %s\n", buf);
        return;
     }

   strcpy(tmpn, "/tmp/e_bgdlg_new.edc-tmp-XXXXXX");
   fd = mkstemp(tmpn);
   if (fd < 0)
     {
        printf("Error Creating tmp file: %s\n", strerror(errno));
        _import_free(inst);
        return;
     }

   f = fdopen(fd, "w");
   if (!f)
     {
        printf("Cannot open %s for writing\n", tmpn);
        return;
     }

   anim = eina_str_has_extension(in_file, "gif");
   imgdir = ecore_file_dir_get(in_file);
   if (!imgdir) ipart[0] = '\0';
   else
     {
        snprintf(ipart, sizeof(ipart), "-id %s", e_util_filename_escape(imgdir));
        free(imgdir);
     }

   fstrip = strdupa(e_util_filename_escape(file));
   snprintf(enc, sizeof(enc), "LOSSY %i", 90);
   fprintf(f,
             "images { image: \"%s\" %s; }\n"
             "collections {\n"
             "group { name: \"e/desktop/background\";\n"
             "%s"
             "data { item: \"style\" \"0\"; }\n"
             "max: %i %i;\n"
             "parts {\n"
             "part { name: \"bg\"; mouse_events: 0;\n"
             "description { state: \"default\" 0.0;\n"
             "image { normal: \"%s\"; scale_hint: STATIC; }\n"
             "} } } } }\n"
             , fstrip, enc, anim ? "" : "data.item: \"noanimation\" \"1\";\n", w, h, fstrip);

   fclose(f);

   snprintf(cmd, sizeof(cmd), "edje_cc -v %s %s %s",
            ipart, tmpn, e_util_filename_escape(buf));

   inst->tmpf = strdup(tmpn);
   if (inst->exe_handler) ecore_event_handler_del(inst->exe_handler);
   inst->exe_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                             _edj_cc_exit, inst);
   inst->exe = ecore_exe_run(cmd, inst);
}
