#include "News.h"

#define DIALOG_CLASS "_e_modules_news_feeds_config_dialog"

struct _E_Config_Dialog_Data
{
   Evas_Object *ilist_feeds;
   Evas_Object *ilist_categories;
   Evas_Object *textblock_feed_infos;
   News_Feed *selected_feed;
   News_Feed_Category *selected_category;
   Evas_Object *button_feed_del;
   Evas_Object *button_feed_conf;
   Evas_Object *button_feed_up;
   Evas_Object *button_feed_down;
   Evas_Object *button_cat_del;
   Evas_Object *button_cat_conf;
   Evas_Object *button_cat_up;
   Evas_Object *button_cat_down;
   Evas_Object *button_langs;

   E_Confirm_Dialog *cd;

   int langs_all;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void         _buttons_feed_update(E_Config_Dialog_Data *cfdata);
static void         _buttons_category_update(E_Config_Dialog_Data *cfdata);

static void         _cb_feed_up(void *data, void *data2);
static void         _cb_feed_down(void *data, void *data2);

static void         _cb_feed_list(void *data);
static void         _cb_feed_add(void *data, void *data2);
static void         _cb_feed_del(void *data, void *data2);
static void         _cb_feed_config(void *data, void *data2);

static void         _cb_category_up(void *data, void *data2);
static void         _cb_category_down(void *data, void *data2);

static void         _cb_category_list(void *data);
static void         _cb_category_add(void *data, void *data2);
static void         _cb_category_del(void *data, void *data2);
static void         _cb_category_config(void *data, void *data2);

static void         _cb_langs_all_change(void *data, Evas_Object *obj);
static void         _cb_langs_config(void *data, void *data2);

static void         _cb_empty(void *data, void *data2);
static void         _cb_empty_yes(void *data);
static void         _cb_reset(void *data, void *data2);
static void         _cb_reset_yes(void *data);
static void         _cb_confirm_dialog_destroy(void *data);

/*
 * Public functions
 */

int
news_config_dialog_feeds_show(void)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("News", DIALOG_CLASS))
     return 0;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("News Feeds Configuration"),
                             "News", DIALOG_CLASS,
                             news_theme_file_get(NEWS_THEME_CAT_ICON), 0, v, NULL);

   if (news->config->feed.langs_notset)
     {
        news_config_dialog_langs_show();
        news->config->feed.langs_notset = 0;
        news_config_save();
     }

   return 1;
}

void
news_config_dialog_feeds_hide(void)
{
   e_object_del(E_OBJECT(news->config_dialog_feeds));
   news->config_dialog_feeds = NULL;
}

void
news_config_dialog_feeds_refresh_feeds(void)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l, *lcat;
   Evas_Object *ilist;
   int pos, pos_to_select;

   if (!news->config_dialog_feeds) return;
   cfdata = news->config_dialog_feeds->cfdata;

   ilist = cfdata->ilist_feeds;
   e_widget_ilist_freeze(ilist);

   e_widget_ilist_clear(ilist);
   _buttons_feed_update(cfdata);

   pos = -1;
   pos_to_select = -1;
   for (lcat=news->config->feed.categories; lcat; lcat=eina_list_next(lcat))
     {
        News_Feed_Category *cat;
        Evas_Object *iccat = NULL;

        cat = eina_list_data_get(lcat);

        if (!cat->feeds_visible)
          continue;

        if (cat->icon)
          {
             iccat = e_icon_add(evas_object_evas_get(ilist));
             e_icon_file_set(iccat, cat->icon);
          }

        e_widget_ilist_header_append(ilist, iccat, cat->name);
	pos++;

        for (l=cat->feeds_visible; l; l=eina_list_next(l))
          {
             Evas_Object *ic = NULL;
             News_Feed *f;
             char buf[1024];
	   
             f = eina_list_data_get(l);
	
             if (f->icon && f->icon[0])
               {
                  ic = e_icon_add(evas_object_evas_get(ilist));
                  e_icon_file_set(ic, f->icon);
               }
	   
             snprintf(buf, sizeof(buf), "%s%s", (f->important) ? "[i] " : "", f->name);
	   
             e_widget_ilist_append(ilist, ic, buf, _cb_feed_list, f, NULL);
	     pos++;

	     if (cfdata->selected_feed == f)
               pos_to_select = pos;
          }
     }
   e_widget_ilist_go(ilist);
   e_widget_ilist_thaw(ilist);

   /* select a feed */
   if (pos_to_select != -1)
     {
        e_widget_ilist_selected_set(ilist, pos_to_select);
        _cb_feed_list(cfdata->selected_feed);
     }

   /* ilist size */
   if (pos == -1)
     e_widget_size_min_set(ilist, 165, 120);
   else
     {
        int wmw, wmh;
        e_widget_size_min_get(ilist, &wmw, &wmh);
        e_widget_size_min_set(ilist, wmw, 180);
     }
}

void
news_config_dialog_feeds_refresh_categories(void)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l;
   Evas_Object *ilist;
   int pos, pos_to_select;

   if (!news->config_dialog_feeds) return;
   cfdata = news->config_dialog_feeds->cfdata;

   ilist = cfdata->ilist_categories;
   e_widget_ilist_freeze(ilist);

   e_widget_ilist_clear(ilist);
   _buttons_category_update(cfdata);

   pos = -1;
   pos_to_select = -1;
   for (l=news->config->feed.categories; l; l=eina_list_next(l))
     {
        Evas_Object *ic = NULL;
        News_Feed_Category *fc;
        char buf[1024];
        
        fc = eina_list_data_get(l);

        if (fc->icon && fc->icon[0])
          {
             ic = e_icon_add(evas_object_evas_get(ilist));
             e_icon_file_set(ic, fc->icon);
          }

        snprintf(buf, sizeof(buf), "%s", fc->name);

        e_widget_ilist_append(ilist, ic, buf, _cb_category_list, fc, NULL);
	pos++;

	if (cfdata->selected_category == fc)
          pos_to_select = pos;
     }
   e_widget_ilist_go(ilist);
   e_widget_ilist_thaw(ilist);

   if (pos_to_select != -1)
     e_widget_ilist_selected_set(ilist, pos_to_select);

   if (pos == -1)
     e_widget_size_min_set(ilist, 165, 120);
   else
     {
        int wmw, wmh;
        e_widget_size_min_get(ilist, &wmw, &wmh);
        e_widget_size_min_set(ilist, wmw, 120);
     }
}

/*
 * Private functions
 *
 */

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;

   news->config_dialog_feeds = cfd;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   cfd->cfdata = cfdata;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if (cfdata->cd) e_object_del(E_OBJECT(cfdata->cd));

   news->config_dialog_feeds = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   News_Config *c;

   c = news->config;

   cfdata->langs_all = c->feed.langs_all;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *o2, *o3, *o4;
   Evas_Object *of, *ob;
   int wmw, wmh;

   o = e_widget_list_add(evas, 1, 1);

   o2 = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_frametable_add(evas, D_("Categories"), 0);

   cfdata->selected_category = NULL;
   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_selector_set(ob, 1);
   cfdata->ilist_categories = ob;
   news_config_dialog_feeds_refresh_categories();
   e_widget_frametable_object_append(of, ob, 0, 0, 6, 1, 1, 1, 1, 1);

   if (!news->config->feed.sort_name)
     {
        ob = e_widget_button_add(evas, D_("Move Up"), "go-up", _cb_category_up, cfdata, NULL);
        cfdata->button_cat_up = ob;
        e_widget_frametable_object_append(of, ob, 0, 1, 3, 1, 1, 0, 1, 0);
        ob = e_widget_button_add(evas, D_("Move Down"), "go-down", _cb_category_down, cfdata, NULL);
        cfdata->button_cat_down = ob;
        e_widget_frametable_object_append(of, ob, 3, 1, 3, 1, 1, 0, 1, 0);
     }
   else
     {
        cfdata->button_cat_up = NULL;
        cfdata->button_cat_down = NULL;
     }

   ob = e_widget_button_add(evas, D_("Add"), NULL, _cb_category_add, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 0, 2, 2, 1, 1, 0, 1, 0);
   ob = e_widget_button_add(evas, D_("Delete"), NULL, _cb_category_del, cfdata, NULL);
   cfdata->button_cat_del = ob;
   e_widget_frametable_object_append(of, ob, 2, 2, 2, 1, 1, 0, 1, 0);
   ob = e_widget_button_add(evas, D_("Configure"), NULL, _cb_category_config, cfdata, NULL);
   cfdata->button_cat_conf = ob;
   _buttons_category_update(cfdata);
   e_widget_frametable_object_append(of, ob, 4, 2, 2, 1, 1, 0, 1, 0);

   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   ob = e_widget_label_add(evas, "");
   e_widget_list_object_append(o2, ob, 0, 0, 0.5);

   o3 = e_widget_list_add(evas, 0, 1);

   of = e_widget_framelist_add(evas, D_("Languages"), 1);

   ob = e_widget_check_add(evas, D_("All"), &(cfdata->langs_all));
   e_widget_on_change_hook_set(ob, _cb_langs_all_change, cfdata);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_button_add(evas, D_("Select"), NULL, _cb_langs_config, cfdata, NULL);
   if (cfdata->langs_all)
     e_widget_disabled_set(ob, 1);
   cfdata->button_langs = ob;
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o3, of, 1, 0, 0.5);

   o4 = e_widget_list_add(evas, 0, 0);

   ob = e_widget_button_add(evas, D_("Empty the lists"), "enlightenment", _cb_empty, cfdata, NULL);
   e_widget_list_object_append(o4, ob, 1, 0, 0.5);

   ob = e_widget_button_add(evas, D_("Restore default lists"), "enlightenment", _cb_reset, cfdata, NULL);
   e_widget_list_object_append(o4, ob, 1, 0, 0.5);

   e_widget_list_object_append(o3, o4, 1, 1, 0.5);

   e_widget_list_object_append(o2, o3, 0, 0, 0.5);

   e_widget_list_object_append(o, o2, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, D_("Feeds"), 0);

   ob = e_widget_textblock_add(evas);
   cfdata->textblock_feed_infos = ob;
   e_widget_size_min_get(ob, &wmw, &wmh);
   e_widget_size_min_set(ob, wmw, 40);
   e_widget_frametable_object_append(of, ob, 0, 0, 6, 1, 1, 1, 0, 0);

   cfdata->selected_feed = NULL;
   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_selector_set(ob, 1);
   cfdata->ilist_feeds = ob;
   news_config_dialog_feeds_refresh_feeds();
   e_widget_frametable_object_append(of, ob, 0, 1, 6, 1, 1, 1, 1, 1);

   if (!news->config->feed.sort_name)
     {
        ob = e_widget_button_add(evas, D_("Move Up"), "go-up", _cb_feed_up, cfdata, NULL);
        cfdata->button_feed_up = ob;
        e_widget_frametable_object_append(of, ob, 0, 2, 3, 1, 1, 0, 1, 0);
        ob = e_widget_button_add(evas, D_("Move Down"), "go-down", _cb_feed_down, cfdata, NULL);
        cfdata->button_feed_down = ob;
        e_widget_frametable_object_append(of, ob, 3, 2, 3, 1, 1, 0, 1, 0);
     }
   else
     {
        cfdata->button_feed_up = NULL;
        cfdata->button_feed_down = NULL;
     }

   ob = e_widget_button_add(evas, D_("Add"), NULL, _cb_feed_add, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 0, 3, 2, 1, 1, 0, 1, 0);
   ob = e_widget_button_add(evas, D_("Delete"), NULL, _cb_feed_del, cfdata, NULL);
   cfdata->button_feed_del = ob;
   e_widget_frametable_object_append(of, ob, 2, 3, 2, 1, 1, 0, 1, 0);
   ob = e_widget_button_add(evas, D_("Configure"), NULL, _cb_feed_config, cfdata, NULL);
   cfdata->button_feed_conf = ob;
   _buttons_feed_update(cfdata);
   e_widget_frametable_object_append(of, ob, 4, 3, 2, 1, 1, 0, 1, 0);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

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
_buttons_feed_update(E_Config_Dialog_Data *cfdata)
{
   if (cfdata->selected_feed)
     {
        e_widget_disabled_set(cfdata->button_feed_del, 0);
        e_widget_disabled_set(cfdata->button_feed_conf, 0);
        e_widget_disabled_set(cfdata->button_feed_up, 0);
        e_widget_disabled_set(cfdata->button_feed_down, 0);
     }
   else
     {
        e_widget_disabled_set(cfdata->button_feed_del, 1);
        e_widget_disabled_set(cfdata->button_feed_conf, 1);
        e_widget_disabled_set(cfdata->button_feed_up, 1);
        e_widget_disabled_set(cfdata->button_feed_down, 1);
     }
}

static void
_buttons_category_update(E_Config_Dialog_Data *cfdata)
{
   if (cfdata->selected_category)
     {
        e_widget_disabled_set(cfdata->button_cat_del, 0);
        e_widget_disabled_set(cfdata->button_cat_conf, 0);
        e_widget_disabled_set(cfdata->button_cat_up, 0);
        e_widget_disabled_set(cfdata->button_cat_down, 0);
     }
   else
     {
        e_widget_disabled_set(cfdata->button_cat_del, 1);
        e_widget_disabled_set(cfdata->button_cat_conf, 1);
        e_widget_disabled_set(cfdata->button_cat_up, 1);
        e_widget_disabled_set(cfdata->button_cat_down, 1);
     }
}

static void
_cb_feed_up(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Category *cat;
   News_Feed *f, *f_prev;
   Eina_List *l, *l_prev;

   if (news->config->feed.sort_name) return;

   cfdata = data;

   f = cfdata->selected_feed;
   if (!f) return;
   cat = f->category;

   l = eina_list_data_find_list(cat->feeds_visible, f);
   l_prev = eina_list_prev(l);
   f_prev = eina_list_data_get(l_prev);
   if (!l_prev) return;
  
   cat->feeds = eina_list_remove(cat->feeds, f);
   cat->feeds = eina_list_prepend_relative(cat->feeds, f, f_prev);

   news_feed_lists_refresh(0);
}

static void
_cb_feed_down(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Category *cat;
   News_Feed *f, *f_next;
   Eina_List *l, *l_next;

   if (news->config->feed.sort_name) return;

   cfdata = data;

   f = cfdata->selected_feed;
   if (!f) return;
   cat = f->category;

   l = eina_list_data_find_list(cat->feeds, f);
   l_next = eina_list_next(l);
   f_next = eina_list_data_get(l_next);
   if (!l_next) return;
  
   cat->feeds = eina_list_remove(cat->feeds, f);
   cat->feeds = eina_list_append_relative(cat->feeds, f, f_next);

   news_feed_lists_refresh(0);
}

static void
_cb_feed_list(void *data)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed *f;
   char buf[4096];
   
   f = data;
   cfdata = news->config_dialog_feeds->cfdata;

   cfdata->selected_feed = f;
   _buttons_feed_update(cfdata);

   snprintf(buf, sizeof(buf),
            "%s",
            f->description ? f->description : "No description for the selected feed");

   e_widget_textblock_markup_set(cfdata->textblock_feed_infos, buf);
}

static void
_cb_feed_add(void *data, void *data2)
{
   if (news->config_dialog_feed_new) return;

   news_config_dialog_feed_show(NULL);
}

static void
_cb_feed_del(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Category *cat;
   News_Feed *f;
   News_Viewer *nv = NULL;
   int pos;

   cfdata = data;
   f = cfdata->selected_feed;
   if (!f) return;
   if (f->item && f->item->viewer)
     nv = f->item->viewer;

   cat = f->category;
   cat->feeds = eina_list_remove(cat->feeds, f);
   news_feed_free(f);
   cfdata->selected_feed = NULL;

   pos = e_widget_ilist_selected_get(cfdata->ilist_feeds);
   news_feed_lists_refresh(0);
   /* trick to call the callback in selected_set */
   e_widget_ilist_selector_set(cfdata->ilist_feeds, 0);
   e_widget_ilist_selected_set(cfdata->ilist_feeds, pos);
   e_widget_ilist_selector_set(cfdata->ilist_feeds, 1);
   if (nv) news_viewer_refresh(nv);

   news_config_save();
}

static void
_cb_feed_config(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed *f;

   cfdata = data;
   f = cfdata->selected_feed;
   if (!f) return;
   if (f->config_dialog) return;

   news_config_dialog_feed_show(f);
}

static void
_cb_category_up(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Category *cat;
   Eina_List *l, *l_prev;

   if (news->config->feed.sort_name) return;

   cfdata = data;

   cat = cfdata->selected_category;
   if (!cat) return;
   l = eina_list_data_find_list(news->config->feed.categories, cat);
   l_prev = eina_list_prev(l);
   if (!l_prev) return;
  
   news->config->feed.categories =
      eina_list_remove_list(news->config->feed.categories, l);
   news->config->feed.categories =
      eina_list_prepend_relative_list(news->config->feed.categories, cat, l_prev);

   news_feed_lists_refresh(0);
}

static void
_cb_category_down(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Category *cat;
   Eina_List *l, *l_next;

   if (news->config->feed.sort_name) return;

   cfdata = data;

   cat = cfdata->selected_category;
   if (!cat) return;
   l = eina_list_data_find_list(news->config->feed.categories, cat);
   l_next = eina_list_next(l);
   if (!l_next) return;
  
   news->config->feed.categories =
      eina_list_remove_list(news->config->feed.categories, l);
   news->config->feed.categories =
      eina_list_append_relative_list(news->config->feed.categories, cat, l_next);

   news_feed_lists_refresh(0);
}

static void
_cb_category_list(void *data)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Category *c;
   const char *label;
   int pos;

   c = data;
   cfdata = news->config_dialog_feeds->cfdata;

   if (cfdata->selected_category != c)
     {
        cfdata->selected_category = c;
        _buttons_category_update(cfdata);

        /* select the first feed in this category */
        if (c->feeds_visible)
          {
             pos = 0;
             while ((label = e_widget_ilist_nth_label_get(cfdata->ilist_feeds, pos)))
               {
                  if (e_widget_ilist_nth_is_header(cfdata->ilist_feeds, pos))
                    {
                       if (!strcmp(label, c->name))
                         {
                            /* trick to call the callback in selected_set */
                            e_widget_ilist_selector_set(cfdata->ilist_feeds, 0);
                            e_widget_ilist_selected_set(cfdata->ilist_feeds, pos+1);
                            e_widget_ilist_selector_set(cfdata->ilist_feeds, 1);
                            break;
                         }
                    }
                  pos++;
               }
          }
     }
}

static void
_cb_category_add(void *data, void *data2)
{
   if (news->config_dialog_category_new) return;

   news_config_dialog_category_show(NULL);
}

static void
_cb_category_del(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Category *c;
   int pos;

   cfdata = data;
   c = cfdata->selected_category;
   if (!c) return;
   if (c->feeds && eina_list_count(c->feeds))
     {
        news_util_message_error_show(D_("There are <hilight>feeds</hilight> in this category.<br>"
                                        "You have to <hilight>remove them first</hilight>"));
        return;
     }

   news_feed_category_free(c);
   news->config->feed.categories = eina_list_remove(news->config->feed.categories, c);
   cfdata->selected_category = NULL;

   pos = e_widget_ilist_selected_get(cfdata->ilist_categories);
   news_feed_lists_refresh(1);
   /* trick to call the callback in selected_set */
   e_widget_ilist_selector_set(cfdata->ilist_categories, 0);
   e_widget_ilist_selected_set(cfdata->ilist_categories, pos);
   e_widget_ilist_selector_set(cfdata->ilist_categories, 1);

   news_config_save();
}

static void
_cb_category_config(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Category *c;

   cfdata = data;
   c = cfdata->selected_category;
   if (!c) return;
   if (c->config_dialog) return;

   news_config_dialog_category_show(c);
}

static void
_cb_langs_all_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   news->config->feed.langs_all = cfdata->langs_all;

   if (news->config->feed.langs_all)
     e_widget_disabled_set(cfdata->button_langs, 1);
   else
     e_widget_disabled_set(cfdata->button_langs, 0);

   /* refreshes ! */
   news_feed_lang_list_refresh();
   news_feed_lists_refresh(0);
}

static void
_cb_langs_config(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   news_config_dialog_langs_show();
}

static void
_cb_empty(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   E_Confirm_Dialog *cd = NULL;

   cfdata = data;
   if (cfdata->cd) return;
   cd = e_confirm_dialog_show(D_("News Module - Are you sure ?"), "enlightenment",
                                 "<hilight>Empty  the lists</hilight><br>"
                                 "It will delete all the categories and feeds<br><br>"
                                 "<hilight>Confirm ?</hilight>",
                              NULL, NULL, _cb_empty_yes, NULL, NULL, NULL, 
                              _cb_confirm_dialog_destroy, cfdata);
   if (!cd) return;
   cfdata->cd = cd;
}

static void
_cb_empty_yes(void *data)
{
   news_feed_all_delete();
   news_viewer_all_refresh(0, 0);
}

static void
_cb_reset(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   E_Confirm_Dialog *cd = NULL;

   cfdata = data;
   if (cfdata->cd) return;
   cd = e_confirm_dialog_show(D_("News Module - Are you sure ?"), "enlightenment",
                                 "<hilight>Restore default lists</hilight><br>"
                                 "It will delete all the categories and feeds,<br>"
                                 "and restore the default ones<br><br>"
                                 "<hilight>Confirm ?</hilight>",
                              NULL, NULL, _cb_reset_yes, NULL, NULL, NULL, 
                              _cb_confirm_dialog_destroy, cfdata);
   if (!cd) return;
   cfdata->cd = cd;
}

static void
_cb_reset_yes(void *data)
{
   news_feed_all_restore();
   news_viewer_all_refresh(0, 0);
}

static void
_cb_confirm_dialog_destroy(void *data)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   cfdata->cd = NULL;
}
