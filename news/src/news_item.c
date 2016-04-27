#include "News.h"

static void       _item_refresh_mode_one(News_Item *ni, int changed_order, int changed_state);
static void       _item_refresh_mode_feed(News_Item *ni, int important_only, int unread_only, int changed_order, int changed_content, int changed_state);
static void       _cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void       _cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void       _cb_item_open(void *data, Evas_Object *obj, const char *emission, const char *source);


/*
 * Public functions
 */

News_Item *
news_item_new(E_Gadcon_Client *gcc, Evas_Object *obj, const char *id)
{
   News_Item *ni;
   News_Config_Item *nic;
   Evas_Object *view;
   Eina_List *l;

   ni = E_NEW(News_Item, 1);

   news_theme_edje_set(obj, NEWS_THEME_ITEM);

   nic = news_config_item_add(id);
   ni->gcc = gcc;
   ni->obj = obj;
   ni->config = nic;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
				  _cb_mouse_down, ni);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_OUT,
				  _cb_mouse_out, ni);

   /* attach feeds */
   for (l=nic->feed_refs; l; l=eina_list_next(l))
     {
        News_Feed_Ref *ref;

        ref = eina_list_data_get(l);
        news_feed_attach(NULL, ref, ni);
     }
   news_feed_list_ui_refresh();

   /* main view */
   view = e_box_add(gcc->gadcon->evas);
   e_box_homogenous_set(view, 1);
   e_box_orientation_set(view, 1);
   edje_object_part_swallow(ni->obj, "view", view);
   evas_object_show(view);
   ni->view.box = view;

   news_item_refresh(ni, 1, 1, 1);

   return ni;
}

void
news_item_free(News_Item *ni)
{
   DITEM(("Item del"));

   NEWS_ITEM_FEEDS_FOREACH_BEG(ni);
   news_feed_detach(_feed, 0);
   NEWS_ITEM_FEEDS_FOREACH_END();

   news_feed_list_ui_refresh();
   
   if (ni->config_dialog) news_config_dialog_item_hide(ni);
   if (ni->config_dialog_content) news_config_dialog_item_content_hide(ni);
   if (ni->menu) news_menu_item_hide(ni);
   if (ni->menu_browser) news_menu_browser_hide(ni);

   if (ni->view.box) evas_object_del(ni->view.box);
   if (ni->view.obj_mode_one) evas_object_del(ni->view.obj_mode_one);

   if (ni->viewer) news_viewer_destroy(ni->viewer);

   if (ni->obj) evas_object_del(ni->obj);

   free(ni);
}

/*
 * need to be called when
 *  view orientation
 *  view item size
 *  a feed has been added OR remove to/from an item
 * has changed
 */
void
news_item_refresh(News_Item *ni, int changed_order, int changed_content, int changed_state)
{
   Evas_Object *box;
   News_Item_View_Mode mode;

   box = ni->view.box;
   e_box_freeze(box);

   if (changed_order)
     {
        Evas_Object *o;

        while ((o = e_box_pack_object_first(box)))
          {
             e_box_unpack(o);
             evas_object_hide(o);
          }
     }

   if (!eina_list_count(ni->config->feed_refs))
     mode = NEWS_ITEM_VIEW_MODE_ONE;
   else
     mode = ni->config->view_mode;

   switch (mode)
     {
     case NEWS_ITEM_VIEW_MODE_ONE:
        _item_refresh_mode_one(ni, changed_order, changed_state);
        break;
     case NEWS_ITEM_VIEW_MODE_FEED:
        _item_refresh_mode_feed(ni, 0, 0, changed_order, changed_content, changed_state);
        break;
     case NEWS_ITEM_VIEW_MODE_FEED_UNREAD:
        _item_refresh_mode_feed(ni, 0, 1, changed_order, changed_content, changed_state);
     case NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT:
        _item_refresh_mode_feed(ni, 1, 0, changed_order, changed_content, changed_state);
        break;
     case NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT_UNREAD:
        _item_refresh_mode_feed(ni, 1, 1, changed_order, changed_content, changed_state);
        break;
     }

   e_box_thaw(box);

   /* resize the gadcon */
   if (changed_order && ni->gcc->client_class)
     ni->gcc->client_class->func.orient(ni->gcc, ni->gcc->gadcon->orient);
}

void
news_item_orient_set(News_Item *ni, int horizontal)
{
   e_box_orientation_set(ni->view.box, horizontal);

   e_box_align_set(ni->view.box, 0.5, 0.5);
}

void
news_item_loadingstate_refresh(News_Item *ni)
{
   // TODO
}

void
news_item_unread_count_change(News_Item *ni, int nb)
{
   int was_empty = 0;

   if (!nb) return;
   if (!ni->unread_count)
     was_empty = 1;
   ni->unread_count += nb;
   if (was_empty || !ni->unread_count)
     {
        if (ni->config->view_mode == NEWS_ITEM_VIEW_MODE_FEED_UNREAD)
          news_item_refresh(ni, 1, 0, 1);
        else
          news_item_refresh(ni, 0, 0, 1);

        if (ni->viewer)
          news_viewer_refresh(ni->viewer);
     }

   DFEED(("UNREAD count = %d", ni->unread_count));
}

/*
 * Private functions
 */

static void
_item_refresh_mode_one(News_Item *ni, int changed_order, int changed_state)
{
   Evas_Object *box;
   Evas_Object *obj;

   box = ni->view.box;
   obj = ni->view.obj_mode_one;

   /* create obj */

   if (!obj)
     {
        obj = edje_object_add(ni->gcc->gadcon->evas);
        news_theme_edje_set(obj, NEWS_THEME_FEEDONE);
        edje_object_signal_callback_add(obj, "e,action,open", "e",
                                        _cb_item_open, ni);
     }

   if (!ni->view.obj_mode_one || changed_order)
     {
        e_box_pack_end(box, obj);
        e_box_pack_options_set(obj,
                               1, 1, /* fill */
                               1, 1, /* expand */
                               0, 0, /* align */
                               0, 0,
                               -1, -1
                               );
        evas_object_show(obj);
     }

   /* state */

   if (!ni->view.obj_mode_one || changed_state)
     {
        if (ni->unread_count)
          edje_object_signal_emit(obj, "e,state,new,set", "e");
        else
          edje_object_signal_emit(obj, "e,state,new,unset", "e");
     }

   ni->view.obj_mode_one = obj;
}

static void
_item_refresh_mode_feed(News_Item *ni, int important_only, int unread_only, int changed_order, int changed_content, int changed_state)
{
   Evas_Object *box;
   Evas_Object *obj;

   if (!eina_list_count(ni->config->feed_refs)) return;

   box = ni->view.box;

   NEWS_ITEM_FEEDS_FOREACH_BEG_LIST(ni->config->feed_refs);
   {
      obj = _feed->obj;

      if (important_only &&
          (!_feed->important))
        continue;
      if (unread_only &&
          (!_feed->doc || !_feed->doc->unread_count))
        continue;
      
      DITEM(("refresh : feed %s", _feed->name));
      
      if (!_feed->obj || changed_content || changed_state)
        {
           news_feed_obj_refresh(_feed, changed_content, changed_state);
           obj = _feed->obj;
        }
      
      if (!_feed->obj || changed_order)
        {
           /* insert in the box */
           
           e_box_pack_end(box, obj);
           e_box_pack_options_set(obj,
                                  1, 1, /* fill */
                                  1, 1, /* expand */
                                  0, 0, /* align */
                                  0, 0,
                                  -1, -1
                                  );
           evas_object_show(obj);
        }
      
      _feed->obj = obj;
   }
   NEWS_ITEM_FEEDS_FOREACH_END();
}

static void
_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   News_Item *ni;
   Evas_Event_Mouse_Down *ev;

   ni = data;
   ev = event_info;

   DITEM(("Mouse down %d", ev->button));

   switch(ev->button)
     {
        int cx, cy, cw, ch;

     case 3:
        if (ni->menu) break;
        news_menu_item_show(ni);
        e_gadcon_canvas_zone_geometry_get(ni->gcc->gadcon,
                                          &cx, &cy, &cw, &ch);
        e_menu_activate_mouse(ni->menu,
                              e_util_zone_current_get(e_manager_current_get()),
                              cx + ev->output.x, cy + ev->output.y, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(ni->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
        break;
     }
}

static void
_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   News_Item *ni;
   Evas_Event_Mouse_Out *ev;

   ni = data;
   ev = event_info;

   DITEM(("Mouse out"));

}

static void
_cb_item_open(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   News_Item *ni;
   E_Manager *man;
   E_Zone *zone;
   int cx, cy;

   ni = data;

   switch (ni->config->openmethod)
     {
     case NEWS_ITEM_OPENMETHOD_VIEWER:
        if (!ni->viewer)
          news_viewer_create(ni);
        else
          news_viewer_destroy(ni->viewer);
        break;
     case NEWS_ITEM_OPENMETHOD_BROWSER:
        news_menu_browser_show(ni);
        man = e_manager_current_get();
        zone = e_util_zone_current_get(man);
        ecore_x_pointer_xy_get(man->root, &cx, &cy);
        e_menu_activate_mouse(ni->menu_browser, zone,
                              cx, cy, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, 
                              ecore_x_current_time_get());
        break;
     }
}
