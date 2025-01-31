#include <e.h>
#include "e_mod_main.h"

/* gadcon requirements */
static E_Gadcon_Client *_gc_init (E_Gadcon * gc, const char *name,
                                  const char *id, const char *style);
static void _gc_shutdown (E_Gadcon_Client * gcc);
static void _gc_orient (E_Gadcon_Client * gcc, E_Gadcon_Orient orient);
static const char *_gc_label (const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon (const E_Gadcon_Client_Class *client_class, Evas * evas);
static const char *_gc_id_new (const E_Gadcon_Client_Class *client_class);

/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class = {
  GADCON_CLIENT_CLASS_VERSION,
  "winselector",
  {
     _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
     e_gadcon_site_is_not_toolbar
  },
  E_GADCON_CLIENT_STYLE_PLAIN
};

/* actual module specifics */
typedef struct _Instance Instance;
struct _Instance
{
  E_Gadcon_Client *gcc;
  Evas_Object     *o_button;
  Evas_Object     *bd_icon;

  Eina_List       *handlers;
};

static void _button_cb_mouse_down (void *data, Evas * e, Evas_Object * obj,
                                   void *event_info);
static void _menu_cb_post(void *data, E_Menu *m);
static E_Menu * _win_menu_new(Instance *inst);
static void _win_menu_pre_cb(void *data, E_Menu *m);
static void _win_menu_item_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void _win_menu_icon_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void _win_menu_item_drag(void *data, E_Menu *m, E_Menu_Item *mi);
static void _win_menu_free_hook(void *obj);
static void _win_menu_item_create(E_Border *bd, E_Menu *m);
static Eina_Bool _window_cb_focus_in(void *data, int type, void *event);
static Eina_Bool _window_cb_focus_out(void *data, int type, void *event);
static Eina_Bool _window_cb_icon_change(void *data, int type, void *event);
static Eina_Bool _window_cb_border_remove(void *data, int type, void *event);
static void _focus_in(E_Border *bd, Instance *inst);
static void _focus_out(Instance *inst);
static void _win_menu_fill_separator(E_Menu *m);
static int _win_menu_fill_normal(Eina_List *normal, E_Menu *m);
static int _win_menu_fill_alt(Eina_List *alt, E_Menu *m);
static int _win_menu_fill_iconified  (Eina_List *borders, E_Menu *m);
static int _win_menu_sort_alpha_cb (const void *d1, const void *d2);
static int _win_menu_sort_z_order_cb (const void *d1, const void *d2);
static int _win_menu_group_desk_cb (const void *d1, const void *d2);
static int  _win_menu_group_class_cb (const void *d1, const void *d2);
static void _win_menu_add_by_class (Eina_List *borders, E_Menu *m);
static void _win_menu_add_by_desk (E_Desk *curr_desk, Eina_List *borders, E_Menu *m, E_Gadcon_Orient orient);
static void _win_menu_add_by_none (Eina_List *borders, E_Menu *m);

static E_Module *winsel_module = NULL;

static E_Gadcon_Client *
_gc_init (E_Gadcon * gc, const char *name, const char *id, const char *style)
{
  Evas_Object *o;
  E_Gadcon_Client *gcc;
  Instance *inst;
  char buf[PATH_MAX];

  inst = E_NEW (Instance, 1);

  snprintf (buf, sizeof (buf), "%s/winselector.edj", e_module_dir_get
           (winsel_module));
  o = edje_object_add (gc->evas);
  if (!e_theme_edje_object_set (o, "base/theme/modules/winselector",
                               "modules/winselector/main"))
    edje_object_file_set (o, buf, "modules/winselector/main");
  evas_object_show (o);
  edje_object_signal_emit (o, "passive", "");

  gcc = e_gadcon_client_new (gc, name, id, style, o);
  gcc->data = inst;

  inst->gcc = gcc;
  inst->o_button = o;

  e_gadcon_client_util_menu_attach (gcc);

  inst->handlers = eina_list_append (inst->handlers, ecore_event_handler_add
        (ECORE_X_EVENT_WINDOW_FOCUS_IN, _window_cb_focus_in, inst));
  inst->handlers = eina_list_append (inst->handlers, ecore_event_handler_add
        (ECORE_X_EVENT_WINDOW_FOCUS_OUT, _window_cb_focus_out, inst));
  inst->handlers = eina_list_append (inst->handlers, ecore_event_handler_add
        (E_EVENT_BORDER_ICON_CHANGE, _window_cb_icon_change, inst));
  inst->handlers = eina_list_append (inst->handlers, ecore_event_handler_add
        (E_EVENT_BORDER_REMOVE, _window_cb_border_remove, inst));
  evas_object_event_callback_add (o, EVAS_CALLBACK_MOUSE_DOWN,
        _button_cb_mouse_down, inst);
  return gcc;
}

static void
_gc_shutdown (E_Gadcon_Client * gcc)
{
   Instance *inst = gcc->data;

  if (inst->bd_icon)
    {
       edje_object_part_unswallow(inst->o_button, inst->bd_icon);
       evas_object_del(inst->bd_icon);
       inst->bd_icon = NULL;
    }
  if (inst->o_button)
    {
       evas_object_del(inst->o_button);
       inst->o_button = NULL;
    }

  while (inst->handlers) 
    {
       ecore_event_handler_del(inst->handlers->data);
       inst->handlers = eina_list_remove_list(inst->handlers, inst->handlers);
    }
  free (inst);
}

static void
_gc_orient (E_Gadcon_Client * gcc, E_Gadcon_Orient orient __UNUSED__)
{
  e_gadcon_client_aspect_set (gcc, 16, 16);
  e_gadcon_client_min_size_set (gcc, 16, 16);
}

static const char *
_gc_label (const E_Gadcon_Client_Class *client_class __UNUSED__)
{
  return "Winselector";
}

static Evas_Object *
_gc_icon (const E_Gadcon_Client_Class *client_class __UNUSED__, Evas * evas)
{
  Evas_Object *o;
  char buf[PATH_MAX];

  o = edje_object_add (evas);
  snprintf (buf, sizeof (buf), "%s/e-module-winselector.edj",
            e_module_dir_get (winsel_module));
  edje_object_file_set (o, buf, "icon");
  return o;
}

static const char *
_gc_id_new (const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   return _gadcon_class.name;
}

static void
_button_cb_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
                      void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;
   
   inst = data;
   ev = event_info;
   if (ev->button == 1)
     {
        Evas_Coord x, y, w, h;
        int cx, cy, cw, ch;
        E_Menu *m;

        evas_object_geometry_get(inst->o_button, &x, &y, &w, &h); 
        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon,
            &cx, &cy, &cw, &ch);
        x += cx;
        y += cy;
        m = _win_menu_new(inst);

        int dir;

        e_menu_post_deactivate_callback_set(m, _menu_cb_post,
            inst);
        dir = E_MENU_POP_DIRECTION_AUTO;
        switch (inst->gcc->gadcon->orient)
           {
           case E_GADCON_ORIENT_TOP:
             dir = E_MENU_POP_DIRECTION_DOWN;
             break;
           case E_GADCON_ORIENT_BOTTOM:
             dir = E_MENU_POP_DIRECTION_UP;
             break;
           case E_GADCON_ORIENT_LEFT:
             dir = E_MENU_POP_DIRECTION_RIGHT;
             break;
           case E_GADCON_ORIENT_RIGHT:
             dir = E_MENU_POP_DIRECTION_LEFT;
             break;
           case E_GADCON_ORIENT_CORNER_TL:
             dir = E_MENU_POP_DIRECTION_DOWN;
             break;
           case E_GADCON_ORIENT_CORNER_TR:
             dir = E_MENU_POP_DIRECTION_DOWN;
             break;
           case E_GADCON_ORIENT_CORNER_BL:
             dir = E_MENU_POP_DIRECTION_UP;
             break;
           case E_GADCON_ORIENT_CORNER_BR:
             dir = E_MENU_POP_DIRECTION_UP;
             break;
           case E_GADCON_ORIENT_CORNER_LT:
             dir = E_MENU_POP_DIRECTION_RIGHT;
             break;
           case E_GADCON_ORIENT_CORNER_RT:
             dir = E_MENU_POP_DIRECTION_LEFT;
             break;
           case E_GADCON_ORIENT_CORNER_LB:
             dir = E_MENU_POP_DIRECTION_RIGHT;
             break;
           case E_GADCON_ORIENT_CORNER_RB:
             dir = E_MENU_POP_DIRECTION_LEFT;
             break;
           case E_GADCON_ORIENT_FLOAT:
           case E_GADCON_ORIENT_HORIZ:
           case E_GADCON_ORIENT_VERT:
           default:
             dir = E_MENU_POP_DIRECTION_AUTO;
             break;
          }
          e_menu_activate_mouse(m,
               e_util_zone_current_get(e_manager_current_get()),
               x, y, w, h, dir, ev->timestamp);
         edje_object_signal_emit(inst->o_button, "active", "");
         evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
              EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static E_Menu *
_win_menu_new(Instance *inst)
{
   E_Menu *m;

   m = e_menu_new();
   e_menu_title_set(m, "Applications");
   e_menu_pre_activate_callback_set(m, _win_menu_pre_cb, inst);
   return m;
}

static void
_win_menu_pre_cb(void *data, E_Menu *m)
{
   E_Menu_Item *mi;
   Eina_List *l, *borders = NULL;
   E_Zone *zone = NULL;
   E_Desk *desk = NULL;
   Instance *inst;

   inst = data;
   e_menu_pre_activate_callback_set(m, NULL, NULL);
   /* get the current clients */
   zone = e_util_zone_current_get (e_manager_current_get ());
   desk = e_desk_current_get(zone);
   for (l = e_border_client_list(); l; l = l->next)
     {
        E_Border *border;

        border = l->data;
        if (border->client.netwm.state.skip_taskbar) continue;
        if (border->user_skip_winlist) continue;
        if ((border->zone == zone) || (border->iconic))
          borders = eina_list_append(borders, border);
     }

   if (!borders)
     {
        /* FIXME here we want nothing, but that crashes!!! */
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi,  D_("No windows"));
        return;
     }

   /* Sort the borders */
   if (e_config->clientlist_sort_by == E_CLIENTLIST_SORT_ALPHA)
     borders = eina_list_sort(borders, eina_list_count(borders), 
           _win_menu_sort_alpha_cb);

   if (e_config->clientlist_sort_by == E_CLIENTLIST_SORT_ZORDER)
     borders = eina_list_sort(borders, eina_list_count(borders),
           _win_menu_sort_z_order_cb);

   /* Group the borders */
   if (e_config->clientlist_group_by == E_CLIENTLIST_GROUP_DESK)
     {
        borders = eina_list_sort(borders, eina_list_count(borders), 
            _win_menu_group_desk_cb);
        _win_menu_add_by_desk(desk, borders, m, inst->gcc->gadcon->orient);
     }
   if (e_config->clientlist_group_by == E_CLIENTLIST_GROUP_CLASS) 
     {
        borders = eina_list_sort(borders, eina_list_count(borders), 
             _win_menu_group_class_cb);
       _win_menu_add_by_class(borders, m);
     }
   if (e_config->clientlist_group_by == E_CLIENTLIST_GROUP_NONE)
     _win_menu_add_by_none(borders, m);

   e_object_free_attach_func_set(E_OBJECT(m), _win_menu_free_hook);
   e_object_data_set(E_OBJECT(m), borders);
}

static void
_win_menu_item_create(E_Border *bd, E_Menu *m)
{
   E_Menu_Item *mi;
   const char *title;

   title = e_border_name_get(bd);
   mi = e_menu_item_new(m);
   e_menu_item_check_set(mi, 1);
   if ((title) && (title[0]))
     e_menu_item_label_set(mi, title);
   else
     e_menu_item_label_set(mi, D_("No name!"));
   /* ref the border as we implicitly unref it in the callback */
   e_object_ref(E_OBJECT(bd));
/*   e_object_breadcrumb_add(E_OBJECT(bd), "clients_menu");*/
   e_menu_item_callback_set(mi, _win_menu_item_cb, bd);
   e_menu_item_realize_callback_set(mi, _win_menu_icon_cb, bd);
   if (!bd->iconic) e_menu_item_toggle_set(mi, 1);
}

static void 
_win_menu_item_cb(void *data, E_Menu *m __UNUSED__, E_Menu_Item *mi __UNUSED__)
{
   E_Border *bd;
   
   bd = data;
   E_OBJECT_CHECK(bd);
   if (bd->iconic)
     {
        if (!bd->lock_user_iconify)
          e_border_uniconify(bd);
     }
   e_desk_show(bd->desk);
   if (!bd->lock_user_stacking)
     e_border_raise(bd);
   if (!bd->lock_focus_out)
     {
       if (e_config->focus_policy != E_FOCUS_CLICK)
          ecore_x_pointer_warp(bd->zone->container->win,
                               bd->x + (bd->w / 2),
                               bd->y + (bd->h / 2));
         e_border_focus_set(bd, 1, 1);
     }
}

static void 
_win_menu_icon_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;
   Evas_Object *o;
   
   bd = data;
   E_OBJECT_CHECK(bd);

   o = e_icon_add(m->evas);
   e_icon_edje_object_set(o, e_border_icon_add(bd, m->evas));
   e_menu_item_drag_callback_set(mi, _win_menu_item_drag, bd);

   mi->icon_object = o;
}

static void
_win_menu_item_drag(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   if (!(bd = data)) return;
   if (!mi->icon_object) return;

   E_Drag *drag;
   Evas_Object *o = NULL;
   Evas_Coord x, y, w, h;
   const char *drag_types[] = { "enlightenment/border" };

   evas_object_geometry_get(mi->icon_object,
                            &x, &y, &w, &h);
   drag = e_drag_new(
      m->zone->container, x, y,
      drag_types, 1, bd, -1, NULL, NULL);

   o = e_icon_add(drag->evas);
   e_icon_edje_object_set(o, e_border_icon_add(bd, drag->evas));
   e_drag_object_set(drag, o);

   e_drag_resize(drag, w, h);
   e_drag_start(drag, mi->drag.x + w, mi->drag.y + h);
}

static void
_win_menu_free_hook(void *obj)
{
   E_Menu *m;
   Eina_List *borders;
   
   m = obj;
   borders = e_object_data_get(E_OBJECT(m));
   while (borders)
     {
        E_Border *bd;

        bd = borders->data;
        borders = eina_list_remove_list(borders, borders);
//	e_object_breadcrumb_del(E_OBJECT(bd), "clients_menu");
        e_object_unref(E_OBJECT(bd));
     }
}

static void
_menu_cb_post(void *data, E_Menu *m __UNUSED__)
{
   Instance *inst = data;

   if (!inst->gcc->menu) return;
   edje_object_signal_emit(inst->o_button, "passive", "");
}

static Eina_Bool _window_cb_focus_in(void *data, int type __UNUSED__, void *event)
{
   Ecore_X_Event_Window_Focus_In *ev;
   E_Border *bd;
   Instance *inst;

   ev = event;
   inst = data;
   if (!(bd = e_border_find_by_client_window(ev->win)))
     return EINA_TRUE;

   _focus_in(bd, inst);
   return EINA_TRUE;
}

static Eina_Bool _window_cb_focus_out(void *data, int type __UNUSED__, void *event)
{ 
   Ecore_X_Event_Window_Focus_In *ev;
   E_Border *bd;
   Instance *inst;

   ev = event;
   inst = data;
   if (!(bd = e_border_find_by_client_window(ev->win)))
     return EINA_TRUE;

   _focus_out(inst);

   return EINA_TRUE;
}

static Eina_Bool
_window_cb_icon_change(void *data, int type __UNUSED__, void *event)
{
   E_Event_Border_Icon_Change *ev;
   E_Border *bd;
   Instance *inst;

   ev = event;
   inst = data;
   if (!(bd = ev->border)) return EINA_TRUE;

   _focus_in(bd, inst);
   return EINA_TRUE;
}

static Eina_Bool _window_cb_border_remove(void *data, int type __UNUSED__, void *event)
{ 
   E_Event_Border_Remove *ev;
   E_Border *bd;
   Instance *inst;

   ev = event;
   inst = data;
   if (!(bd = ev->border)) return EINA_TRUE;

   _focus_out(inst);

   return EINA_TRUE;
}

static void _focus_in(E_Border *bd, Instance *inst)
{
   if (!inst->o_button)
     return;

   _focus_out(inst);
   inst->bd_icon = e_border_icon_add(bd, evas_object_evas_get(inst->o_button));
   edje_object_signal_emit(inst->o_button, "focus_in", "");
   edje_object_part_swallow(inst->o_button, "e.swallow.icon", inst->bd_icon);
   evas_object_show(inst->bd_icon);
}

static void _focus_out(Instance *inst)
{
   if (!inst->o_button)
     return;

   edje_object_signal_emit(inst->o_button, "focus_out", "");
   if (inst->bd_icon)
     {
        evas_object_del(inst->bd_icon);
        inst->bd_icon = NULL;
     }
}

static int _win_menu_sort_alpha_cb(const void *d1, const void *d2)
{
   const E_Border *bd1;
   const E_Border *bd2;
   const char *name1;
   const char *name2;
   
   if (!d1) return 1;
   if (!d2) return -1;

   bd1 = d1;
   bd2 = d2;
   name1 = e_border_name_get(bd1);
   name2 = e_border_name_get(bd2);
   
   if (strcasecmp(name1, name2) > 0) return 1;
   if (strcasecmp(name1, name2) < 0) return -1;
   return 0;
}

static int _win_menu_sort_z_order_cb(const void *d1, const void *d2)
{
   const E_Border *bd1;
   const E_Border *bd2;

   if (!d1) return 1;
   if (!d2) return -1;

   bd1 = d1;
   bd2 = d2;

   if (bd1->layer < bd2->layer) return 1;
   if (bd1->layer > bd2->layer) return -1;
   return 0;   
}

static int _win_menu_group_desk_cb(const void *d1, const void *d2)
{
   const E_Border *bd1;
   const E_Border *bd2;
   int j,k;

   if (!d1) return 1;
   if (!d2) return -1;

   bd1 = d1;
   bd2 = d2;

   j = bd1->desk->y * 12 + bd1->desk->x;
   k = bd2->desk->y * 12 + bd2->desk->x;

   if (j > k) return 1;
   if (j < k) return -1;
   return -1;   /* Returning '-1' on equal is intentional */
}

static int _win_menu_group_class_cb(const void *d1, const void *d2)
{
   const E_Border *bd1;
   const E_Border *bd2;

   if (!d1) return 1;
   if (!d2) return -1;

   bd1 = d1;
   bd2 = d2;

   if (strcmp((const char*)bd1->client.icccm.class, 
              (const char*)bd2->client.icccm.class) > 0)
     return 1;

   if (strcmp((const char*)bd1->client.icccm.class, 
              (const char*)bd2->client.icccm.class) < 0)
     return -1;

   return -1;   /* Returning '-1' on equal is intentional */
}

static void _win_menu_add_by_class(Eina_List *borders, E_Menu *m)
{
   Eina_List *l = NULL, *ico = NULL;
   E_Menu *subm = NULL;
   E_Menu_Item *mi = NULL;
   char *class = NULL;

   class = strdup("");
   for (l = borders; l; l = l->next) 
     { 
        E_Border *bd; 
        bd = l->data; 

        if ((bd->iconic) && 
            (e_config->clientlist_separate_iconified_apps == E_CLIENTLIST_GROUPICONS_SEP))
          { 
             ico = eina_list_append(ico, bd); 
             continue;
          }

        if (((strcmp(class, bd->client.icccm.class) != 0) && 
            e_config->clientlist_separate_with != E_CLIENTLIST_GROUP_SEP_NONE))
          {
             if (e_config->clientlist_separate_with == E_CLIENTLIST_GROUP_SEP_MENU)
               { 
                  if (subm && mi) 
                    e_menu_item_submenu_set(mi, subm);
                  mi = e_menu_item_new(m);
                  e_menu_item_label_set(mi, bd->client.icccm.class);
                  e_util_menu_item_theme_icon_set(mi, "preferences-system-windows");
                  subm = e_menu_new();
               }
             else
               {
                  mi = e_menu_item_new(m); 
                  e_menu_item_separator_set(mi, 1);
               }
             class = strdup(bd->client.icccm.class);
          }
        if (e_config->clientlist_separate_with == E_CLIENTLIST_GROUP_SEP_MENU) 
          _win_menu_item_create(bd, subm);
        else
          _win_menu_item_create(bd, m);
     }

   if ((e_config->clientlist_separate_with == E_CLIENTLIST_GROUP_SEP_MENU)
       && subm && mi)
     e_menu_item_submenu_set(mi, subm);

   _win_menu_fill_iconified(ico, m);
}

static void _win_menu_add_by_desk(E_Desk *curr_desk, Eina_List *borders, E_Menu *m, E_Gadcon_Orient orient)
{
   Eina_List *l = NULL, *normal = NULL, *alt = NULL, *ico = NULL;

   /* Deal with present desk first */
   for (l = borders; l; l = l->next)
     {
        E_Border *bd;

        bd = l->data;
        if (bd->iconic && e_config->clientlist_separate_iconified_apps && E_CLIENTLIST_GROUPICONS_SEP)
          {
             ico = eina_list_append(ico, bd);
             continue;
          }

        if (bd->desk != curr_desk)
          {
             if ((!bd->iconic) || 
                  (bd->iconic && e_config->clientlist_separate_iconified_apps == 
                 E_CLIENTLIST_GROUPICONS_OWNER))
               {
                  alt = eina_list_append(alt, bd);
                  continue;
               }
          }
        else
          normal = eina_list_append(normal, bd);
     }

   if (orient == E_GADCON_ORIENT_BOTTOM 
         || orient == E_GADCON_ORIENT_CORNER_BL
         || orient == E_GADCON_ORIENT_CORNER_BR
         || orient == E_GADCON_ORIENT_CORNER_RB
         || orient == E_GADCON_ORIENT_CORNER_LB)
     {
        if (_win_menu_fill_iconified(ico, m))
          _win_menu_fill_separator(m);
        if (_win_menu_fill_alt(alt, m) && eina_list_count(normal) > 0)
          _win_menu_fill_separator(m);

        _win_menu_fill_normal(normal, m);
     }
   else
     {
        if (_win_menu_fill_normal(normal, m))
          _win_menu_fill_separator(m);
        if (_win_menu_fill_alt(alt, m) && eina_list_count(ico) > 0)
          _win_menu_fill_separator(m);

        _win_menu_fill_iconified(ico, m);
     }
}

static void _win_menu_add_by_none(Eina_List *borders, E_Menu *m)
{
   Eina_List *l = NULL, *ico = NULL;

   for (l = borders; l; l = l->next)
     {
        E_Border *bd;

        bd = l->data;
        if (bd->iconic && e_config->clientlist_separate_iconified_apps && E_CLIENTLIST_GROUPICONS_SEP) 
          {
             ico = eina_list_append(ico, bd);
             continue; 
          }
        _win_menu_item_create(bd, m);
     }

   _win_menu_fill_iconified(ico, m);
}

static void _win_menu_fill_separator(E_Menu *m)
{
   E_Menu_Item *mi = NULL;

   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);
}

static int _win_menu_fill_normal(Eina_List *normal, E_Menu *m)
{
   Eina_List *l = NULL;

   if (eina_list_count(normal) > 0)
     {
        for (l = normal; l; l = l->next)
          {
             E_Border *bd;

             bd = l->data; 
             _win_menu_item_create(bd, m);
          }
        return 1;
     }
   return 0;
}

static int _win_menu_fill_alt(Eina_List *alt, E_Menu *m)
{
   E_Menu_Item *mi = NULL;
   E_Menu *subm = NULL;
   E_Desk *desk = NULL;
   Eina_List *l = NULL;
   int first = 1;

   if (eina_list_count(alt) > 0)
     {
        for (l = alt; l; l = l->next)
          {
             E_Border *bd;

             bd = l->data;

             if (bd->desk != desk && 
                 e_config->clientlist_separate_with != E_CLIENTLIST_GROUP_SEP_NONE)
               {
                  if (e_config->clientlist_separate_with == E_CLIENTLIST_GROUP_SEP_MENU)
                    {
                       if (subm && mi)
                         e_menu_item_submenu_set(mi, subm);
                       mi = e_menu_item_new(m);
                       e_menu_item_label_set(mi, bd->desk->name);
                       e_util_menu_item_theme_icon_set(mi, "preferences-desktop");
                       subm = e_menu_new();
                    }
                  else
                    {
                       if (!first)
                         {
                            mi = e_menu_item_new(m); 
                            e_menu_item_separator_set(mi, 1);
                         }
                       first = 0;
                    }
                  desk = bd->desk;
               }
             if (e_config->clientlist_separate_with == E_CLIENTLIST_GROUP_SEP_MENU) 
               _win_menu_item_create(bd, subm); 
             else
              _win_menu_item_create(bd, m);
          }
        if (e_config->clientlist_separate_with == E_CLIENTLIST_GROUP_SEP_MENU
           && subm && mi)
          e_menu_item_submenu_set(mi, subm);
        return 1;
     }
   return 0;
}

static int _win_menu_fill_iconified(Eina_List *borders, E_Menu *m)
{
   Eina_List *l;

   if (eina_list_count(borders) > 0)
     {
        for (l = borders; l; l = l->next)
          {
             E_Border *bd; 
             bd = l->data; 

             _win_menu_item_create(bd, m);
          }
        return 1;
     }
   return 0;
}

/* module setup */
EAPI E_Module_Api e_modapi = {
   E_MODULE_API_VERSION,
   "Winselector"
};

EAPI void *
e_modapi_init (E_Module * m)
{
   /* Set up module locales*/
   bindtextdomain(LOCALEDOMAIN, LOCALEDIR);
   bind_textdomain_codeset(LOCALEDOMAIN, "UTF-8");
   winsel_module = m;
   e_gadcon_provider_register (&_gadcon_class);
   return winsel_module;
}

EAPI int
e_modapi_shutdown (E_Module * m __UNUSED__)
{
   winsel_module = NULL;
   e_gadcon_provider_unregister (&_gadcon_class);
   return 1;
}

EAPI int
e_modapi_save (E_Module * m __UNUSED__)
{
   return 1;
}
