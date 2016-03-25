#include "News.h"

#define DIALOG_CLASS "_e_mod_news_config_dialog_item_content"

struct _E_Config_Dialog_Data
{
   Evas_Object *ilist_feeds;
   Eina_List   *ilist_feeds_sel;
   Evas_Object *ilist_selected_feeds;
   Eina_List   *ilist_selected_feeds_sel;
   int          ilist_selected_feeds_inrefresh;
   Evas_Object *button_add;
   Evas_Object *button_rem;
   Evas_Object *button_up;
   Evas_Object *button_down;

   News_Item *ni;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata, News_Item *pi);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void         _cb_feed_up(void *data, void *data2);
static void         _cb_feed_down(void *data, void *data2);

static void         _cb_feed_add(void *data, void *data2);
static void         _cb_feed_remove(void *data, void *data2);

static void         _cb_feed_change(void *data, Evas_Object *obj);
static void         _cb_selected_feed_change(void *data, Evas_Object *obj);

/*
 * Public functions
 */

int
news_config_dialog_item_content_show(News_Item *ni)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("News Gadget Content Configuration"),
                             "News", DIALOG_CLASS,
                             news_theme_file_get(NEWS_THEME_CAT_ICON), 0, v, ni);

   if (news->config->feed.langs_notset)
     {
        news_config_dialog_langs_show();
        news->config->feed.langs_notset = 0;
        news_config_save();
     }

   return 1;
}

void
news_config_dialog_item_content_hide(News_Item *ni)
{
   e_object_del(E_OBJECT(ni->config_dialog_content));
   ni->config_dialog_content = NULL;
}

void
news_config_dialog_item_content_refresh_feeds(News_Item *ni)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l, *l2;
   Evas_Object *ilist;
   int pos;
   int iw, ih;
  
   if (!ni->config_dialog_content) return;
   cfdata = ni->config_dialog_content->cfdata;

   ilist = cfdata->ilist_feeds;
   e_widget_ilist_freeze(ilist);
   /* disable the callback until we finish to refresh the list */
   e_widget_on_change_hook_set(ilist, NULL, NULL);

   e_widget_ilist_clear(ilist);
   if (cfdata->button_add)
     e_widget_disabled_set(cfdata->button_add, 1);

   pos = -1;
   for(l=news->config->feed.categories; l; l=eina_list_next(l))
     {
        News_Feed_Category *cat;
        Evas_Object *iccat = NULL;

        cat = eina_list_data_get(l);

        if (!cat->feeds_visible)
          continue;

        if (cat->icon)
          {
             iccat = e_icon_add(evas_object_evas_get(ilist));
             e_icon_file_set(iccat, cat->icon);
          }

        e_widget_ilist_header_append(ilist, iccat, cat->name);
	pos++;

        for(l2=cat->feeds_visible; l2; l2=eina_list_next(l2))
          {
             Evas_Object *ic = NULL;
             News_Feed *f;
             char buf[1024];
             
             f = eina_list_data_get(l2);
             
             if (f->icon && f->icon[0])
               {
                  ic = e_icon_add(evas_object_evas_get(ilist));
                  e_icon_file_set(ic, f->icon);
               }
             
             snprintf(buf, sizeof(buf), "%s%s", (f->important) ? "[i] " : "", f->name);
             
             e_widget_ilist_append(ilist, ic, buf, NULL, f, NULL);
	     pos++;

	     if (eina_list_data_find(cfdata->ilist_feeds_sel, f))
               e_widget_ilist_multi_select(ilist, pos);
          }
     }
   e_widget_ilist_thaw(ilist);

   e_widget_size_min_get(ilist, &iw, &ih);
   if (iw < 200) iw = 200;
   e_widget_size_min_set(ilist, iw, 250);
   
   e_widget_ilist_go(ilist);
   _cb_feed_change(cfdata, NULL);
   /* restore the callback */
   e_widget_on_change_hook_set(ilist, _cb_feed_change, cfdata);
}

void
news_config_dialog_item_content_refresh_selected_feeds(News_Item *ni)
{
   E_Config_Dialog_Data *cfdata;
   Evas_Object *ilist;
   int pos, iw, ih;
  
   if (!ni->config_dialog_content) return;
   cfdata = ni->config_dialog_content->cfdata;

   ilist = cfdata->ilist_selected_feeds;
   e_widget_ilist_freeze(ilist);
   /* disable the callback until we finish to refresh the list */
   e_widget_on_change_hook_set(ilist, NULL, NULL);

   e_widget_ilist_clear(ilist);
   if (cfdata->button_rem)
     {
        e_widget_disabled_set(cfdata->button_rem, 1);
        e_widget_disabled_set(cfdata->button_up, 1);
        e_widget_disabled_set(cfdata->button_down, 1);
     }

   cfdata->ilist_selected_feeds_inrefresh = 1;
   pos = -1;
   NEWS_ITEM_FEEDS_FOREACH_BEG(ni);
   {
      Evas_Object *ic = NULL;
      char buf[1024];
        
        if (_feed->icon && _feed->icon[0])
          {
             ic = e_icon_add(evas_object_evas_get(ilist));
             e_icon_file_set(ic, _feed->icon);
          }

        snprintf(buf, sizeof(buf), "%s%s", (_feed->important) ? "[i] " : "", _feed->name);

        e_widget_ilist_append(ilist, ic, buf, NULL, _feed, NULL);
	pos++;

	if (eina_list_data_find(cfdata->ilist_selected_feeds_sel, _feed))
          e_widget_ilist_multi_select(ilist, pos);
   }
   NEWS_ITEM_FEEDS_FOREACH_END();
   cfdata->ilist_selected_feeds_inrefresh = 0;

   e_widget_size_min_get(ilist, &iw, &ih);
   if (iw < 200) iw = 200;
   e_widget_size_min_set(ilist, iw, 250);
   
   e_widget_ilist_go(ilist);
   e_widget_ilist_thaw(ilist);
   _cb_selected_feed_change(cfdata, NULL);
   /* restore the callback */
   e_widget_on_change_hook_set(ilist, _cb_selected_feed_change, cfdata);
}

/*
 * Private functions
 *
 */

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   News_Item *ni;

   ni = cfd->data;
   
   ni->config_dialog_content = cfd;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata, ni);
   cfd->cfdata = cfdata;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if (cfdata->ilist_feeds_sel)
     eina_list_free(cfdata->ilist_feeds_sel);
   if (cfdata->ilist_selected_feeds_sel)
     eina_list_free(cfdata->ilist_selected_feeds_sel);

   cfdata->ni->config_dialog_content = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata, News_Item *ni)
{
   News_Config_Item *nic;

   nic = ni->config;
   cfdata->ni = ni;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o;
   Evas_Object *of, *ob;

   o = e_widget_list_add(evas, 0, 1);

   of = e_widget_frametable_add(evas, D_("Available Feeds"), 0);

   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_multi_select_set(ob, 1);
   cfdata->ilist_feeds = ob;
   news_config_dialog_item_content_refresh_feeds(cfdata->ni);
   e_widget_on_change_hook_set(ob, _cb_feed_change, cfdata);
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   
   ob = e_widget_button_add(evas, D_("Add this Feed"), NULL, _cb_feed_add, cfdata, NULL);
   e_widget_disabled_set(ob, 1);
   cfdata->button_add = ob;
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 0);

   e_widget_list_object_append(o, of, 1, 1, 0.0);

   of = e_widget_frametable_add(evas, D_("Selected Feeds"), 0);

   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_multi_select_set(ob, 1);
   cfdata->ilist_selected_feeds = ob;
   news_config_dialog_item_content_refresh_selected_feeds(cfdata->ni);
   e_widget_on_change_hook_set(ob, _cb_selected_feed_change, cfdata);
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 4, 1, 1, 1, 1);

   ob = e_widget_button_add(evas, D_("Move Up"), "go-up", _cb_feed_up, cfdata, NULL);
   e_widget_disabled_set(ob, 1);
   cfdata->button_up = ob;
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 0, 0, 0, 0);
   ob = e_widget_button_add(evas, D_("Move Down"), "go-down", _cb_feed_down, cfdata, NULL);
   e_widget_disabled_set(ob, 1);
   cfdata->button_down = ob;
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 0, 0, 0, 0);

   ob = e_widget_button_add(evas, D_("Remove this Feed"), NULL, _cb_feed_remove, cfdata, NULL);
   cfdata->button_rem = ob;
   e_widget_disabled_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 0, 5, 1, 4, 1, 1, 1, 0);

   e_widget_list_object_append(o, of, 1, 1, 1.0);

   e_dialog_resizable_set(cfd->dia, 1);

   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   news_config_save();
   return 1;
}

static void
_cb_feed_up(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed *f;
   News_Item *ni;
   News_Feed_Ref *ref;
   Eina_List *sel, *l, *lf;

   cfdata = data;
   ni = cfdata->ni;

   for (sel = cfdata->ilist_selected_feeds_sel; sel; sel=eina_list_next(sel))
     {
        f = sel->data;
        ref = news_feed_ref_find(f, ni);
        if (!ref) return;

        l = eina_list_data_find_list(ni->config->feed_refs, ref);
        lf = eina_list_prev(l);
        if (!lf) return;

        ni->config->feed_refs = eina_list_remove_list(ni->config->feed_refs, l);
        ni->config->feed_refs = eina_list_prepend_relative_list(ni->config->feed_refs,
                                                                ref, lf);
     }

   news_item_refresh(ni, 1, 0, 0);
   news_config_dialog_item_content_refresh_selected_feeds(ni);
}

static void
_cb_feed_down(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed *f;
   News_Item *ni;
   News_Feed_Ref *ref;
   Eina_List *sel, *l, *lf;

   cfdata = data;
   ni = cfdata->ni;

   for (sel=eina_list_last(cfdata->ilist_selected_feeds_sel); sel; sel=eina_list_prev(sel))
     {
        f = sel->data;
        ref = news_feed_ref_find(f, ni);
        if (!ref) return;

        l = eina_list_data_find_list(ni->config->feed_refs, ref);
        lf = eina_list_next(l);
        if (!lf) return;
  
        ni->config->feed_refs = eina_list_remove_list(ni->config->feed_refs, l);
        ni->config->feed_refs = eina_list_append_relative_list(ni->config->feed_refs,
                                                               ref, lf);
     }

   news_item_refresh(ni, 1, 0, 0);
   news_config_dialog_item_content_refresh_selected_feeds(ni);
}

static void
_cb_feed_add(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Item *ni;
   News_Feed *f;
   Eina_List *l;
  
   cfdata = data;
   ni = cfdata->ni;

   for (l=cfdata->ilist_feeds_sel; l; l=eina_list_next(l))
     {
        f = l->data;
        if (f->item) continue;

        if (news_feed_attach(f, NULL, ni))
          news_feed_obj_refresh(f, 1, 1);

        /* dont reselect this feed */
        cfdata->ilist_feeds_sel = eina_list_remove(cfdata->ilist_feeds_sel, f);
     }

   news_item_refresh(ni, 1, 0, 0);
   news_feed_list_ui_refresh();
   news_viewer_refresh(ni->viewer);

   news_config_save();
}

static void
_cb_feed_remove(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Item *ni;
   News_Feed *f;
   Eina_List *l;
  
   cfdata = data;
   ni = cfdata->ni;

   for (l=cfdata->ilist_selected_feeds_sel; l; l=eina_list_next(l))
     {
        f = l->data;
        news_feed_detach(f, 1);
     }

   news_item_refresh(ni, 1, 0, 0);
   news_config_dialog_item_content_refresh_selected_feeds(ni);
   news_viewer_refresh(ni->viewer);

   news_config_save();
}

static void
_cb_feed_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;
   E_Ilist_Item *item;
   News_Feed *feed;
   Eina_List *sel, *items, *l;
   int i;

   cfdata = data;

   if (cfdata->ilist_feeds_sel) eina_list_free(cfdata->ilist_feeds_sel);
   sel = NULL;
   items = e_widget_ilist_items_get(cfdata->ilist_feeds);
   for (l=items, i=0; l; l=eina_list_next(l), i++)
     {
        item = l->data;
        if (item->header) continue;
        if (!item->selected) continue;
        feed = e_widget_ilist_nth_data_get(cfdata->ilist_feeds, i);
        sel = eina_list_append(sel, feed);
     }
   cfdata->ilist_feeds_sel = sel;

   if (e_widget_ilist_selected_get(cfdata->ilist_feeds) >= 0)
     {
        if (cfdata->button_add)
          e_widget_disabled_set(cfdata->button_add, 0);
     }
   else
     {
        if (cfdata->button_add)
          e_widget_disabled_set(cfdata->button_add, 1);
     }
}

static void
_cb_selected_feed_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;
   E_Ilist_Item *item;
   News_Feed *feed;
   Eina_List *sel, *items, *l;
   int i;

   cfdata = data;
   if (cfdata->ilist_selected_feeds_inrefresh) return;

   if (cfdata->ilist_selected_feeds_sel) eina_list_free(cfdata->ilist_selected_feeds_sel);
   sel = NULL;
   items = e_widget_ilist_items_get(cfdata->ilist_selected_feeds);
   for (l=items, i=0; l; l=eina_list_next(l), i++)
     {
        item = l->data;
        if (item->header) continue;
        if (!item->selected) continue;
        feed = e_widget_ilist_nth_data_get(cfdata->ilist_selected_feeds, i);
        sel = eina_list_append(sel, feed);
     }
   cfdata->ilist_selected_feeds_sel = sel;

   if (e_widget_ilist_selected_get(cfdata->ilist_selected_feeds) >= 0)
     {
        if (cfdata->button_rem)
          {
             e_widget_disabled_set(cfdata->button_rem, 0);
             e_widget_disabled_set(cfdata->button_up, 0);
             e_widget_disabled_set(cfdata->button_down, 0);
          }
     }
   else
     {
        if (cfdata->button_rem)
          {
             e_widget_disabled_set(cfdata->button_rem, 1);
             e_widget_disabled_set(cfdata->button_up, 1);
             e_widget_disabled_set(cfdata->button_down, 1);
          }
     }
}
