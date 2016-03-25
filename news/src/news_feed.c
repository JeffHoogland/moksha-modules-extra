#include "News.h"

#define CFG_CAT_ADD(id, icon)                                       \
 {                                                                  \
    snprintf(buf, sizeof(buf), "%s/%s",                             \
             e_module_dir_get(news->module), icon);                 \
    cat = news_feed_category_new(id, buf);                          \
    c->feed.categories = eina_list_append(c->feed.categories, cat); \
 }
#define CFG_FEED_ADD(id, icon)                       \
 {                                                   \
    snprintf(buf, sizeof(buf), "%s/%s",              \
             e_module_dir_get(news->module), icon);  \
    feed = news_feed_new(id, buf, 1, 0, cat);        \
    cat->feeds = eina_list_append(cat->feeds, feed); \
 }

static int        _feed_activate(News_Feed *f);
static void       _feed_deactivate(News_Feed *f);
static News_Feed *_feed_find(News_Feed_Category *cat, char *name);
static News_Feed_Category *_feed_category_find(char *name);
static char      *_get_host_from_url(const char *url);
static char      *_get_file_from_url(const char *url);
static void       _cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void       _cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void       _cb_feed_open(void *data, Evas_Object *obj, const char *emission, const char *source);
static Eina_Bool        _cb_feed_server_add(void *data, int type, void *event);
static Eina_Bool        _cb_feed_server_del(void *data, int type, void *event);
static Eina_Bool        _cb_feed_server_data(void *data, int type, void *event);
static void       _cb_feed_parse(News_Feed_Document *doc, News_Parse_Error error, int changes);
static Eina_Bool        _cb_feeds_timer(void *data);
static int        _cb_sort_cats(const void *d1, const void *d2);
static int        _cb_sort_feeds(const void *d1, const void *d2);

static const News_Feed_Lang _feed_langs[] = {
   {"ca", "Catalan"},
   {"zh", "Chinese"},
   {"cs", "Czech"},
   {"nl", "Dutch"},
   {"da", "Danish"},
   {"de", "German"},
   {"en", "English"},
   {"fi", "Finnish"},
   {"fr", "French"},
   {"hu", "Hungarian"},
   {"it", "Italian"},
   {"ja", "Japanese"},
   {"ko", "Korean"},
   {"pl", "Polish"},
   {"pt", "Portuguese"},
   {"ru", "Russian"},
   {"sk", "Slovak"},
   {"sl", "Slovenian"},
   {"es", "Spanish"},
   {"sv", "Swedish"},
   {NULL, NULL}
};

/*
 * Public functions
 */

int
news_feed_init(void)
{
   News_Feed_Lang *lang;
   Eina_List *l;
   int i;

   /* create dynamic languages list from static one
      we create this list so language are accessible from everywhere */
   l = NULL;
   i = 0;
   while (_feed_langs[i].key)
     {
        lang = E_NEW(News_Feed_Lang, 1);
        lang->key = eina_stringshare_add(_feed_langs[i].key);
        lang->name = eina_stringshare_add(_feed_langs[i].name);
        l = eina_list_append(l, lang);
        i++;
     }
   news->langs = l;

   /* validate each feed and create host and file strings + attach to category */
   NEWS_FEED_FOREACH_BEG();
   if (!news_feed_edit(_feed,
                       (char *)_feed->name, _feed->name_ovrw,
                       (char *)_feed->language, _feed->language_ovrw,
                       (char *)_feed->description, _feed->description_ovrw,
                       (char *)_feed->url_home, _feed->url_home_ovrw,
                       (char *)_feed->url_feed,
                       (char *)_feed->icon, _feed->icon_ovrw,
                       _feed->important,
                       _cat, 1))
     _cat->feeds = eina_list_remove_list(_cat->feeds, _l_cats);
   NEWS_FEED_FOREACH_END();

   /* create 'feeds_visible' lists in categories */
   news_feed_lists_refresh(0);

   /* set the update timer */
   news_feed_timer_set(news->config->feed.timer_m);

   return 1;
}

void
news_feed_shutdown(void)
{
   /* destroy dynamic languages list */
   news_feed_lang_list_free(news->langs);

   /* delete the update timer */
   news_feed_timer_set(0);
}

void
news_feed_all_delete(void)
{
   News_Config *c;
   News_Feed *f;
   News_Feed_Category *cat;

   c = news->config;
   while (c->feed.categories)
     {
        cat = c->feed.categories->data;
        while (cat->feeds)
          {
             f = cat->feeds->data;
             cat->feeds = eina_list_remove_list(cat->feeds, cat->feeds);
             news_feed_free(f);
          }
        c->feed.categories = eina_list_remove_list(c->feed.categories,
                                                   c->feed.categories);
        news_feed_category_free(cat);
     }

   news_feed_lists_refresh(0);
}

void
news_feed_all_restore(void)
{
   News_Config *c;
   News_Feed_Category *cat;
   News_Feed *feed;
   char buf[4096];
   
   c = news->config;
   news_feed_all_delete();

   /* FEED_ADD uses the cat created by CAT_ADD */
   CFG_CAT_ADD( NEWS_FEED_ITEM_CAT_COMICS, NEWS_FEED_ITEM_CAT_ICON_COMICS );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_BOULETCORP, NEWS_FEED_ITEM_FEED_ICON_BOULETCORP );
   CFG_CAT_ADD( NEWS_FEED_ITEM_CAT_COMPUTERS, NEWS_FEED_ITEM_CAT_ICON_COMPUTERS );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_FRESHMEAT, NEWS_FEED_ITEM_FEED_ICON_FRESHMEAT );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_OSNEWS, NEWS_FEED_ITEM_FEED_ICON_OSNEWS );
   CFG_CAT_ADD( NEWS_FEED_ITEM_CAT_ENLIGHTENMENT, NEWS_FEED_ITEM_CAT_ICON_ENLIGHTENMENT );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_E_CVS, NEWS_FEED_ITEM_FEED_ICON_E_CVS );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_E_FR, NEWS_FEED_ITEM_FEED_ICON_E_FR );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_NEWS, NEWS_FEED_ITEM_FEED_ICON_NEWS );
   CFG_CAT_ADD( NEWS_FEED_ITEM_CAT_NEWS, NEWS_FEED_ITEM_CAT_ICON_NEWS );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_AFP_EN, NEWS_FEED_ITEM_FEED_ICON_AFP_EN );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_BBC_WORLD, NEWS_FEED_ITEM_FEED_ICON_BBC_WORLD );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_CNN, NEWS_FEED_ITEM_FEED_ICON_CNN );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_COURRIERINTERNATIONAL, NEWS_FEED_ITEM_FEED_ICON_COURRIERINTERNATIONAL );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_GOOGLE, NEWS_FEED_ITEM_FEED_ICON_GOOGLE );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_GOOGLE_FR, NEWS_FEED_ITEM_FEED_ICON_GOOGLE_FR );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_LEMONDE_UNE, NEWS_FEED_ITEM_FEED_ICON_LEMONDE );
   CFG_CAT_ADD( NEWS_FEED_ITEM_CAT_SCIENCES, NEWS_FEED_ITEM_CAT_ICON_SCIENCES );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_FUTURASCIENCES, NEWS_FEED_ITEM_FEED_ICON_FUTURASCIENCES );
   CFG_CAT_ADD( NEWS_FEED_ITEM_CAT_TEST, NEWS_FEED_ITEM_CAT_ICON_TEST );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_TEST, NEWS_FEED_ITEM_FEED_ICON_TEST );
   CFG_CAT_ADD( NEWS_FEED_ITEM_CAT_VARIOUS, NEWS_FEED_ITEM_CAT_ICON_VARIOUS );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_BARRAPUNTO, NEWS_FEED_ITEM_FEED_ICON_BARRAPUNTO );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_DIGG, NEWS_FEED_ITEM_FEED_ICON_DIGG );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_MENEAME, NEWS_FEED_ITEM_FEED_ICON_MENEAME );
   CFG_FEED_ADD(  NEWS_FEED_ITEM_FEED_SLASHDOT, NEWS_FEED_ITEM_FEED_ICON_SLASHDOT );

   news_feed_lists_refresh(0);
}

void
news_feed_lists_refresh(int sort)
{
   Eina_List *l, *l2, *list;
   News_Feed_Category *cat;
   News_Feed *f;
   int list_free;

   /* 1. sort by name */

   if (sort && news->config->feed.sort_name)
     {
        list = news->config->feed.categories;
        list = eina_list_sort(list, eina_list_count(list), _cb_sort_cats);
        news->config->feed.categories = list;

        for (l=news->config->feed.categories; l; l=eina_list_next(l))
          {
             cat = l->data;
             list = cat->feeds;
             list = eina_list_sort(list, eina_list_count(list), _cb_sort_feeds);
             cat->feeds = list;
          }
     }

   /* 2. create "feeds_visible" in categories */

   for (l=news->config->feed.categories; l; l=eina_list_next(l))
     {
        cat = l->data;
        list = NULL;
        list_free = 0;
        if (news->config->feed.langs_all)
          {
             list = cat->feeds;
          }
        else
          {
             for (l2=cat->feeds; l2; l2=eina_list_next(l2))
               {
                  f = l2->data;
                  if (news_feed_lang_selected_is(f->language))
                    list = eina_list_append(list, f);
               }
             list_free = 1;
          }

        if (cat->feeds_visible_free && cat->feeds_visible)
          eina_list_free(cat->feeds_visible);

        cat->feeds_visible = list;
        cat->feeds_visible_free = list_free;
     }

   /* 3. refresh ui */

   news_feed_category_list_ui_refresh();
}

void
news_feed_timer_set(int time)
{
   if (news->feeds_timer)
     {
        ecore_timer_del(news->feeds_timer);
        news->feeds_timer = NULL;
     }
   if (!time) return;

   news->config->feed.timer_m = time;
   news->feeds_timer = ecore_timer_add(time * 60,
                                       _cb_feeds_timer, NULL);
}

News_Feed *
news_feed_new(char *name, int name_ovrw, char *language, int language_ovrw, char *description, int description_ovrw, char *url_home, int url_home_ovrw, char *url_feed, char *icon, int icon_ovrw, int important, News_Feed_Category *category)
{
   News_Feed *f;

   f = E_NEW(News_Feed, 1);

   if (!news_feed_edit(f,
                       name, name_ovrw,
                       language, language_ovrw,
                       description, description_ovrw,
                       url_home, url_home_ovrw,
                       url_feed,
                       icon, icon_ovrw,
                       important,
                       category, 0))
     {
        free(f);
        return NULL;
     }

   return f;
}

int
news_feed_edit(News_Feed *f, char *name, int name_ovrw, char *language, int language_ovrw, char *description, int description_ovrw, char *url_home, int url_home_ovrw, char *url_feed, char *icon, int icon_ovrw, int important, News_Feed_Category *category, int check_only)
{
   News_Feed *f2;
   char *host, *file;
   int update = 0;

   if ( !name || !name[0] )
     {
        news_util_message_error_show(D_("The <hilight>name</hilight> you entered is not correct"));
        return 0;
     }
   if (!category)
     {
        news_util_message_error_show(D_("The need to <hilight>select a category</hilight>."));
        return 0;
     }
   f2 = _feed_find(category, name);
   if ( f2 && (f != f2) )
     {
        news_util_message_error_show(D_("A feed with the <hilight>name</hilight> %s <hilight>already exists</hilight><br>"
                                        "in the %s category<br><br>"
                                        "Its not possible to have feeds with the same name<br>"
                                        "in one category."),
                                     name, category->name);
        return 0;
     }
   if (!language || !language[0])
     {
        news_util_message_error_show(D_("You need to select a <hilight>language</hilight>."));
        return 0;
     }
   if ( !url_feed || !url_feed[0] ||
        !(host = _get_host_from_url(url_feed)) ||
        !(file = _get_file_from_url(url_feed)) )
     {
        news_util_message_error_show(D_("The <hilight>url</hilight> you specified is not correct"));
        if (host) free(host);
        return 0;
     }

   if (!check_only)
     {
        if (f->name != name)
          {
             if (f->name) eina_stringshare_del(f->name);
             f->name = eina_stringshare_add(name);
          }
        f->name_ovrw = name_ovrw;

        if (f->language != language)
          {
             char lang[3];

             if (f->language) eina_stringshare_del(f->language);
             snprintf(lang, sizeof(lang), "%2s", language);
             if (language) f->language = eina_stringshare_add(lang);
          }
        f->language_ovrw = language_ovrw;

        if (f->description != description)
          {
             if (f->description) eina_stringshare_del(f->description);
             if (description) f->description = eina_stringshare_add(description);
          }
        f->description_ovrw = description_ovrw;

        if (f->url_home != url_home)
          {
             if (f->url_home) eina_stringshare_del(f->url_home);
             if (url_home) f->url_home = eina_stringshare_add(url_home);
          }
        f->url_home_ovrw = url_home_ovrw;

        if ( (f->url_feed != url_feed) &&
             (!f->url_feed || strcmp(f->url_feed, url_feed)) )
          {
             if (f->url_feed) eina_stringshare_del(f->url_feed);
             f->url_feed = eina_stringshare_add(url_feed);
             update = 1;
          }

        if (f->icon != icon)
          {
             if ( f->icon &&
                  !(icon && !strcmp(f->icon, icon)) )
               {
                  eina_stringshare_del(f->icon);
                  f->icon = NULL;
                  if (f->obj_icon)
                    {
                       evas_object_del(f->obj_icon);
                       f->obj_icon = NULL;
                    }
               }
             if (icon && !f->icon) f->icon = eina_stringshare_add(icon);
          }
        f->icon_ovrw = icon_ovrw;

        if (f->important != important)
          {
             f->important = important;
             if (f->item
                 && ( (f->item->config->view_mode == NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT)
                      ||
                      ((f->item->config->view_mode == NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT_UNREAD)
                       && f->doc && f->doc->unread_count)))
               news_item_refresh(f->item, 1, 0, 1);
          }

        f->category = category;

        if (f->item)
	  {
             News_Feed_Ref *ref;

             if (f->icon && !f->obj_icon)
               news_feed_obj_refresh(f, 1, 1);

             /* Feed Ref update */
             ref = news_feed_ref_find(f, f->item);
             if (ref)
               {
                  if (ref->name) eina_stringshare_del(ref->name);
                  ref->name = eina_stringshare_add(f->name);
                  if (ref->category) eina_stringshare_del(ref->category);
                  ref->category = eina_stringshare_add(f->category->name);
               }
	  }

        news_config_save();
     }
   else
     {
        f->category = category;
     }

   E_FREE(f->host);
   f->host = host;
   E_FREE(f->file);
   f->file = file;

   if (update && f->item) news_feed_update(f);

   return 1;
}

void
news_feed_free(News_Feed *f)
{
   News_Item *ni;

   ni = f->item;
   if (ni)
     {
        news_feed_detach(f, 1);
        news_item_refresh(ni, 1, 0, 0);
        news_viewer_refresh(ni->viewer);
     }
   if (f->doc) _feed_deactivate(f);

   if (f->config_dialog) news_config_dialog_feed_hide(f);
   if (f->menu) news_menu_feed_hide(f);
   if (f->obj_icon) evas_object_del(f->obj_icon);

   if (f->name) eina_stringshare_del(f->name);
   if (f->language) eina_stringshare_del(f->language);
   if (f->description) eina_stringshare_del(f->description);
   if (f->url_home) eina_stringshare_del(f->url_home);
   if (f->url_feed) eina_stringshare_del(f->url_feed);
   if (f->icon) eina_stringshare_del(f->icon);

   if (f->host) free(f->host);
   if (f->file) free(f->file);

   free(f);
}

/*
 * Attach a feed to an item
 * 
 * You can give the feed or the feed ref
 */
int
news_feed_attach(News_Feed *f, News_Feed_Ref *ref, News_Item *ni)
{
   if (!f)
     {
        News_Feed_Category *cat;
        News_Feed *f_look;
        Eina_List *l, *l2;
      
        for (l=news->config->feed.categories; l; l=eina_list_next(l))
          {
             cat = eina_list_data_get(l);
             if (!strcmp(cat->name, ref->category))
               {
                  for (l2=cat->feeds; l2; l2=eina_list_next(l2))
                    {
                       f_look = eina_list_data_get(l2);
                       if (!strcmp(f_look->name, ref->name))
                         {
                            f = f_look;
                            l = NULL;
                            break;
                         }
                    }
               }
          }
        if (!f) return 0;
     }

   if (!ref)
     {
        ref = E_NEW(News_Feed_Ref, 1);
        ref->category = eina_stringshare_add(f->category->name);
        ref->name = eina_stringshare_add(f->name);
        ref->feed = f;
        ni->config->feed_refs = eina_list_append(ni->config->feed_refs, ref);
     }
   else
     {
        ref->feed = f;
     }

   f->item = ni;
   if (f->doc)
     {
        if (f->doc->unread_count)
          news_item_unread_count_change(f->item, +1);
     }
   else
     {
        _feed_activate(f);
        news_feed_update(f);
     }

   return 1;
}

/*
 * Detach a feed from the item wich its attached to
 * 
 * If @p really is 0, only detach the feed from the item, but keep feed ref
 * If @p really is 1, remove the feed ref too
 */
void
news_feed_detach(News_Feed *f, int really)
{
   News_Item *ni;

   /* 1. delete the feed reference from items to feed */
   ni = f->item;
   if (ni)
     {
        News_Feed_Ref *ref;

        ref = news_feed_ref_find(f, ni);
        if (ref)
          {
             if (really)
               {
                  DITEM(("feed detach : delete feed ref (%s)", ref->feed->name));
                  ni->config->feed_refs = eina_list_remove(ni->config->feed_refs, ref);
                  eina_stringshare_del(ref->category);
                  eina_stringshare_del(ref->name);
                  free(ref);
               }
             else
               ref->feed = NULL;
          }
     }

   /* 2. gui items */
   if (f->obj)
     {
        evas_object_del(f->obj);
        f->obj = NULL;
     }
   if (f->obj_icon)
     {
        evas_object_del(f->obj_icon);
        f->obj_icon = NULL;
     }

   /* 3. unread count */
   if (f->doc && f->doc->unread_count)
     news_item_unread_count_change(f->item, -1);

   /* 4. misc cleanups */
   if (ni && ni->viewer)
     {
        if (ni->viewer->vfeeds.selected == f)
          ni->viewer->vfeeds.selected = NULL;
     }    

   /* 5. delete reference from feed to item */
   f->item = NULL;

   /* 6. the feed will be deactivated on next feed_update :)
      we wait until that because _detach is also called when there
      is a gadcon change (like size) */
}

int
news_feed_update(News_Feed *f)
{
   News_Feed_Document *doc;

   /* if the feed is not attached to an item
      it means that it was detached and never reattached,
      so deactivate the feed */
   if (!f->item)
     {
        _feed_deactivate(f);
        return 0;
     }

   doc = f->doc;

   if (doc->server.buffer)
     {
        free(doc->server.buffer);
        doc->server.buffer = NULL;
     }
   doc->server.buffer_size = 0;

   if (doc->server.conn)
     {
        //FIXME: segfault appears if i delete the server when connection has not been created yet (needs e patch)
        if (doc->server.waiting_reply)
          {
             //news_util_debug("Forced feed dl with waiting_replay of 1 !<br>ecore_con_server_del !<br>(%d tries)", doc->server.nb_tries);
             ecore_con_server_del(doc->server.conn);
             doc->server.waiting_reply = 0;
          }
        if (doc->server.nb_tries >= NEWS_FEED_NB_TRIES_MAX)
          return 0;
        doc->server.nb_tries++;
        doc->server.conn = NULL;
        //TODO: popup to warn that we are at at the nb_tries try =)
     }
   else
     doc->server.nb_tries = 1;

   if (news->config->proxy.enable &&
       news->config->proxy.port &&
       news->config->proxy.host && news->config->proxy.host[0])
     doc->server.conn = ecore_con_server_connect(ECORE_CON_REMOTE_NODELAY,
						 news->config->proxy.host,
						 news->config->proxy.port,
						 doc);
   else
     doc->server.conn = ecore_con_server_connect(ECORE_CON_REMOTE_NODELAY,
						 f->host,
						 80,
						 doc);

   if (!doc->server.conn)
     {
        //TODO: popup, if proxy, tell it
        DFEED(("Could not start connection to %s",
               f->host));
        return 0;
     }
   DFEED(("Trying to update feed %s", f->host));

   if (f->item && !f->item->loading_state)
     news_item_loadingstate_refresh(f->item);

   if (f->item && f->item->viewer &&
       (f->item->viewer->vfeeds.selected == f))
     news_viewer_feed_selected_infos_refresh(f->item->viewer);

   return 1;
}

/* 
 * Refresh the object showed in item
 */
void
news_feed_obj_refresh(News_Feed *f, int changed_content, int changed_state)
{
   News_Item *ni;
   Evas_Object *obj;

   obj = f->obj;
   ni = f->item;

   if (!ni) return;

   /* base obj */

   if (!f->obj)
     {
        obj = edje_object_add(ni->gcc->gadcon->evas);
        news_theme_edje_set(obj, NEWS_THEME_FEED);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                       _cb_mouse_down, f);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_OUT,
                                       _cb_mouse_out, f);
        edje_object_signal_callback_add(obj, "e,action,open", "e",
                                        _cb_feed_open, f);
        evas_object_propagate_events_set(obj, 0);
        evas_object_show(obj);
     }

   if (!f->obj || changed_state)
     {
        if (f->doc && f->doc->unread_count)
          edje_object_signal_emit(obj, "e,state,new,set", "e");
        else
          edje_object_signal_emit(obj, "e,state,new,unset", "e");
     }

   if (!f->obj || changed_content)
     {
        edje_object_part_text_set(obj, "name", f->name);
     }

   /* icon */

   if (f->obj_icon && changed_content)
     {
        evas_object_del(f->obj_icon);
        f->obj_icon = NULL;
     }

   if (!f->obj_icon && f->icon && f->icon[0])
     {
        Evas_Object *icon;
        
        DFEED(("Refresh view obj icon of feed %s : %s", f->name, f->icon));
        icon = e_icon_add(ni->gcc->gadcon->evas);
        e_icon_file_set(icon, f->icon);
        e_icon_fill_inside_set(icon, 1);
        edje_object_part_swallow(obj, "icon", icon);
        evas_object_show(icon);
        f->obj_icon = icon;
     }

   f->obj = obj;
}


News_Feed_Ref *
news_feed_ref_find(News_Feed *f, News_Item *ni)
{
   NEWS_ITEM_FEEDS_FOREACH_BEG(ni);
   if (_feed == f)
     return _ref;
   NEWS_ITEM_FEEDS_FOREACH_END();

   return NULL;
}

void
news_feed_read_set(News_Feed *feed)
{
   Eina_List *l;
   News_Feed_Article *art;

   if (!feed->doc) return;
   if (!feed->doc->unread_count) return;

   for (l=feed->doc->articles; l; l=eina_list_next(l))
     {
        art = l->data;
        if (art->unread)
          news_feed_article_unread_set(art, 0);
     }
}

void
news_feed_unread_count_change(News_Feed *feed, int nb)
{
   int was_empty = 0;

   if (!nb) return;
   if (!feed->doc->unread_count)
     was_empty = 1;
   feed->doc->unread_count += nb;
   if (was_empty || !feed->doc->unread_count)
     {
        if (nb > 0)
          news_item_unread_count_change(feed->item, 1);
        else
          news_item_unread_count_change(feed->item, -1);
        
        switch ((News_Item_View_Mode)feed->item->config->view_mode)
          {
          case NEWS_ITEM_VIEW_MODE_ONE:
             break;
          case NEWS_ITEM_VIEW_MODE_FEED:
          case NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT:
             news_feed_obj_refresh(feed, 0, 1);
             break;
          case NEWS_ITEM_VIEW_MODE_FEED_UNREAD:
             news_item_refresh(feed->item, 1, 0, 1);
             break;
          case NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT_UNREAD:
             if (feed->important)
               news_item_refresh(feed->item, 1, 0, 1);
             break;
          }

        if (feed->item->viewer)
          news_viewer_refresh(feed->item->viewer);
     }
   
   DFEED(("%s: UNREADcount = %d", feed->name, feed->doc->unread_count));
}

void
news_feed_list_ui_refresh(void)
{
   Eina_List *l;

   if (news->config_dialog_feeds)
     news_config_dialog_feeds_refresh_feeds();

   for (l=news->items; l; l=eina_list_next(l))
     {
        News_Item *ni;

        ni = eina_list_data_get(l);
        if (ni->config_dialog_content)
	  {
             news_config_dialog_item_content_refresh_feeds(ni);
             news_config_dialog_item_content_refresh_selected_feeds(ni);
	  }
     }
}

void
news_feed_article_del(News_Feed_Article *art)
{
   News_Feed_Document *doc;

   doc = art->doc;
   if (art->title) free(art->title);
   if (art->url) free(art->url);
   if (art->description) free(art->description);
   if (art->image) evas_object_del(art->image);

   if (doc->feed->item && doc->feed->item->viewer)
     {
        if (doc->feed->item->viewer->varticles.selected == art)
          doc->feed->item->viewer->varticles.selected = NULL;
     }

   if (doc->feed->item && art->unread)
     news_feed_unread_count_change(doc->feed, -1);

   doc->articles = eina_list_remove(doc->articles, art);

   free(art);
}

void
news_feed_article_unread_set(News_Feed_Article *art, int unread)
{
   /* check already in that state */
   if (unread == art->unread) return;

   art->unread = unread;
   if (art->doc->feed->item->viewer)
     news_viewer_article_state_refresh(art->doc->feed->item->viewer, art);

   if (unread)
     news_feed_unread_count_change(art->doc->feed, 1);
   else
     news_feed_unread_count_change(art->doc->feed, -1);
}

News_Feed_Category *
news_feed_category_new(char *name, char *icon)
{
   News_Feed_Category *cat;

   cat = E_NEW(News_Feed_Category, 1);

   if (!news_feed_category_edit(cat, name, icon))
     {
        free(cat);
        return NULL;
     }

   return cat;
}

int
news_feed_category_edit(News_Feed_Category *cat, char *name, char *icon)
{
   News_Feed_Category *cat2;

   if (!name || !name[0])
     {
        news_util_message_error_show(D_("You need to enter a <hilight>name</hilight> !"));
        return 0;
     }

   /* that name already exists ? */
   cat2 = _feed_category_find(name);
   if (cat2 && (cat2 != cat))
     {
        news_util_message_error_show(D_("The <hilight>name</hilight> you entered is <hilight>already used</hilight><br>"
                                       "by another category"));
        return 0;
     }

   if (cat->name != name)
     {
        if (cat->name) eina_stringshare_del(cat->name);
        cat->name = eina_stringshare_add(name);
     }

   if (cat->icon != icon)
     {
        if (cat->icon) eina_stringshare_del(cat->icon);
        if (icon) cat->icon = eina_stringshare_add(icon);
     }

   return 1;
}

void
news_feed_category_free(News_Feed_Category *cat)
{
   if (cat->feeds && eina_list_count(cat->feeds)) return;
   
   if (cat->name) eina_stringshare_del(cat->name);
   if (cat->icon) eina_stringshare_del(cat->icon);
   if (cat->feeds_visible_free && cat->feeds_visible)
     eina_list_free(cat->feeds_visible);
   if (cat->config_dialog) news_config_dialog_category_hide(cat);
   
   free(cat);
}

void
news_feed_category_list_ui_refresh(void)
{
   if (news->config_dialog_feeds)
     news_config_dialog_feeds_refresh_categories();
   if (news->config_dialog_feed_new)
     news_config_dialog_feed_refresh_categories(NULL);

   NEWS_FEED_FOREACH_BEG();
   if (_feed->config_dialog)
     news_config_dialog_feed_refresh_categories(_feed);
   NEWS_FEED_FOREACH_END();

   news_feed_list_ui_refresh();
}

void
news_feed_lang_list_refresh(void)
{
   if (news->config_dialog_feed_new)
     news_config_dialog_feed_refresh_langs(NULL);

   NEWS_FEED_FOREACH_BEG();
   if (_feed->config_dialog)
     news_config_dialog_feed_refresh_langs(_feed);
   NEWS_FEED_FOREACH_END();
}

void
news_feed_lang_list_free(Eina_List *list)
{
   News_Feed_Lang *lang;

   while ((lang = eina_list_data_get(list)))
     {
        if (lang->key) eina_stringshare_del(lang->key);
        if (lang->name) eina_stringshare_del(lang->name);
        list = eina_list_remove_list(list, list);
        free(lang);
     }
}

const char *
news_feed_lang_name_get(const char *key)
{
   int i;

   i = 0;
   while (_feed_langs[i].key)
     {
        if (!strncmp(_feed_langs[i].key, key, 2))
          return _feed_langs[i].name;
        i++;
     }
   return NULL;
}

int
news_feed_lang_selected_is(const char *key)
{
   News_Feed_Lang *lang;
   Eina_List *l;

   for (l=news->config->feed.langs; l; l=eina_list_next(l))
     {
        lang = l->data;
        if (!strncmp(lang->key, key, 2))
          return 1;
     }
   return 0;
}

/*
 * Private functions
 *
 */

static int
_feed_activate(News_Feed *f)
{
   News_Feed_Document *doc;

   doc = E_NEW(News_Feed_Document, 1);

   doc->feed = f;
   f->doc = doc;

   doc->ui_needrefresh = 1;

   doc->server.handler_add = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
						     _cb_feed_server_add, doc);
   doc->server.handler_del = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
						     _cb_feed_server_del, doc);
   doc->server.handler_data = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
                                                      _cb_feed_server_data, doc);

   doc->parse.type = NEWS_FEED_TYPE_UNKNOWN;

   return 1;
}

static void
_feed_deactivate(News_Feed *f)
{
   News_Feed_Document *doc;

   doc = f->doc;
   if (!doc) return;

   DFEED(("Feed %s deactivate", f->name));

   if (doc->parse.parser) news_parse_stop(doc);
       
   if (doc->parse.meta_article) eina_stringshare_del(doc->parse.meta_article);
   if (doc->parse.meta_channel) eina_stringshare_del(doc->parse.meta_channel);
   if (doc->parse.meta_date) eina_stringshare_del(doc->parse.meta_date);
   if (doc->parse.charset) eina_stringshare_del(doc->parse.charset);

   //FIXME: waiting ecore_con patch to be able to delete the server in all cases
   if (doc->server.conn && doc->server.waiting_reply) ecore_con_server_del(doc->server.conn);
   ecore_event_handler_del(doc->server.handler_add);
   ecore_event_handler_del(doc->server.handler_del);
   ecore_event_handler_del(doc->server.handler_data);
   if (doc->server.buffer) free(doc->server.buffer);
  
   if (doc->articles)
     {
        News_Feed_Article *article;

        while(doc->articles)
          {
             article = doc->articles->data;
             news_feed_article_del(article);
          }
     }
  
   if (doc->popw) news_popup_del(doc->popw);

   f->doc = NULL;
   free(doc);
}

static News_Feed *
_feed_find(News_Feed_Category *cat, char *name)
{
   News_Feed *f;
   Eina_List *l;

   for (l=cat->feeds; l; l=eina_list_next(l))
     {
        f = l->data;
        if (!strcmp(f->name, name))
          return f;
     }

   return NULL;
}

static News_Feed_Category *
_feed_category_find(char *name)
{
   Eina_List *l;

   for (l=news->config->feed.categories; l; l=eina_list_next(l))
     {
        News_Feed_Category *cat;

        cat = eina_list_data_get(l);
        if (!strcmp(cat->name, name))
          return cat;
     }

   return NULL;
}

static char *
_get_host_from_url(const char *url)
{
   char *host, *p;
   char buf[4096];

   strncpy(buf, url, sizeof(buf));
   if (strncmp(buf, "http://", 7))
     return NULL;
   if (*(buf+7) == '\0')
     return NULL;
   p = strchr(buf+7, '/');
   if (p) *p = '\0';
   host = strdup(buf+7);

   return host;
}

static char *
_get_file_from_url(const char *url)
{
   char *file, *p;

   p = strstr(url, "://");
   if (!p)
     return NULL;
   p += 3;
   p = strstr(p, "/");
   if (!p)
     file = strdup("/");
   else
     file = strdup(p);

   return file;
}

static void
_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   News_Feed *f;
   Evas_Event_Mouse_Down *ev;

   f = data;
   ev = event_info;

   DFEED(("Mouse down %d", ev->button));

   switch(ev->button)
     {
        int cx, cy, cw, ch;
        
     case 3:
        if (f->menu) break;
        
        news_menu_feed_show(f);
        
        e_gadcon_canvas_zone_geometry_get(f->item->gcc->gadcon,
                                          &cx, &cy, &cw, &ch);
        e_menu_activate_mouse(f->menu,
                              e_util_zone_current_get(e_manager_current_get()),
                              cx + ev->output.x, cy + ev->output.y, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(f->item->gcc->gadcon->evas, ev->button,
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
_cb_feed_open(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   News_Item *ni;
   News_Feed *f;

   f = data;
   ni = f->item;

   DITEM(("Cb feed %s open", f->name));

   /* if the object is not loaded yet, abort */
   if (!f->doc) return;

   switch (f->doc->feed->item->config->openmethod)
     {
     case NEWS_ITEM_OPENMETHOD_VIEWER:
        if (ni->viewer)
          {
             if (ni->viewer->vfeeds.selected == f)
               news_viewer_destroy(ni->viewer);
             else
               news_viewer_feed_select(ni->viewer, f);
          }
        else
          {
             news_viewer_create(ni);
             news_viewer_feed_select(ni->viewer, f);
          }
        break;
     case NEWS_ITEM_OPENMETHOD_BROWSER:
        if (ni->config->browser_open_home)
          news_util_browser_open(f->url_home);
        else
          news_util_browser_open(f->url_feed);
        news_feed_read_set(f);
        break;
     }
}

static Eina_Bool
_cb_feed_server_add(void *data, int type, void *event)
{
   News_Feed_Document *doc;
   Ecore_Con_Event_Server_Add *ev;
   char buf[4096];

   doc = data;
   ev = event;

   /* check if the event is our event */
   if (doc->server.conn != ev->server)
     return EINA_TRUE;

   DFEED(("Connection established after %d tries, sending request", doc->server.nb_tries));

   snprintf(buf, sizeof(buf), "GET %s HTTP/1.0\r\n", doc->feed->file);
   ecore_con_server_send(doc->server.conn, buf, strlen(buf));
   snprintf(buf, sizeof(buf), "Host: %s\r\n", doc->feed->host);
   ecore_con_server_send(doc->server.conn, buf, strlen(buf));
   snprintf(buf, sizeof(buf), "User-Agent: %s/v%d\r\n\r\n", "Enlightenment News module", news->config->version);
   ecore_con_server_send(doc->server.conn, buf, strlen(buf));

   doc->server.waiting_reply = 1;

   return EINA_TRUE;
}

static Eina_Bool
_cb_feed_server_del(void *data, int type, void *event)
{
   News_Feed_Document *doc;
   Ecore_Con_Event_Server_Del *ev;

   ev = event;
   doc = data;

   /* check if the event is our event */
   if (doc->server.conn != ev->server)
     return EINA_TRUE;

   doc->server.conn = NULL;

   DFEED(("Connection end"));

   if (!doc->server.buffer || !doc->server.buffer_size)
     {
        // TODO: error popup
        if (doc->feed->item)
          news_item_loadingstate_refresh(doc->feed->item);
        return EINA_TRUE;
     }

   doc->server.buffer_size++;
   doc->server.buffer = realloc(doc->server.buffer,
				doc->server.buffer_size);
   doc->server.buffer[doc->server.buffer_size - 1] = '\0';

   doc->server.waiting_reply = 0;

   news_parse_go(doc, _cb_feed_parse);

   return EINA_TRUE;
}

static Eina_Bool
_cb_feed_server_data(void *data, int type, void *event)
{
   News_Feed_Document *doc;
   Ecore_Con_Event_Server_Data *ev;

   ev = event;
   doc = data;

   DFEED(("Received %d octets !", ev->size));

   /* check if the event is our event */
   if (doc->server.conn != ev->server)
     return EINA_TRUE;

   doc->server.buffer = realloc(doc->server.buffer,
 				doc->server.buffer_size + ev->size);
   memcpy(doc->server.buffer + doc->server.buffer_size, ev->data, ev->size);
   doc->server.buffer_size += ev->size;

   return EINA_TRUE;
}

static void
_cb_feed_parse(News_Feed_Document *doc, News_Parse_Error error, int changes)
{
   News_Item *ni;

   ni = doc->feed->item;

   free(doc->server.buffer);
   doc->server.nb_tries = 0;
   doc->server.buffer = NULL;
   doc->server.buffer_size = 0;
   doc->parse.last_time = ecore_time_unix_get();

   //TODO with popups
   doc->parse.error = error;
   switch(error)
     {
     case NEWS_PARSE_ERROR_NO:
        break;
     case NEWS_PARSE_ERROR_BROKEN_FEED:
        break;
     case NEWS_PARSE_ERROR_TYPE_UNKNOWN:
        break;
     case NEWS_PARSE_ERROR_NOT_IMPLEMENTED:
        break;
     }

   if (changes)
     {
        //TODO: popup

        doc->ui_needrefresh = 1;

        if (ni)
          {
             if (ni->viewer)
               news_viewer_refresh(ni->viewer);
          }
     }
   else
     {
        if (ni && ni->viewer &&
            (ni->viewer->vfeeds.selected == doc->feed))
          news_viewer_feed_selected_infos_refresh(ni->viewer);
     }
}

static Eina_Bool
_cb_feeds_timer(void *data)
{
   NEWS_FEED_FOREACH_BEG();
   if (_feed->doc)
     news_feed_update(_feed);
   NEWS_FEED_FOREACH_END();

   return EINA_TRUE;
}

static int
_cb_sort_cats(const void *d1, const void *d2)
{
   const News_Feed_Category *c1, *c2;

   c1 = d1;
   c2 = d2;

   return strcmp(c1->name, c2->name);
}

static int
_cb_sort_feeds(const void *d1, const void *d2)
{
   const News_Feed *f1, *f2;

   f1 = d1;
   f2 = d2;

   return strcmp(f1->name, f2->name);
}
