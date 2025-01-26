#include <e.h>
#include "e_mod_main.h"

#define GOLDEN_RATIO 1.618033989

typedef struct _Instance Instance;
typedef struct _Calendar Calendar;

struct _Instance
{
   E_Gadcon_Client *gcc;
   Calendar        *calendar;
   Config_Item     *ci;

   E_Gadcon_Popup  *popup;
   Evas_Object     *list;
   Evas_Object     *label;
   Evas_Object     *table;
   Eina_List       *notes;

   char            *edje_module;

   struct tm        displayed_time;
   struct tm        current_time;
};

struct _Calendar
{
   Instance *inst;
   Evas_Object *o_icon, *o_today;
};

static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static Config_Item *_config_item_get(const char *id);
static Eina_Bool _update_date(void *data);
static void _update_calendar_sheet(Instance *inst);
static void _calendar_popup_content_create(Instance *inst);
static void _calendar_popup_content_populate(Instance *inst, struct tm *time);
static void _calendar_popup_content_update(Instance *inst);
static void _calendar_popup_destroy(Instance *inst);
static void _cb_action(E_Object *obj, const char *params);
static void _cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _menu_cb_post(void *data, E_Menu *m);
static void _calendar_firstweekday_su(void *data, E_Menu *m, E_Menu_Item *mi);
static void _calendar_firstweekday_mo(void *data, E_Menu *m, E_Menu_Item *mi);

static int days_in_month[2][12] =
{
   {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
   {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;
static E_Action *act = NULL;

static Config *calendar_conf = NULL;

static const E_Gadcon_Client_Class _gc_class =
{
  GADCON_CLIENT_CLASS_VERSION, "calendar",
  {
    _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new,
    NULL, e_gadcon_site_is_not_toolbar
  },  E_GADCON_CLIENT_STYLE_PLAIN
};

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Calendar        *calendar;
   Instance        *inst;
   E_Gadcon_Client *gcc;
   int              length;

   inst = E_NEW(Instance, 1);
   inst->ci = _config_item_get(id);

   calendar = E_NEW(Calendar, 1);
   calendar->inst = inst;

   /* Find module specific edje file */
   length = strlen(e_module_dir_get(calendar_conf->module));
   inst->edje_module = malloc(length + 14);
   snprintf(inst->edje_module, length + 14, "%s/calendar.edj",
            e_module_dir_get(calendar_conf->module));

   calendar->o_icon = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(calendar->o_icon,
                               "base/theme/modules/calendar", "modules/calendar/main"))
     edje_object_file_set(calendar->o_icon, inst->edje_module, "modules/calendar/main");
   evas_object_show(calendar->o_icon);

   gcc = e_gadcon_client_new(gc, name, id, style, calendar->o_icon);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->calendar = calendar;

   calendar_conf->instances = eina_list_append(calendar_conf->instances, inst);
   evas_object_event_callback_add(calendar->o_icon, EVAS_CALLBACK_MOUSE_IN,
                                  _cb_mouse_in, inst);
   evas_object_event_callback_add(calendar->o_icon, EVAS_CALLBACK_MOUSE_OUT,
                                  _cb_mouse_out, inst);
   evas_object_event_callback_add(calendar->o_icon, EVAS_CALLBACK_MOUSE_DOWN,
                                  _cb_mouse_down, inst);

   _update_calendar_sheet(inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst;
   Calendar *calendar;
   
   inst = gcc->data;
   calendar = inst->calendar;

   if (inst->popup) _calendar_popup_destroy(inst);
   if (calendar->o_icon) evas_object_del(calendar->o_icon);
   
   calendar_conf->instances = eina_list_remove(calendar_conf->instances, inst);
   free(inst->edje_module);
   E_FREE(calendar);
   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient __UNUSED__)
{
   Instance *inst;
   Evas_Coord mw, mh;

   inst = gcc->data;
   mw = 0, mh = 0;
   edje_object_size_min_get(inst->calendar->o_icon, &mw, &mh);
   if ((mw < 1) || (mh < 1))
     edje_object_size_min_calc(inst->calendar->o_icon, &mw, &mh);
   if (mw < 4) mw = 4;
   if (mh < 4) mh = 4;
   e_gadcon_client_aspect_set(gcc, mw, mh);
   e_gadcon_client_min_size_set(gcc, mw, mh);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   return "Calendar";
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class __UNUSED__, Evas *evas)
{
   Evas_Object *o;
   char buf[4096];

   if (!calendar_conf->module) return NULL;
   snprintf(buf, sizeof(buf), "%s/e-module-calendar.edj",
            e_module_dir_get(calendar_conf->module));

   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   Config_Item *ci;
   const char *id;
   char buf[128];
   int num = 0;

   /* Create id */
   if (calendar_conf->items)
     {
        const char *p;
        ci = eina_list_data_get(eina_list_last(calendar_conf->items));
        p = strrchr(ci->id, '.');
        if (p) num = atoi(p + 1) + 1;
     }
   snprintf(buf, sizeof(buf), "%s.%d", _gc_class.name, num);
   id = eina_stringshare_add(buf);
   return id;
}

static Config_Item *
_config_item_get(const char *id) 
{
   Eina_List   *l;
   Config_Item *ci;
   char buf[128];

   if (!id)
     {
        int  num = 0;

        /* Create id */
        if (calendar_conf->items)
          {
             const char *p;
             ci = eina_list_last(calendar_conf->items)->data;
             p = strrchr(ci->id, '.');
             if (p) num = atoi(p + 1) + 1;
          }
        snprintf(buf, sizeof(buf), "%s.%d", _gc_class.name, num);
        id = buf;
     }
   else
     {
        for (l = calendar_conf->items; l; l = l->next) 
          {
             ci = l->data;
             if (!ci->id) continue;
             if (!strcmp(ci->id, id))
               return ci;
          }
     }

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->firstweekday = 0;

   calendar_conf->items = eina_list_append(calendar_conf->items, ci);
   return ci;
}

static Eina_Bool
_update_date(void *data __UNUSED__)
{
   Instance *inst;
   Eina_List *l;
   time_t current_time;
   struct tm *local_time;
   static int prev_day = 0;

   if (!calendar_conf->instances) return EINA_TRUE;

   current_time = time (NULL);
   local_time = localtime (&current_time);
   if (prev_day == local_time->tm_mday)
     return EINA_TRUE;
   else
     prev_day = local_time->tm_mday;

   EINA_LIST_FOREACH(calendar_conf->instances, l, inst)
     {
        if (!inst) continue;
        _update_calendar_sheet(inst);
     }

   return EINA_TRUE;
}

static void
_update_calendar_sheet(Instance *inst)
{
   Calendar *calendar;
   char buf[5];
   time_t current_time;
   struct tm *local_time;

   if (!inst) return;
   calendar = inst->calendar;
   if (!calendar) return;

   current_time = time (NULL);
   local_time = localtime_r(&current_time, &inst->current_time);

   strftime (buf, sizeof(buf), "%d", local_time);
   edje_object_part_text_set (calendar->o_icon, "monthday", buf);
   strftime (buf, sizeof(buf), "%a", local_time);
   edje_object_part_text_set (calendar->o_icon, "weekday", buf);

   if (inst->popup)
     _calendar_popup_content_update(inst);
   else
     _calendar_popup_content_create(inst);
}

static const char *days[] = {
  "Su",
  "Mo",
  "Tu",
  "We",
  "Th",
  "Fr",
  "Sa"
};

static void
_calendar_popup_content_update(Instance *inst)
{
   evas_object_del(inst->table);
   inst->table = NULL;

   _calendar_popup_content_populate(inst, &inst->displayed_time);
}

static void
_day_today(void *data, __UNUSED__ Evas_Object *obj, __UNUSED__ const char *emission, __UNUSED__ const char *source)
{
   Instance *inst = data;
   
   //~ time_t current_time;
   //~ struct tm *local_time;
   //~ current_time = time (NULL);
   //~ local_time = localtime_r(&current_time, &inst->displayed_time);
   inst->displayed_time.tm_mday = 1;
   _calendar_popup_content_update(inst);
}

static void
_year_minus(void *data, __UNUSED__ Evas_Object *obj, __UNUSED__ const char *emission, __UNUSED__ const char *source)
{
   Instance *inst = data;

   inst->displayed_time.tm_year--;
   inst->displayed_time.tm_mday = 1;
   _calendar_popup_content_update(inst);
}

static void
_year_plus(void *data, __UNUSED__ Evas_Object *obj, __UNUSED__ const char *emission, __UNUSED__ const char *source)
{
   Instance *inst = data;

   inst->displayed_time.tm_year++;
   inst->displayed_time.tm_mday = 1;
   _calendar_popup_content_update(inst);
}

static void
_month_minus(void *data, __UNUSED__ Evas_Object *obj, __UNUSED__ const char *emission, __UNUSED__ const char *source)
{
   Instance *inst = data;

   inst->displayed_time.tm_mon--;
   inst->displayed_time.tm_mday = 1;
   if (inst->displayed_time.tm_mon < 0)
     {
        inst->displayed_time.tm_mon = 11;
        inst->displayed_time.tm_year--;
     }
   _calendar_popup_content_update(inst);
}

static void
_month_plus(void *data, __UNUSED__ Evas_Object *obj, __UNUSED__ const char *emission, __UNUSED__ const char *source)
{
   Instance *inst = data;

   inst->displayed_time.tm_mon++;
   inst->displayed_time.tm_mday = 1;
   if (inst->displayed_time.tm_mon > 11)
     {
        inst->displayed_time.tm_mon = 0;
        inst->displayed_time.tm_year++;
     }
   _calendar_popup_content_update(inst);
}

static void
_calendar_popup_content_populate(Instance *inst, struct tm *time)
{
   Evas *evas;
   Evas_Object *table;
   Evas_Object *o;
   struct tm *month_start;
   time_t current_time, start_time;
   int today, month, year, maxdays;
   int startwd, day;
   int col, row;
   Evas_Coord mw, mh;
   char buf[32];

   today = time->tm_mday;
   month = time->tm_mon;
   year = time->tm_year + 1900;

   maxdays = days_in_month[(!(year % 4) && (!(year % 400) || (year % 100)))]
      [month];

   current_time = mktime(time);
   start_time = current_time - ((today-1) * 86400);
   month_start = localtime (&start_time);
   strftime (buf, sizeof (buf), "%w", month_start);
   startwd = atoi (buf) - inst->ci->firstweekday;
   if (startwd < 0) startwd = 6;

   /* Set current month */
   strftime(buf, sizeof (buf), "%B %Y", time);
   e_widget_label_text_set(inst->label, buf);

   evas = inst->popup->win->evas;

   table = e_widget_table_add(evas, 0);

   /* Column titles */
   day = inst->ci->firstweekday;
   for (col = 0; col < (int) (sizeof (days) / sizeof (char *)); ++col)
     {
        o = e_widget_label_add(evas, days[(col + inst->ci->firstweekday) % (sizeof (days) / sizeof (char *))]);
        e_widget_table_object_append(table, o, col, 0, 1, 1, 0, 0, 0, 0);
     }

   /* output days */
   day = 0;
   for (row = 1; row <= 6; row++)
     {
        for (col = 0; col <= 6; col++)
          {
             int cday = 0;

             if (!day) if (col == startwd) day = 1;

             cday = day;
             if (day && (day <= maxdays))
               snprintf(buf, sizeof(buf), "%02d", day++);
             else
               buf[0] = 0;

             if (cday == inst->current_time.tm_mday
                 && month == inst->current_time.tm_mon
                 && time->tm_year == inst->current_time.tm_year)
               {
                  o = inst->calendar->o_today = edje_object_add(evas);

                  if (!e_theme_edje_object_set(o,
                        "base/theme/modules/calendar", "modules/calendar/today"))
                    edje_object_file_set(o, inst->edje_module, "modules/calendar/today");
                  edje_object_part_text_set(o, "e.text.label", buf);
                  evas_object_show(o);

                  e_widget_sub_object_add(table, o);
               }
             else
               o = e_widget_label_add(evas, buf);
             e_widget_table_object_append(table, o, col, row, 1, 1, 0, 0, 0, 0);
          }
        if (day > maxdays+1) break;
     }

   e_widget_list_object_append(inst->list, table, 1, 1, 0.5);
   e_widget_size_min_get(inst->list, &mw, &mh);
   if ((double) mw / mh > GOLDEN_RATIO)
     mh = mw / GOLDEN_RATIO;
   else if ((double) mw / mh < GOLDEN_RATIO - (double) 1)
     mw = mh * (GOLDEN_RATIO - (double) 1);
   evas_object_size_hint_min_set(inst->list, mw, mh);

   inst->table = table;
}

static void
_calendar_popup_content_create(Instance *inst)
{
   Evas_Object *list, *label, *oe, *ow;
   Evas *evas;
   Evas_Coord mw, mh;
   time_t current_time;
   struct tm *local_time;

   if (inst->popup) _calendar_popup_destroy(inst);
   inst->popup = e_gadcon_popup_new(inst->gcc);

   current_time = time (NULL);
   local_time = localtime_r(&current_time, &inst->displayed_time);
   inst->current_time = inst->displayed_time;

   evas = inst->popup->win->evas;
   list = e_widget_list_add(evas, 0, 0);

   label = e_widget_label_add(evas, NULL);

   oe = edje_object_add(evas);
   if (!e_theme_edje_object_set(oe,
                                "base/theme/modules/calendar",
                                "modules/calendar/header"))
     edje_object_file_set(oe, inst->edje_module, "modules/calendar/header");
   edje_object_part_swallow(oe, "content", label);
   edje_object_signal_callback_add(oe, "day", "today", _day_today, inst);
   edje_object_signal_callback_add(oe, "year", "minus", _year_minus, inst);
   edje_object_signal_callback_add(oe, "year", "plus", _year_plus, inst);
   edje_object_signal_callback_add(oe, "month", "minus", _month_minus, inst);
   edje_object_signal_callback_add(oe, "month", "plus", _month_plus, inst);
   evas_object_show(oe);

   ow = e_widget_add(evas);
   e_widget_data_set(ow, NULL);
   edje_object_size_min_get(oe, &mw, &mh);
   e_widget_size_min_set(ow, mw, mh);
   e_widget_sub_object_add(ow, oe);
   e_widget_resize_object_set(ow, oe);
   evas_object_show(ow);

   e_widget_list_object_append(list, ow, 1, 0, 0.5);

   inst->list = list;
   inst->label = label;

   _calendar_popup_content_populate(inst, local_time);

   e_gadcon_popup_content_set(inst->popup, inst->list);
}

static void
_calendar_popup_destroy(Instance *inst)
{
   if (!inst->popup) return;
   e_object_del(E_OBJECT(inst->popup));
}

static void
_cb_action(E_Object *obj __UNUSED__, const char *params __UNUSED__)
{
   Eina_List *l;
   Instance *inst;
   Evas_Coord w, h;

   EINA_LIST_FOREACH(calendar_conf->instances, l, inst)
     {
        if (!inst) continue;
        if (!inst->popup) continue;

        evas_object_geometry_get(inst->calendar->o_icon, NULL, NULL, &w, &h);
        if (w == 0 || h == 0) continue ;

        if (inst->popup->win->visible)
          {
             e_gadcon_popup_toggle_pinned(inst->popup);
             e_gadcon_popup_hide(inst->popup);
          }
        else
          {
             e_gadcon_popup_toggle_pinned(inst->popup);
             e_gadcon_popup_show(inst->popup);
          }
     }
}

static void
_cb_mouse_in(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, 
             void *event_info __UNUSED__)
{
   Instance *inst;

   if (!(inst = data)) return;
   e_gadcon_popup_show(inst->popup);
}

static void
_cb_mouse_out(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, 
              void *event_info __UNUSED__)
{
   Instance *inst;

   if (!(inst = data)) return;
   e_gadcon_popup_hide(inst->popup);
}

static void
_cb_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;
   if (ev->button == 1)
     {
        e_gadcon_popup_toggle_pinned(inst->popup);
     }
   if (ev->button == 3)
     {
        E_Menu *m, *mo;
        E_Menu_Item *mi;
        int cx, cy, cw, ch;

        m = e_menu_new();

        mo = e_menu_new();
        calendar_conf->menu_firstweekday = mo;

        mi = e_menu_item_new(mo);
        e_menu_item_label_set(mi, D_("Sunday"));
        e_menu_item_radio_set(mi, 1);
        e_menu_item_radio_group_set(mi, 1);
        if (!inst->ci->firstweekday) e_menu_item_toggle_set(mi, 1);
        e_menu_item_callback_set(mi, _calendar_firstweekday_su, inst);

        mi = e_menu_item_new(mo);
        e_menu_item_label_set(mi, D_("Monday"));
        e_menu_item_radio_set(mi, 1);
        e_menu_item_radio_group_set(mi, 1);
        if (inst->ci->firstweekday == 1) e_menu_item_toggle_set(mi, 1);
        e_menu_item_callback_set(mi, _calendar_firstweekday_mo, inst);

        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("First Day of Week"));
        e_util_menu_item_theme_icon_set(mi, "preferences-system");
        e_menu_item_submenu_set(mi, calendar_conf->menu_firstweekday);

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
        e_menu_post_deactivate_callback_set(m, _menu_cb_post, inst);

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy, &cw, &ch);
        e_menu_activate_mouse(m,
                              e_util_zone_current_get(e_manager_current_get()),
                              cx + ev->output.x, cy + ev->output.y, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void
_menu_cb_post(void *data __UNUSED__, E_Menu *m __UNUSED__)
{
   if (calendar_conf->menu_firstweekday)
     e_object_del(E_OBJECT(calendar_conf->menu_firstweekday));
   calendar_conf->menu_firstweekday = NULL;
}

static void
_calendar_firstweekday_su(void *data, E_Menu *m __UNUSED__, E_Menu_Item *mi __UNUSED__)
{
   Instance *inst;
 
   inst = data;

   inst->ci->firstweekday = 0;
   e_config_save_queue();

   _update_calendar_sheet(inst);
}

static void
_calendar_firstweekday_mo(void *data, E_Menu *m __UNUSED__, E_Menu_Item *mi __UNUSED__)
{
   Instance *inst;
 
   inst = data;

   inst->ci->firstweekday = 1;
   e_config_save_queue();

   _update_calendar_sheet(inst);
}

EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION, "Calendar"
};

EAPI void *
e_modapi_init(E_Module *m) 
{
   /* Set up module locales*/
   bindtextdomain(LOCALEDOMAIN, LOCALEDIR);
   bind_textdomain_codeset(LOCALEDOMAIN, "UTF-8");

   conf_item_edd = E_CONFIG_DD_NEW("Calendar_Config_Item", Config_Item);
   conf_edd = E_CONFIG_DD_NEW("Calendar_Config", Config);

   E_CONFIG_VAL(conf_item_edd, Config_Item, id, STR);
   E_CONFIG_VAL(conf_item_edd, Config_Item, firstweekday, INT);
   E_CONFIG_LIST(conf_edd, Config, items, conf_item_edd);

   calendar_conf = e_config_domain_load("module.calendar", conf_edd);
   if (!calendar_conf)
     calendar_conf = E_NEW(Config, 1);

   calendar_conf->module = m;
   e_gadcon_provider_register(&_gc_class);
   /* add module supplied action */
   act = e_action_add("calendar");
   if (act)
     {
        act->func.go = _cb_action;
        e_action_predef_name_set(D_("Calendar"),
                                 D_("Monthview Popup (Show/Hide)"),
                                    "calendar", "<none>", NULL, 0);
     }
   calendar_conf->timer = ecore_timer_add(1, _update_date, calendar_conf);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   if (calendar_conf->timer) ecore_timer_del(calendar_conf->timer);
   calendar_conf->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);
   /* remove module-supplied action */
   if (act)
     {
        e_action_predef_name_del(D_("Calendar"), D_("Monthview Popup (Show/Hide)"));
        e_action_del("calendar");
        act = NULL;
     }

   while (calendar_conf->items)
     {
        Config_Item *ci;

        ci = calendar_conf->items->data;
        if (ci->id)
          eina_stringshare_del(ci->id);
        calendar_conf->items = eina_list_remove_list(calendar_conf->items, calendar_conf->items);
        E_FREE(ci);
     }

   E_FREE(calendar_conf);
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m __UNUSED__)
{
   e_config_domain_save("module.calendar", conf_edd, calendar_conf);
   return 1;
}
