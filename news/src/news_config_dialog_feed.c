#include "News.h"

struct _E_Config_Dialog_Data
{
   struct
   {
      Evas_Object *ilist_categories;
      Evas_Object *ilist_langs;
      Evas_Object *button_icon;
      Evas_Object *icon;

      Evas_Object *icon_sel;
      E_Dialog    *icon_sel_dia;
   } gui;

   char *name;
   int   name_ovrw;
   char *language;
   int   language_ovrw;
   char *description;
   int   description_ovrw;
   char *url_home;
   int   url_home_ovrw;
   char *url_feed;
   char *icon;
   int   icon_ovrw;
   int   important;
   News_Feed_Category *category;

   News_Feed *feed;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata, News_Feed *f);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void         _common_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata, Evas_Object *o);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _common_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void         _cb_lang_change(void *data, Evas_Object *obj);

static void         _cb_category_list(void *data);

static void         _icon_select(void *data1, void *data2);
static void         _icon_select_cb(void *data, Evas_Object *obj);
static void         _icon_select_ok(void *data, E_Dialog *dia);
static void         _icon_select_cancel(void *data, E_Dialog *dia);
static void         _icon_select_changed(void *data);

/*
 * Public functions
 */

int
news_config_dialog_feed_show(News_Feed *feed)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (!eina_list_count(news->config->feed.categories))
     {
        news_util_message_error_show(D_("You need to <hilight>create a category</hilight> first"));
        return 0;
     }

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _common_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _common_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;
   
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("News Feed Configuration"),
                             "E", "_e_mod_news_config_dialog_feed",
                             news_theme_file_get(NEWS_THEME_CAT_ICON), 0, v, feed);

   return 1;
}

void
news_config_dialog_feed_hide(News_Feed *feed)
{
   if (feed)
     {
        e_object_del(E_OBJECT(feed->config_dialog));
        feed->config_dialog = NULL;
     }
   else
     {
        e_object_del(E_OBJECT(news->config_dialog_feed_new));
        news->config_dialog_feed_new = NULL;
     }
}

void
news_config_dialog_feed_refresh_categories(News_Feed *feed)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l;
   Evas_Object *ilist;
   int pos, pos_to_select;
   int w;

   if (feed)
     {
        if (!feed->config_dialog) return;
        cfdata = feed->config_dialog->cfdata;
     }
   else
     {
        if (!news->config_dialog_feed_new) return;
        cfdata = news->config_dialog_feed_new->cfdata;
     }

   ilist = cfdata->gui.ilist_categories;
   e_widget_ilist_freeze(ilist);
   e_widget_ilist_clear(ilist);

   pos = 0;
   pos_to_select = -1;
   for(l=news->config->feed.categories; l; l=eina_list_next(l))
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

	snprintf(buf, sizeof(buf), "%s",
		 fc->name);

	if (cfdata->category == fc)
          pos_to_select = pos;

        e_widget_ilist_append(ilist, ic, buf, _cb_category_list, cfdata, NULL);

	pos++;
     }
   e_widget_ilist_go(ilist);
   e_widget_ilist_thaw(ilist);

   if (pos_to_select != -1)
     e_widget_ilist_selected_set(ilist, pos_to_select);
   else
     e_widget_ilist_selected_set(ilist, 0);
   _cb_category_list(cfdata);

   e_widget_size_min_get(ilist, &w, NULL);
   e_widget_size_min_set(ilist, w, 110);
}

void
news_config_dialog_feed_refresh_langs(News_Feed *feed)
{
   E_Config_Dialog_Data *cfdata;
   Evas_Object *ilist;
   News_Feed_Lang *lang;
   Eina_List *list, *l;
   int pos, pos_to_select;
   int w;

   if (feed)
     {
        if (!feed->config_dialog) return;
        cfdata = feed->config_dialog->cfdata;
     }
   else
     {
        if (!news->config_dialog_feed_new) return;
        cfdata = news->config_dialog_feed_new->cfdata;
     }

   ilist = cfdata->gui.ilist_langs;
   e_widget_ilist_freeze(ilist);
   e_widget_ilist_clear(ilist);

   if (news->config->feed.langs_all)
     list = news->langs;
   else
     list = news->config->feed.langs;
   pos = 0;
   pos_to_select = -1;
   for (l=list; l; l=eina_list_next(l))
     {
        lang = eina_list_data_get(l);
        e_widget_ilist_append(ilist, NULL, lang->name, NULL, lang, NULL);

        if (cfdata->language && !strcmp(cfdata->language, lang->key))
          pos_to_select = pos;
        pos++;
     }
   e_widget_ilist_go(ilist);
   e_widget_ilist_thaw(ilist);

   if (pos_to_select != -1)
     e_widget_ilist_selected_set(ilist, pos_to_select);
   else
     e_widget_ilist_selected_set(ilist, 0);
   _cb_lang_change(cfdata, NULL);
   

   e_widget_size_min_get(ilist, &w, NULL);
   e_widget_size_min_set(ilist, w, 110);
}

/*
 * Private functions
 *
 */

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   News_Feed *feed;

   feed = cfd->data;

   if (feed) feed->config_dialog = cfd;
   else news->config_dialog_feed_new = cfd;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata, feed);
   cfd->cfdata = cfdata;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   free(cfdata->name);
   free(cfdata->language);
   free(cfdata->description);
   free(cfdata->url_home);
   free(cfdata->url_feed);
   free(cfdata->icon);

   if (cfdata->gui.icon_sel)
     e_object_del(E_OBJECT(cfdata->gui.icon_sel));

   if (cfdata->feed)
     cfdata->feed->config_dialog = NULL;
   else
     news->config_dialog_feed_new = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata, News_Feed *f)
{
   char buf[4096];

   cfdata->feed = f;

   if (f)
     {
        fflush(stdout);
        cfdata->name = strdup(f->name);
        cfdata->name_ovrw = f->name_ovrw;
        cfdata->category = f->category;
        if (f->language) cfdata->language = strdup(f->language);
        else cfdata->language = strdup("");
        cfdata->language_ovrw = f->language_ovrw;
        if (f->description) cfdata->description = strdup(f->description);
        else cfdata->description = strdup("");
        cfdata->description_ovrw = f->description_ovrw;
        if (f->url_home) cfdata->url_home = strdup(f->url_home);
        else cfdata->url_home = strdup("");
        cfdata->url_home_ovrw = f->url_home_ovrw;
        cfdata->url_feed = strdup(f->url_feed);
        if (f->icon) cfdata->icon = strdup(f->icon);
        else
          {
             snprintf(buf, sizeof(buf), "%s/%s",
                      e_module_dir_get(news->module), NEWS_FEED_ITEM_FEED_ICON_DEFAULT);
             cfdata->icon = strdup(buf);
          }
        cfdata->icon_ovrw = f->icon_ovrw;
        cfdata->important = f->important;
     }
   else
     {
        cfdata->name = strdup("");
        cfdata->name_ovrw = NEWS_FEED_NAME_OVRW_DEFAULT;
        cfdata->category = NULL;
        cfdata->language = strdup("");
        cfdata->language_ovrw = NEWS_FEED_LANGUAGE_OVRW_DEFAULT;
        cfdata->description = strdup("");
        cfdata->description_ovrw = NEWS_FEED_DESCRIPTION_OVRW_DEFAULT;
        cfdata->url_home = strdup("http://");
        cfdata->url_home_ovrw = NEWS_FEED_URL_HOME_OVRW_DEFAULT;
        cfdata->url_feed = strdup("http://");
        snprintf(buf, sizeof(buf), "%s/%s",
                 e_module_dir_get(news->module), NEWS_FEED_ITEM_FEED_ICON_DEFAULT);
        cfdata->icon = strdup(buf);
        cfdata->icon_ovrw = NEWS_FEED_ICON_OVRW_DEFAULT;
     }
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o;

   o = e_widget_table_add(evas, 0);

   _common_create_widgets(cfd, evas, cfdata, o);

   e_dialog_resizable_set(cfd->dia, 1);

   return o;
}

static void
_common_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata, Evas_Object *o)
{
   Evas_Object *of, *ob;

   of = e_widget_frametable_add(evas, D_("Basic informations"), 0);

   ob = e_widget_button_add(evas, "", NULL,
                            _icon_select, cfdata, NULL);
   cfdata->gui.button_icon = ob;
   if (cfdata->icon)
     _icon_select_changed(cfdata);
   e_widget_size_min_set(ob, 48, 48);
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 0, 0, 0, 0);

   //ob = e_widget_check_add(evas, D_("Get from the server"), &(cfdata->icon_ovrw));
   //TODO: NOT IMPLEMENTED YET
   //e_widget_check_checked_set(ob, 0);
   //e_widget_disabled_set(ob, 1);
   //   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   ob = e_widget_label_add(evas, D_("Name :"));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &(cfdata->name), NULL, NULL, NULL);
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 0, 1, 0);
   ob = e_widget_label_add(evas, D_("Feed url :"));
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &(cfdata->url_feed), NULL, NULL, NULL);
   e_widget_frametable_object_append(of, ob, 0, 4, 1, 1, 1, 0, 1, 0);
   ob = e_widget_check_add(evas, D_("Mark as important feed"), &(cfdata->important));
   e_widget_frametable_object_append(of, ob, 0, 5, 1, 1, 1, 0, 1, 0);

   e_widget_table_object_append(o, of, 0, 0, 2, 1, 1, 1, 1, 1);

   of = e_widget_framelist_add(evas, D_("Category"), 0);

   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_selector_set(ob, 1);
   cfdata->gui.ilist_categories = ob;
   news_config_dialog_feed_refresh_categories(cfdata->feed);
   e_widget_framelist_object_append(of, ob);

   e_widget_table_object_append(o, of, 2, 0, 2, 1, 1, 1, 1, 1);

   of = e_widget_framelist_add(evas, D_("Language"), 0);

   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_selector_set(ob, 1);
   e_widget_on_change_hook_set(ob, _cb_lang_change, cfdata);
   cfdata->gui.ilist_langs = ob;
   news_config_dialog_feed_refresh_langs(cfdata->feed);
   e_widget_framelist_object_append(of, ob);

   e_widget_table_object_append(o, of, 4, 0, 2, 1, 1, 1, 1, 1);
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o;
   Evas_Object *of, *ob;

   o = e_widget_table_add(evas, 0);

   _common_create_widgets(cfd, evas, cfdata, o);


   of = e_widget_framelist_add(evas, D_("Advanced informations"), 0);

   ob = e_widget_label_add(evas, D_("Description"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_entry_add(evas, &(cfdata->description), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, D_("Home url"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_entry_add(evas, &(cfdata->url_home), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ob);

   e_widget_table_object_append(o, of, 0, 1, 3, 1, 1, 1, 1, 1);


   of = e_widget_framelist_add(evas, D_("Server informations"), 0);

   ob = e_widget_label_add(evas, D_("Do allow the server to overwrite the informations you entered ?"));
   e_widget_framelist_object_append(of, ob);

   ob = e_widget_check_add(evas, D_("Name"), &(cfdata->name_ovrw));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, D_("Language"), &(cfdata->language_ovrw));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, D_("Description"), &(cfdata->description_ovrw));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, D_("Home url"), &(cfdata->url_home_ovrw));
   e_widget_framelist_object_append(of, ob);

   e_widget_table_object_append(o, of, 3, 1, 3, 1, 1, 1, 1, 1);

   e_dialog_resizable_set(cfd->dia, 1);

   return o;
}

static int
_common_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   News_Feed *f;

   f = cfdata->feed;
   if (f)
     {
        News_Feed_Category *old_cat;

        old_cat = f->category;
        if ( news_feed_edit(f,
                            cfdata->name, cfdata->name_ovrw,
                            cfdata->language, cfdata->language_ovrw,
                            cfdata->description, cfdata->description_ovrw,
                            cfdata->url_home, cfdata->url_home_ovrw,
                            cfdata->url_feed,
                            cfdata->icon, cfdata->icon_ovrw,
                            cfdata->important,
                            cfdata->category, 0) )
          {
             if (old_cat != cfdata->category)
               {
                  old_cat->feeds = eina_list_remove(old_cat->feeds, f);
                  f->category->feeds = eina_list_prepend(f->category->feeds, f);
               }
             if (f->item && f->item->viewer)
               news_viewer_refresh(f->item->viewer);
          }
        else
          {
             DD(("FAILED to edit feed ! icon %s", cfdata->icon));
             return 0;
          }
     }
   else
     {
        News_Feed_Category *cat;

        cat = cfdata->category;

        f = news_feed_new(cfdata->name, cfdata->name_ovrw,
                          cfdata->language, cfdata->language_ovrw,
                          cfdata->description, cfdata->description_ovrw,
                          cfdata->url_home, cfdata->url_home_ovrw,
                          cfdata->url_feed,
                          cfdata->icon, cfdata->icon_ovrw,
                          cfdata->important,
                          cfdata->category);
        if (!f)
          {
             DD(("FAILED to create feed !"));
             return 0;
          }
        cat->feeds = eina_list_append(cat->feeds, f);

        cfdata->feed = f;
        news->config_dialog_feed_new = NULL;
        f->config_dialog = cfd;
     }

   news_feed_lists_refresh(1);

   news_config_save();

   return 1;
}

static void
_cb_lang_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Lang *lang;
   int pos;

   cfdata = data;
   pos = e_widget_ilist_selected_get(cfdata->gui.ilist_langs);
   if (pos < 0) return;
   lang = e_widget_ilist_nth_data_get(cfdata->gui.ilist_langs, pos);
   if (!lang) return;

   if (cfdata->language) free(cfdata->language);
   cfdata->language = strdup(lang->key);
}

static void
_cb_category_list(void *data)
{
   E_Config_Dialog_Data *cfdata;
   int pos;

   cfdata = data;
   pos = e_widget_ilist_selected_get(cfdata->gui.ilist_categories);
   if (pos < 0) return;

   cfdata->category = eina_list_nth(news->config->feed.categories, pos);
}

static void
_icon_select(void *data1, void *data2)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_Data *cfdata;
   E_Dialog *dia;
   Evas_Object *o;
   Evas_Coord mw, mh;
   char *dir = NULL;

   cfdata = data1;

   if (cfdata->gui.icon_sel) return;

   if (cfdata->feed) cfd = cfdata->feed->config_dialog;
   else cfd = news->config_dialog_feed_new;

   dia = e_dialog_new(cfd->con, "E", "_news_feed_icon_select_dialog");
   if (!dia) return;
   e_dialog_title_set(dia, D_("Select an Icon"));
   dia->data = cfdata;

   if (cfdata->icon)
     dir = ecore_file_dir_get(cfdata->icon);
   
   if (dir)
     {
        o = e_widget_fsel_add(dia->win->evas, dir, "/", NULL, NULL,
		              _icon_select_cb, cfdata,
		              NULL, cfdata, 1);
	free(dir);
     }
   else
     {
        o = e_widget_fsel_add(dia->win->evas, "~/", "/", NULL, NULL,
			      _icon_select_cb, cfdata,
			      NULL, cfdata, 1);
     }
   evas_object_show(o);
   cfdata->gui.icon_sel = o;
   e_widget_size_min_get(o, &mw, &mh);
   e_dialog_content_set(dia, o, mw, mh);

   /* buttons at the bottom */
   e_dialog_button_add(dia, D_("OK"), NULL, _icon_select_ok, cfdata);
   e_dialog_button_add(dia, D_("Cancel"), NULL, _icon_select_cancel, cfdata);
   e_dialog_resizable_set(dia, 1);
   e_win_centered_set(dia->win, 1);
   e_dialog_show(dia);
   e_win_resize(dia->win, 475, 341);
   cfdata->gui.icon_sel_dia = dia;
}

static void
_icon_select_cb(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
}

static void
_icon_select_ok(void *data, E_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;
   const char *file;

   cfdata = data;
   file = e_widget_fsel_selection_path_get(cfdata->gui.icon_sel);
   if (file)
     {
        if(cfdata->icon) free(cfdata->icon);
        cfdata->icon = strdup(file);

        _icon_select_changed(cfdata);
     }

   _icon_select_cancel(data, dia);
}

static void
_icon_select_cancel(void *data, E_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   e_object_del(E_OBJECT(dia));
   cfdata->gui.icon_sel = NULL;
   cfdata->gui.icon_sel_dia = NULL;
}

static void
_icon_select_changed(void *data)
{
   E_Config_Dialog_Data *cfdata;
   Evas_Object *ic;

   cfdata = data;

   if (!cfdata->icon) return;

   ic = e_icon_add(evas_object_evas_get(cfdata->gui.button_icon));
   e_icon_file_set(ic, cfdata->icon);
   e_icon_fill_inside_set(ic, 1);

   if (cfdata->gui.icon) evas_object_del(cfdata->gui.icon);

   e_widget_button_icon_set(cfdata->gui.button_icon, ic);
   cfdata->gui.icon = ic;
}
