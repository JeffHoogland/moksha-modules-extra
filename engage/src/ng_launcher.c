#include "e_mod_main.h"

/*
  TODO
  - update on icon changes

*/

static void      _box_fill           (Ngi_Box *box);
static void      _app_change_cb      (void *data, E_Order *eo);

static void      _item_new           (Ngi_Box *box, Efreet_Desktop *dekstop, int instant, Ngi_Item_Launcher *after);
static void      _item_fill          (Ngi_Item_Launcher *it);

static void      _item_cb_free       (Ngi_Item *it);
static void      _item_cb_drag_start (Ngi_Item *it);
static void      _item_cb_mouse_up   (Ngi_Item *it, Ecore_Event_Mouse_Button *ev);
static void      _item_cb_mouse_down (Ngi_Item *it, Ecore_Event_Mouse_Button *ev);

static void      _cb_drop_enter      (void *data, const char *type, void *event_info);
static void      _cb_drop_leave      (void *data, const char *type, void *event_info);
static void      _cb_drop_move       (void *data, const char *type, void *event_info);
static void      _cb_drop_end        (void *data, const char *type, void *event_info);
static Eina_Bool  _cb_icons_update(void *data, int type, void *event);


void
ngi_launcher_new(Ng *ng, Config_Box *cfg)
{
   char buf[4096];
   Ecore_Event_Handler *h;
   Ngi_Box *box = ngi_box_new(ng);

   box->cfg = cfg;
   cfg->box = box;

   const char *drop[] =
     { "enlightenment/desktop",
       "enlightenment/border",
       "text/uri-list" };

   box->drop_handler = e_drop_handler_add
     (ng->win->drop_win, box,
      _cb_drop_enter, _cb_drop_move,
      _cb_drop_leave, _cb_drop_end,
      drop, 3, 0, 0, 0, 0);

   h = ecore_event_handler_add(E_EVENT_CONFIG_ICON_THEME, _cb_icons_update, box);
   if (h) box->handlers = eina_list_append(box->handlers, h);
   h = ecore_event_handler_add(EFREET_EVENT_ICON_CACHE_UPDATE, _cb_icons_update, box);
   if (h) box->handlers = eina_list_append(box->handlers, h);
   h = ecore_event_handler_add(EFREET_EVENT_DESKTOP_CACHE_UPDATE, _cb_icons_update, box);
   if (h) box->handlers = eina_list_append(box->handlers, h);
   
   if (!cfg->launcher_app_dir || strlen(cfg->launcher_app_dir) == 0)
     return;

   if (cfg->launcher_app_dir[0] != '/')
     {
        snprintf(buf, sizeof(buf), "%s/.e/e/applications/bar/%s/.order",
                 e_user_homedir_get(), cfg->launcher_app_dir);
     }
   else
     snprintf(buf, sizeof(buf), "%s", cfg->launcher_app_dir);

   box->apps = e_order_new(buf);
   e_order_update_callback_set(box->apps, _app_change_cb, box);

   _box_fill(box);
}

void
ngi_launcher_remove(Ngi_Box *box)
{
   Ecore_Event_Handler *h;
   
   e_drop_handler_del(box->drop_handler);

   if (box->apps)
     {
        e_order_update_callback_set(box->apps, NULL, NULL);
        e_object_del(E_OBJECT(box->apps));
     }

   EINA_LIST_FREE(box->handlers, h)
     ecore_event_handler_del(h);

   ngi_box_free(box);
}

static void
_box_fill(Ngi_Box *box)
{
   Efreet_Desktop *desktop;
   Eina_List *l;

   if (!box->apps)
     return;

   ngi_freeze(box->ng);

   EINA_LIST_FOREACH(box->apps->desktops, l, desktop)
     _item_new(box, desktop, 1, NULL);

   ngi_thaw(box->ng);
}

static Eina_Bool
_cb_icons_update(void *data, int type, void *event)
{
   Ngi_Box *box = (Ngi_Box *)data;
   Eina_List *l;
   Ngi_Item_Launcher *it;

   EINA_LIST_FOREACH(box->items, l, it)
     _item_fill(it); 
   
   return ECORE_CALLBACK_PASS_ON;
}

static void
_app_change_cb(void *data, E_Order *eo)
{
   Ngi_Box *box = (Ngi_Box *)data;

   if (box->item_drop)
     return;

   while(box->items)
     ngi_item_free(box->items->data);

   _box_fill(box);
}

/* ******************************* LAUNCHER DROP HANDLER ******************** */

static Ngi_Item *
_drop_item_new(Ngi_Box *box)
{
   Ngi_Item *it;

   it = (Ngi_Item*)E_NEW(Ngi_Item_Launcher, 1);
   it->box = box;
   ngi_item_show(it, 0);
   it->delete_me = EINA_TRUE;
   
   return it;
}

static void
_drop_handle_move(Ngi_Box *box, int x, int y)
{
   Ngi_Item *item;
   Ngi_Item_Launcher *it, *it2;
   Eina_List *l;

   if (box->ng->horizontal)
     box->ng->pos = x + box->ng->size/2 - box->ng->zone->x;
   else
     box->ng->pos = y + box->ng->size/2 - box->ng->zone->y;

   item = ngi_item_at_position_get(box->ng);

   if (item && (item->box != box))
     {
	l = eina_list_last(box->items);
	if (l && (it = l->data) && (!it->app)) return;

	ngi_item_remove(box->item_drop);

   	box->item_drop = _drop_item_new(box);
   	box->items = eina_list_append(box->items, box->item_drop);
     }
   else if (item && item->box == box)
     {
	it = (Ngi_Item_Launcher*)item;
	l = eina_list_data_find_list(box->items, it);

   	if (l) it2 = eina_list_data_get(l->prev);

   	if (it->app && (!it2 || it2->app))
   	  {
	     ngi_item_remove(box->item_drop);

   	     box->item_drop = _drop_item_new(box);
   	     box->items = eina_list_prepend_relative(box->items, box->item_drop, it);
   	  }
     }
}

static void
_cb_drop_enter(void *data, const char *type, void *event_info)
{
   E_Event_Dnd_Enter *ev = (E_Event_Dnd_Enter *)event_info;
   Ngi_Box *box = (Ngi_Box *)data;

   if (!strcmp(type, "text/uri-list"))
     return;

   _drop_handle_move(box, ev->x, ev->y);

   box->ng->dnd = 1;
   ngi_mouse_in(box->ng);
}

static void
_cb_drop_move(void *data, const char *type, void *event_info)
{
   E_Event_Dnd_Move *ev = (E_Event_Dnd_Move *)event_info;
   Ngi_Box *box = (Ngi_Box *)data;

   if (!strcmp(type, "text/uri-list"))
     return;

   _drop_handle_move(box, ev->x, ev->y);

   ngi_thaw(box->ng);
}

static void
_cb_drop_leave(void *data, const char *type, void *event_info)
{
   Ngi_Box *box = (Ngi_Box *)data;

   if (!strcmp(type, "text/uri-list"))
     return;

   if (box->item_drop)
     ngi_item_remove(box->item_drop);
   box->item_drop = NULL;

   ngi_reposition(box->ng);
   ngi_input_extents_calc(box->ng);
   box->ng->dnd = 0;
   ngi_mouse_out(box->ng);
}

static void
_cb_drop_end(void *data, const char *type, void *event_info)
{
   E_Event_Dnd_Drop *ev = (E_Event_Dnd_Drop *)event_info;
   Ngi_Box *box = data;
   Efreet_Desktop *app = NULL;
   Ngi_Item_Launcher *it = NULL;

   Eina_List *l;
   if (!strcmp(type, "enlightenment/desktop"))
     {
        app = (Efreet_Desktop *)ev->data;
     }
   else if (!strcmp(type, "enlightenment/border"))
     {
        E_Border *bd = (E_Border *)ev->data;
        app = bd->desktop;

	if (bd->internal)
	  {
	     const char *class = bd->client.icccm.class;

	     if ((class) && (!strncmp(class, "e_fwin::", 8)) &&
		 (ecore_file_exists(class+8)))
	       {
		  const char *file = class+8;
		  char buf[PATH_MAX];

		  if (!app)
		    {
		       app = e_desktop_border_create(bd);
		       
		       if (app->name) free(app->name);
		       app->name = strdup(ecore_file_file_get(file)); 
		       if (app->icon) free(app->icon);
		       app->icon = strdup("folder");
		       if (app->comment)free(app->comment);
		       app->comment = strdup(D_("Open Directory"));
		       if (app->exec) free(app->exec);
		       snprintf(buf, PATH_MAX, "enlightenment_remote -efm-open-dir %s", file);
		       app->exec = strdup(buf);

		       efreet_desktop_save(app);
		    }
	       }
	  }
        else if (!app)
          {
             app = e_desktop_border_create(bd);
             efreet_desktop_save(app);
             e_desktop_edit(e_container_current_get(e_manager_current_get()), app);
          }
     }
   else if (!strcmp(type, "text/uri-list"))
     return;

   if (!app)
     return;

   l = eina_list_data_find_list(box->items, box->item_drop);
   if (l) it = eina_list_data_get(l->next);

   if (box->item_drop)
     ngi_item_free(box->item_drop);

   if (it && it->app)
     e_order_prepend_relative(box->apps, app, it->app);
   else
     e_order_append(box->apps, app);
   /* _save_order(box); */

   _item_new(box, app, 1, it);

   box->ng->dnd = 0;
   ngi_reposition(box->ng);
   ngi_input_extents_calc(box->ng);
}

/* ******************************* LAUNCHER ITEM ********************************** */

static void
_item_new(Ngi_Box *box, Efreet_Desktop *desktop, int instant, Ngi_Item_Launcher *after)
{
   Ngi_Item_Launcher *it;

   it = E_NEW(Ngi_Item_Launcher, 1);
   it->base.box = box;
   ngi_item_init_defaults((Ngi_Item*)it);

   it->base.cb_free = _item_cb_free;
   it->base.cb_mouse_down = _item_cb_mouse_down;
   it->base.cb_mouse_up = _item_cb_mouse_up;
   it->base.cb_drag_start = _item_cb_drag_start;

   efreet_desktop_ref(desktop);
   it->app = desktop;

   _item_fill(it);

   if (after)
     box->items = eina_list_prepend_relative(box->items, it, after);
   else
     box->items = eina_list_append(box->items, it);

   ngi_item_show((Ngi_Item*)it, instant);
}

static void
_item_cb_free(Ngi_Item *item)
{
   Ngi_Item_Launcher *it = (Ngi_Item_Launcher *) item;

   if (it->app)
     efreet_desktop_unref(it->app);

   if (it->o_icon)
     evas_object_del(it->o_icon);

   if (it->o_proxy)
     evas_object_del(it->o_proxy);

   E_FREE(it);
}

static void
_item_fill(Ngi_Item_Launcher *it)
{
   Evas_Object *o;
   Evas *e = it->base.box->ng->evas;

   if (it->o_icon)
     {
	edje_object_part_unswallow(it->base.obj, it->o_icon);
	evas_object_del(it->o_icon);
     }
   if (it->o_proxy)
     {
	edje_object_part_unswallow(it->base.obj, it->o_proxy);
	evas_object_del(it->o_proxy);
     }

   o = e_util_desktop_icon_add(it->app, 128, e);
   if (o)
     {
	edje_object_part_swallow(it->base.obj, "e.swallow.content", o);
	evas_object_show(o);
	it->o_icon = o;
	
	o = evas_object_image_filled_add(e);
	evas_object_image_fill_set(o, 0, 0, 1, 1);
	evas_object_image_source_set(o, it->base.obj);

	edje_object_part_swallow(it->base.over, "e.swallow.content", o);
	evas_object_show(o);
	it->o_proxy = o;
     }
   
   if (it->app->name && it->app->name[0])
     ngi_item_label_set((Ngi_Item*)it, it->app->name);
   else if (it->app->generic_name && it->app->generic_name[0])
     ngi_item_label_set((Ngi_Item*)it, it->app->generic_name);
}

static void
_cb_drag_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ngi_Box *box = data;

   ngi_bar_lock(box->ng, 0);
   ngi_thaw(box->ng);
}

static void
_cb_drag_finished(E_Drag *d, int dropped)
{
   efreet_desktop_unref((Efreet_Desktop*)d->data);
}

static void
_item_cb_drag_start(Ngi_Item *item)
{
   Ngi_Item_Launcher *it = (Ngi_Item_Launcher*) item;
   E_Drag *d;
   Evas_Object *o;
   Evas_Coord x, y, w, h, px, py;
   Ngi_Box *box = item->box;

   const char *drag_types[] = { "enlightenment/desktop" };

   if (item->box->cfg->launcher_lock_dnd)
     return;

   evas_object_geometry_get(it->o_icon, &x, &y, &w, &h);

   if (!ngi_config->use_composite)
     {
	x -= box->ng->win->rect.x;
	y -= box->ng->win->rect.y;
     }

   d = e_drag_new(box->ng->zone->container, x, y, drag_types, 1,
                  it->app, -1, NULL,
		  _cb_drag_finished);
   efreet_desktop_ref(it->app);

   o = e_util_desktop_icon_add(it->app, MIN(w, h), e_drag_evas_get(d));
   e_drag_object_set(d, o);
   e_drag_resize(d, w, h);
   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, _cb_drag_del, box);

   e_order_remove(item->box->apps, it->app);

   evas_object_hide(it->base.obj);
   ngi_item_remove(&(it->base));

   ecore_x_pointer_xy_get(box->ng->win->input, &px, &py);
   e_drag_start(d, px, py);

   box->ng->item_drag = NULL;

   ngi_bar_lock(box->ng, 1);
   ngi_thaw(box->ng);
}

static void
_menu_cb_lock_dnd(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Ngi_Box *box = (Ngi_Box *)data;

   if (box) box->cfg->launcher_lock_dnd = e_menu_item_toggle_get(mi);
}

static void
_menu_cb_edit_icon(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Ngi_Item_Launcher *it = data;

   if (it) e_desktop_edit(e_container_current_get(e_manager_current_get()), it->app);
}

static void
_menu_cb_configure_bar(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Ng *ng = (Ng *)data;

   if (ng) ngi_configure_module(ng->cfg);
}

static void
_menu_cb_configure_launcher(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Ngi_Box *box = (Ngi_Box *)data;
   char path[4096];

   if (!box)
     return;

   snprintf(path, sizeof(path), "%s/.e/e/applications/bar/%s/.order",
            e_user_homedir_get(), box->cfg->launcher_app_dir);

   e_configure_registry_call("internal/ibar_other",
                             e_container_current_get(e_manager_current_get()),
                             path);
}

static void
_menu_cb_menu_end(void *data, E_Menu *m)
{
   Ng *ng = (Ng *)data;

   e_object_del(E_OBJECT(m));
   ngi_thaw(ng);
}

static void
_item_cb_mouse_down(Ngi_Item *item, Ecore_Event_Mouse_Button *ev)
{
   Ngi_Item_Launcher *it = (Ngi_Item_Launcher *) item;
   Evas_Coord x, y, w, h;
   E_Menu *m;
   E_Menu_Item *mi;
   Ngi_Box *box = item->box;

   if (!(ev->buttons == 3))
     {
	item->mouse_down = 1;
	return;
     }

   if (!it->app)
     return;

   evas_object_geometry_get(item->obj, &x, &y, &w, &h);

   x += box->ng->win->popup->x + box->ng->zone->x;
   y += box->ng->win->popup->y + box->ng->zone->y;

   int dir = E_MENU_POP_DIRECTION_AUTO;

   switch(box->ng->cfg->orient)
     {
      case E_GADCON_ORIENT_TOP:
	 dir = E_MENU_POP_DIRECTION_DOWN;
	 y += h;
	 break;

      case E_GADCON_ORIENT_BOTTOM:
	 dir = E_MENU_POP_DIRECTION_UP;
	 break;

      case E_GADCON_ORIENT_LEFT:
	 dir = E_MENU_POP_DIRECTION_RIGHT;
	 x += w;
	 break;

      case E_GADCON_ORIENT_RIGHT:
	 dir = E_MENU_POP_DIRECTION_LEFT;
	 break;
     }

   m = e_menu_new();

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, D_("Configure Bar"));
   e_menu_item_callback_set(mi, _menu_cb_configure_bar, box->ng);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, D_("Lock Dragging"));
   e_menu_item_check_set(mi, 1);
   e_menu_item_toggle_set(mi, box->cfg->launcher_lock_dnd);
   e_menu_item_callback_set(mi, _menu_cb_lock_dnd, box);

   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, D_("Applications"));
   e_menu_item_callback_set(mi, _menu_cb_configure_launcher, box);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, D_("Edit Icon"));
   e_menu_item_callback_set(mi, _menu_cb_edit_icon, it);

   e_menu_post_deactivate_callback_set(m, _menu_cb_menu_end, box->ng);

   e_menu_activate_mouse(m, box->ng->zone, x, y, 1, 1,
			 dir, ev->timestamp);
}

static void
_item_cb_mouse_up(Ngi_Item *item, Ecore_Event_Mouse_Button *ev)
{
   Ngi_Item_Launcher *it = (Ngi_Item_Launcher *) item;

   if (!item->mouse_down)
     return;

   item->mouse_down = 0;

   if (ev->buttons == 1)
     {
	e_exec(e_util_zone_current_get(e_manager_current_get()), it->app, NULL, NULL, NULL);

	ngi_item_signal_emit((Ngi_Item*)it, "e,action,start");
     }
}

