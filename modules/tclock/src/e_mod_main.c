#include <e.h>
#include "e_mod_main.h"
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
#if HAVE_LOCALE_H
#  include <locale.h>
#endif


/* Func Proto Requirements for Gadcon */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);

/* Module Protos */
static void _tclock_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _tclock_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _tclock_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _tclock_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi);
static Eina_Bool _tclock_cb_check(void *data);
static Config_Item *_tclock_config_item_get(const char *id);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;
static Ecore_Timer *check_timer;

Config *tclock_config = NULL;

/* Define the class and gadcon functions this module provides */
static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "tclock", 
     { _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, 
          NULL, NULL },
   E_GADCON_CLIENT_STYLE_PLAIN
};

typedef struct _Instance Instance;
struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object *tclock, *o_tip;
   Config_Item *ci;
   E_Gadcon_Popup *tip;
};

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;
   char buf[PATH_MAX];

   inst = E_NEW(Instance, 1);

   inst->ci = _tclock_config_item_get(id);
   if (!inst->ci->id) inst->ci->id = eina_stringshare_add(id);

   o = edje_object_add(gc->evas);
   snprintf(buf, sizeof(buf), "%s/tclock.edj", tclock_config->mod_dir);
   if (!e_theme_edje_object_set(o, "base/theme/modules/tclock", 
                                "modules/tclock/main"))
     edje_object_file_set(o, buf, "modules/tclock/main");
   evas_object_show(o);

   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->tclock = o;

   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
                                  _tclock_cb_mouse_down, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,
                                  _tclock_cb_mouse_in, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT,
                                  _tclock_cb_mouse_out, inst);

   tclock_config->instances =
     eina_list_append(tclock_config->instances, inst);

   _tclock_cb_check(inst);
   if (!check_timer)
     check_timer = ecore_timer_add(1.0, _tclock_cb_check, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   if (!(inst = gcc->data)) return;

   evas_object_event_callback_del(inst->tclock, EVAS_CALLBACK_MOUSE_DOWN,
                                  _tclock_cb_mouse_down);
   evas_object_event_callback_del(inst->tclock, EVAS_CALLBACK_MOUSE_IN,
                                  _tclock_cb_mouse_in);
   evas_object_event_callback_del(inst->tclock, EVAS_CALLBACK_MOUSE_OUT,
                                  _tclock_cb_mouse_out);

   if (inst->tclock) evas_object_del(inst->tclock);

   if ((!tclock_config->instances) && (check_timer))
     {
        ecore_timer_del(check_timer);
        check_timer = NULL;
     }

   tclock_config->instances =
     eina_list_remove(tclock_config->instances, inst);

   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient __UNUSED__)
{
   Instance *inst;
   Evas_Coord mw;

   if (!(inst = gcc->data)) return;
   edje_object_size_min_calc(inst->tclock, &mw, NULL);
   e_gadcon_client_min_size_set(gcc, mw, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   return D_("TClock");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class __UNUSED__, Evas *evas)
{
   Evas_Object *o;
   char buf[PATH_MAX];

   o = edje_object_add (evas);
   snprintf(buf, sizeof(buf), "%s/e-module-tclock.edj", tclock_config->mod_dir);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   Config_Item *ci;

   ci = _tclock_config_item_get(NULL);
   return ci->id;
}

static void
_tclock_cb_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;
   if (ev->button == 3)
     {
        E_Menu *m;
        E_Menu_Item *mi;
        int x = 0, y = 0;

        m = e_menu_new();
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Settings"));
        e_util_menu_item_theme_icon_set(mi, "configure");
        e_menu_item_callback_set(mi, _tclock_menu_cb_configure, inst);

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
        e_menu_activate_mouse (m,
                               e_util_zone_current_get(e_manager_current_get()), 
                               x + ev->output.x, y + ev->output.y, 1, 1,
                               E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
     }
}

static void 
_tclock_cb_mouse_in(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, 
                    void *event_info __UNUSED__)
{
   Instance *inst = NULL;
   char buf[1024];
   time_t current_time;
   struct tm *local_time;

   if (!(inst = data)) return;
   if (!inst->ci->show_tip) return;
   if (inst->tip) return;

   inst->tip = e_gadcon_popup_new(inst->gcc);

   current_time = time(NULL);

   local_time = localtime(&current_time);
   memset(buf, 0, sizeof(buf));
   strftime(buf, 1024, inst->ci->tip_format, local_time);
   inst->o_tip = e_widget_label_add(inst->tip->win->evas, buf);

   e_gadcon_popup_content_set(inst->tip, inst->o_tip);
   e_gadcon_popup_show(inst->tip);
}

static void 
_tclock_cb_mouse_out(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, 
                     void *event_info __UNUSED__)
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   if (!inst->tip) return;
   evas_object_del(inst->o_tip);
   e_object_del(E_OBJECT(inst->tip));
   inst->tip = NULL;
   inst->o_tip = NULL;
}

static void
_tclock_menu_cb_configure(void *data, E_Menu *m __UNUSED__, E_Menu_Item *mi __UNUSED__)
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   _config_tclock_module(inst->ci);
}

static void
_eval_instance_size(Instance *inst)
{
   if (!inst) return;
   Evas_Coord mw, mh, omw, omh;

   edje_object_size_min_get(inst->tclock, &mw, &mh);

   omw = mw;
   omh = mh;

   if ((mw < 1) || (mh < 1))
      {
         Evas_Coord x, y, sw = 0, sh = 0, ow, oh;
         Eina_Bool horiz;

         switch (inst->gcc->gadcon->orient)
            {
             case E_GADCON_ORIENT_TOP:
             case E_GADCON_ORIENT_CORNER_TL:
             case E_GADCON_ORIENT_CORNER_TR:
             case E_GADCON_ORIENT_BOTTOM:
             case E_GADCON_ORIENT_CORNER_BL:
             case E_GADCON_ORIENT_CORNER_BR:
             case E_GADCON_ORIENT_HORIZ:
                horiz = EINA_TRUE;
                break;

             case E_GADCON_ORIENT_LEFT:
             case E_GADCON_ORIENT_CORNER_LB:
             case E_GADCON_ORIENT_CORNER_LT:
             case E_GADCON_ORIENT_RIGHT:
             case E_GADCON_ORIENT_CORNER_RB:
             case E_GADCON_ORIENT_CORNER_RT:
             case E_GADCON_ORIENT_VERT:
                horiz = EINA_FALSE;
                break;

             default:
                horiz = EINA_TRUE;
            }

         if (inst->gcc->gadcon->shelf)
            {
               if (horiz)
                  sh = inst->gcc->gadcon->shelf->h;
               else
                  sw = inst->gcc->gadcon->shelf->w;
            }

         evas_object_geometry_get(inst->tclock, NULL, NULL, &ow, &oh);
         evas_object_resize(inst->tclock, sw, sh);
         edje_object_parts_extends_calc(inst->tclock, &x, &y, &mw, &mh);
         evas_object_resize(inst->tclock, ow, oh);
      }

   if (mw < 10) mw = 10;
   if (mh < 10) mh = 10;

   if (mw < omw) mw = omw;
   if (mh < omh) mh = omh;

   e_gadcon_client_min_size_set(inst->gcc, mw + 10, mh);
}

void
_tclock_config_updated(Config_Item *ci __UNUSED__)
{
   if (!tclock_config) return;
   _tclock_cb_check(NULL);
}

static Eina_Bool
_tclock_cb_check(void *data)
{
   Instance *inst = data;
   Eina_List *l;
   time_t current_time;
   struct tm *local_time;
   char buf[127];
   char buff[128];
   int offset_int;

   EINA_LIST_FOREACH(tclock_config->instances, l, inst)
     {
        if (!inst->ci->show_time)
          edje_object_signal_emit(inst->tclock, "time_hidden", "");
        else
          {
             if (!inst->ci->show_date)
               edje_object_signal_emit(inst->tclock, "time_centered", "");
             else
               edje_object_signal_emit(inst->tclock, "time_visible", "");
          }

        if (!inst->ci->show_date)
          edje_object_signal_emit(inst->tclock, "date_hidden", "");
        else
          {
             if (!inst->ci->show_time)
               edje_object_signal_emit(inst->tclock, "date_centered", "");
             else
               edje_object_signal_emit(inst->tclock, "date_visible", "");
          }

        edje_object_message_signal_process(inst->tclock);

        memset (buf, 0, sizeof(buf));

        offset_int = atoi(inst->ci->time_offset);

        current_time = time(NULL) + offset_int * 3600;
        local_time = localtime(&current_time);

        if (inst->ci->time_format)
          {
             strftime(buf, 127, inst->ci->time_format, local_time);
             sprintf(buff, "%s ", buf);
             edje_object_part_text_set(inst->tclock, "tclock_time", buff);
          }
        if (inst->ci->date_format)
          {
             strftime(buf, 127, inst->ci->date_format, local_time);
             sprintf(buff, "%s ", buf);
             edje_object_part_text_set(inst->tclock, "tclock_date", buff);
          }

        edje_object_text_class_set (inst->tclock, "module_large", "Sans:style=Mono",
                                    inst->ci->font_size_up);
        edje_object_text_class_set (inst->tclock, "module_small", "Sans:style=Mono",
                                    inst->ci->font_size_down);
        edje_object_color_class_set (inst->tclock, "module_label", inst->ci->color_r,
                                     inst->ci->color_g, inst->ci->color_b, 
                                     inst->ci->color_alpha, 0, 0, 0, 255, 0, 0, 0, 255);
     }
   _eval_instance_size(inst);
   return EINA_TRUE;
}

static Config_Item *
_tclock_config_item_get(const char *id)
{
   Config_Item *ci;

   GADCON_CLIENT_CONFIG_GET(Config_Item, tclock_config->items, _gc_class, id);

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->show_date = 1;
   ci->show_time = 1;
   ci->show_tip = 1;
   ci->time_format = eina_stringshare_add("%T");
   ci->time_offset = eina_stringshare_add("0");

   ci->date_format = eina_stringshare_add("%d/%m/%y");
   ci->tip_format = eina_stringshare_add("%A, %B %d, %Y");
   ci->font_size_up = 12;
   ci->font_size_down = 10;
   ci->color_r = 255;
   ci->color_g = 255;
   ci->color_b = 255;
   ci->color_alpha = 255;
   tclock_config->items = eina_list_append(tclock_config->items, ci);
   return ci;
}

EAPI E_Module_Api e_modapi = { E_MODULE_API_VERSION, "TClock" };

EAPI void *
e_modapi_init(E_Module *m)
{
#if HAVE_LOCALE_H
   setlocale(LC_ALL, "");
#endif
   /* Set up module locales*/
   bindtextdomain(LOCALEDOMAIN, LOCALEDIR);
   bind_textdomain_codeset(LOCALEDOMAIN, "UTF-8");

   conf_item_edd = E_CONFIG_DD_NEW("TClock_Config_Item", Config_Item);
#undef T
#undef D
#define T Config_Item
#define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, show_date, INT);
   E_CONFIG_VAL(D, T, show_time, INT);
   E_CONFIG_VAL(D, T, show_tip, INT);
   E_CONFIG_VAL(D, T, date_format, STR);
   E_CONFIG_VAL(D, T, time_format, STR);
   E_CONFIG_VAL(D, T, time_offset, STR);
   E_CONFIG_VAL(D, T, tip_format, STR);
   E_CONFIG_VAL(D, T, font_size_up, DOUBLE);
   E_CONFIG_VAL(D, T, font_size_down, DOUBLE);
   E_CONFIG_VAL(D, T, color_r, DOUBLE);
   E_CONFIG_VAL(D, T, color_g, DOUBLE);
   E_CONFIG_VAL(D, T, color_b, DOUBLE);
   E_CONFIG_VAL(D, T, color_alpha, DOUBLE);

   conf_edd = E_CONFIG_DD_NEW("TClock_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);

   tclock_config = e_config_domain_load("module.tclock", conf_edd);
   if (!tclock_config)
     {
        Config_Item *ci;

        tclock_config = E_NEW(Config, 1);

        ci = E_NEW(Config_Item, 1);
        ci->id = eina_stringshare_add("0");
        ci->show_date = 1;
        ci->show_time = 1;
        ci->show_tip = 1;
        ci->time_format = eina_stringshare_add("%T");
        ci->time_offset = eina_stringshare_add("0");
        ci->date_format = eina_stringshare_add("%d/%m/%y");
        ci->tip_format = eina_stringshare_add("%A, %B %d, %Y");
        ci->font_size_up = 12;
        ci->font_size_down = 10;
        ci->color_r = 255;
        ci->color_g = 255;
        ci->color_b = 255;
        ci->color_alpha = 255;
        tclock_config->items = eina_list_append(tclock_config->items, ci);
     }
   tclock_config->mod_dir = eina_stringshare_add(m->dir);

   e_gadcon_provider_register(&_gc_class);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   e_gadcon_provider_unregister(&_gc_class);

   if (tclock_config->config_dialog)
     e_object_del(E_OBJECT(tclock_config->config_dialog));

   while (tclock_config->items)
     {
        Config_Item *ci;

        ci = tclock_config->items->data;
        tclock_config->items =
          eina_list_remove_list(tclock_config->items, tclock_config->items);
        if (ci->id)
          eina_stringshare_del(ci->id);
        if (ci->time_format)
          eina_stringshare_del(ci->time_format);
        if (ci->date_format)
          eina_stringshare_del(ci->date_format);
        if (ci->tip_format)
          eina_stringshare_del(ci->tip_format);
        E_FREE (ci);
     }

   if (check_timer)
     {
        ecore_timer_del(check_timer);
        check_timer = NULL;
     }
   if (tclock_config->mod_dir) eina_stringshare_del(tclock_config->mod_dir);
   E_FREE(tclock_config);
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m __UNUSED__)
{
   e_config_domain_save("module.tclock", conf_edd, tclock_config);
   return 1;
}
