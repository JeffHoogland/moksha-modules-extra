#include "News.h"

#define VIEWER_CREATE_FAIL(nv, msg)   \
{                                     \
   news_util_message_error_show(msg); \
   news_viewer_destroy(nv);           \
   return 0;                          \
}

static int   _dialog_create(News_Viewer *nv);
static void  _dialog_destroy(News_Viewer *nv);
static void  _dialog_geometry_update(News_Viewer *nv);
static int   _dialog_content_create(News_Viewer *nv);
static void  _dialog_content_destroy(News_Viewer *nv);
static void  _dialog_cb_feed_selected(void *data);
static void  _dialog_cb_article_selected(void *data);
static void  _dialog_cb_del(void *obj);
static void  _dialog_cb_key_down(void *data, Evas *e, Evas_Object *obj, void *event);

static void  _vfeeds_buttons_state_refresh(News_Viewer *nv);
static void  _vfeeds_cb_button_feed(void *data, void *data2);
static void  _vfeeds_cb_button_refresh(void *data, void *data2);
static void  _vfeeds_cb_button_setasread(void *data, void *data2);
static void  _varticles_refresh(News_Viewer *nv);
static void  _vcontent_text_set(News_Viewer *nv, char *text);
static void  _vcontent_feed_infos_set(News_Viewer *nv);
static void  _vcontent_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);

static Evas_Object *_article_icon_get(News_Feed_Article *art, Evas *evas);
static Eina_List   *_sort_feedrefs_unreadfirst_list_get(News_Item *ni);
static Eina_List   *_sort_articles_unreadfirst_list_get(News_Feed *f);
static Eina_List   *_sort_articles_unreadfirst(Eina_List *articles);
static Eina_List   *_sort_articles_date_list_get(News_Feed *f);
static int          _sort_articles_date_list_cb(const void *d1, const void *d2);

static Eina_List *_viewers;


/*
 * Public functions
 */

int
news_viewer_init(void)
{
   _viewers = NULL;

   return 1;
}

void
news_viewer_shutdown(void)
{
   News_Viewer *nv;

   while (_viewers)
     {
        nv = _viewers->data;
        news_viewer_destroy(nv);
        _viewers = eina_list_remove_list(_viewers, _viewers);
     }
}

void
news_viewer_all_refresh(int force, int recreate)
{
   News_Viewer *nv;
   int pos, count, counter;

   pos = 0;
   counter = 0;
   count = eina_list_count(_viewers);
   while (counter < count)
     {
        nv = eina_list_nth(_viewers, pos);
        if (force)
          {
             NEWS_ITEM_FEEDS_FOREACH_BEG(nv->item);
             if (!_feed->doc) continue;
             _feed->doc->ui_needrefresh = 1;
             NEWS_ITEM_FEEDS_FOREACH_END();
          }
        if (recreate)
          {
             News_Item *ni;
             
             ni = nv->item;
             news_viewer_destroy(nv);
             news_viewer_create(ni);
          }
        else
          {
             news_viewer_refresh(nv);
             pos++;
          }
        counter++;
     }
}

int
news_viewer_create(News_Item *ni)
{
   News_Viewer *nv;

   if (ni->viewer) return 0;

   nv = E_NEW(News_Viewer, 1);
   nv->item = ni;

   if (!_dialog_create(nv))
     VIEWER_CREATE_FAIL(nv, "Could not create viewer dialog");
   if (!_dialog_content_create(nv))
     VIEWER_CREATE_FAIL(nv, "Could not create viewer dialog content");
   _dialog_geometry_update(nv);
   e_win_raise(nv->dialog.dia->win);

   ni->viewer = nv;
   _viewers = eina_list_append(_viewers, nv);

   news_viewer_refresh(nv);

   return 1;
}

void
news_viewer_destroy(News_Viewer *nv)
{
   _dialog_content_destroy(nv);
   _dialog_destroy(nv);

   nv->item->viewer = NULL;
   _viewers = eina_list_remove(_viewers, nv);
   free(nv);
}

void
news_viewer_refresh(News_Viewer *nv)
{
   Evas_Object *ilist;
   Eina_List *feed_refs;
   int feed_refs_own = 0;
   int pos, toselect_pos;

   if (!nv) return;

   ilist = nv->vfeeds.ilist;
   e_widget_ilist_freeze(ilist);
   e_widget_ilist_clear(ilist);
   if (!eina_list_count(nv->item->config->feed_refs))
     {
        nv->vfeeds.selected = NULL;
        nv->varticles.selected = NULL;
        e_widget_button_icon_set(nv->vfeeds.button_feed, NULL);
	if (nv->varticles.ilist)
	  e_widget_ilist_clear(nv->varticles.ilist);
        _vcontent_text_set(nv, "");
     }

   if (news->config->viewer.vfeeds.unread_first)
     {
        feed_refs = _sort_feedrefs_unreadfirst_list_get(nv->item);
        feed_refs_own = 1;
     }
   else
     feed_refs = nv->item->config->feed_refs;

   DD(("feed_refs_own = %d", feed_refs_own));

   /* refresh the list of feeds */
   pos = 0;
   toselect_pos = -1;
   NEWS_ITEM_FEEDS_FOREACH_BEG_LIST(feed_refs);
   {
      Evas_Object *ic = NULL;
      char buf[4096];
      
      if (!_feed->doc) continue;
      DD(("UI NEED REFRESH = %d (%s)",
          _feed->doc->ui_needrefresh, _feed->name));

      /* add the ui item in  the feeds list */
      if (_feed->icon && _feed->icon[0])
        {
           ic = e_icon_add(evas_object_evas_get(ilist));
           e_icon_file_set(ic, _feed->icon);
        }
      if (_feed->doc && _feed->doc->unread_count)
        snprintf(buf, sizeof(buf), "[UNREAD] %s", _feed->name);
      else
	snprintf(buf, sizeof(buf), "%s", _feed->name);
      e_widget_ilist_append(ilist, ic, buf, _dialog_cb_feed_selected, _feed, NULL);
      if (nv->vfeeds.selected == _feed)
        toselect_pos = pos;

      pos++;
   }
   NEWS_ITEM_FEEDS_FOREACH_END();
   e_widget_ilist_go(ilist);
   e_widget_ilist_thaw(ilist);

   /* select a feed */
   if (toselect_pos != -1)
     e_widget_ilist_selected_set(ilist, toselect_pos);

   /* ilist size */
   if (pos == -1)
     e_widget_size_min_set(ilist, 100, 70);
   else
     {
	int wmw;
	e_widget_size_min_get(ilist, &wmw, NULL);
	e_widget_size_min_set(ilist, wmw, 110);
     }
   
   if (nv->vfeeds.list_own)
     {
        if (nv->vfeeds.list)
          {
             DD(("LIST OWN free !!"));
             eina_list_free(nv->vfeeds.list);
             nv->vfeeds.list = NULL;
          }
        nv->vfeeds.list_own = 0;
     }
   nv->vfeeds.list = feed_refs;
   nv->vfeeds.list_own = feed_refs_own;

   _vfeeds_buttons_state_refresh(nv);
}

int
news_viewer_feed_select(News_Viewer *nv, News_Feed *feed)
{
   int pos;
   int found;

   if (nv->vfeeds.selected == feed) return 1;

   pos = 0;
   found = 0;
   NEWS_ITEM_FEEDS_FOREACH_BEG_LIST(nv->vfeeds.list);
   if (feed == _feed)
     {
        found = 1;
        break;
     }
   pos++;
   NEWS_ITEM_FEEDS_FOREACH_END();
   if (!found) return 0;

   e_widget_ilist_selected_set(nv->vfeeds.ilist, pos);
   _dialog_cb_feed_selected(feed);

   return 1;
}

void
news_viewer_feed_selected_infos_refresh(News_Viewer *nv)
{
   if (!nv->varticles.selected)
     _vcontent_feed_infos_set(nv);
}

void
news_viewer_article_state_refresh(News_Viewer *nv, News_Feed_Article *art)
{
   Eina_List *l;
   News_Feed *f;
   News_Feed_Article *a;
   int pos;

   f = art->doc->feed;
   if (nv->vfeeds.selected != f) return;

   pos = 0;
   for (l=nv->varticles.list; l; l=eina_list_next(l))
     {
        a = l->data;
        if (art == a) break;
        pos++;
     }
   if (!l) return;
   
   e_widget_ilist_nth_icon_set(nv->varticles.ilist, pos,
                               _article_icon_get(art,evas_object_evas_get(nv->varticles.ilist)));
}

/*
 * Private functions
 */

static int
_dialog_create(News_Viewer *nv)
{
   Evas_Modifier_Mask mask;
   E_Dialog    *dia;
   char buf[4096];

   snprintf(buf, sizeof(buf), "_e_mod_news_viewer_%d", nv->item->gcc->id);
   dia = e_dialog_new(e_container_current_get(e_manager_current_get()),
                      "E", buf);
   if (!dia) return 0;

   dia->data = nv;
   e_object_del_attach_func_set(E_OBJECT(dia), _dialog_cb_del);

   e_dialog_title_set(dia, D_("News Viewer"));
   e_dialog_resizable_set(dia, 1);
   e_win_sticky_set(dia->win, 1);
   //e_win_layer_set(dia->win, ECORE_X_WINDOW_LAYER_ABOVE);

   mask = 0;
   evas_object_key_grab(dia->event_object, "Escape", mask, ~mask, 0);
   evas_object_event_callback_add(dia->event_object, EVAS_CALLBACK_KEY_DOWN, _dialog_cb_key_down, nv);

   nv->dialog.dia = dia;

   return 1;
}

static void
_dialog_destroy(News_Viewer *nv)
{
   if (nv->dialog.dia)
     {
        nv->dialog.dia->data = NULL;
        e_object_del(E_OBJECT(nv->dialog.dia));
        nv->dialog.dia = NULL;
     }
}

static void
_dialog_geometry_update(News_Viewer *nv)
{
   int dia_x, dia_y;
   int tab_w, tab_h;
   int canvas_w, canvas_h;

   if (!nv->dialog.dia) return;

   /* dialog position : compute dia_x and dia_y */
   e_widget_size_min_get(nv->dialog.tab, &tab_w, &tab_h);
   news_util_ecanvas_geometry_get(&canvas_w, &canvas_h);
   dia_x = (canvas_w - tab_w) / 2;
   dia_y = (canvas_h - tab_h) / 2;
   DVIEWER(("diax: %d diay : %d", dia_x, dia_y));

   /* apply ! */
   e_win_move(nv->dialog.dia->win, dia_x, dia_y);
   nv->dialog.x = dia_x;
   nv->dialog.y = dia_y;
   e_dialog_show(nv->dialog.dia);
   e_dialog_border_icon_set(nv->dialog.dia, news_theme_file_get(NEWS_THEME_CAT_ICON));
}

static int
_dialog_content_create(News_Viewer *nv)
{
   Evas_Textblock_Style *tb_style;
   Evas *evas;
   Evas_Object *o, *o2, *ob, *of, *icon;
   char buf[4096];
   char buf2[1024] = "";
   int w, h;

   evas = evas_object_evas_get(nv->dialog.dia->bg_object);

   o = e_widget_list_add(evas, 0, 0);
   o2 = e_widget_list_add(evas, 0, 1);
   nv->dialog.tab = o;

   of = e_widget_frametable_add(evas, D_("Feeds in this gadget"), 0);
   nv->dialog.ftab_feeds = of;

   ob = e_widget_button_add(evas, "", NULL, _vfeeds_cb_button_feed, nv, NULL);
   e_widget_size_min_set(ob, 60, 60);
   nv->vfeeds.button_feed = ob;
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 2, 0, 0, 0, 0);

   ob = e_widget_button_add(evas, D_("Update"), "", _vfeeds_cb_button_refresh, nv, NULL);
   icon = e_icon_add(evas);
   news_theme_icon_set(icon, NEWS_THEME_ICON_UPDATE);
   e_icon_fill_inside_set(icon, 1);
   e_widget_button_icon_set(ob, icon);
   nv->vfeeds.button_refresh = ob;
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 0, 0, 0, 0);

   ob = e_widget_button_add(evas, D_("Set as read"), "", _vfeeds_cb_button_setasread, nv, NULL);
   icon = e_icon_add(evas);
   news_theme_icon_set(icon, NEWS_THEME_ICON_SETASREAD);
   e_icon_fill_inside_set(icon, 1);
   e_widget_button_icon_set(ob, icon);
   nv->vfeeds.button_setasread = ob;
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 0, 0, 0, 0);

   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_selector_set(ob, 1);
   nv->vfeeds.ilist = ob;
   news_viewer_refresh(nv);
   e_widget_frametable_object_append(of, ob, 0, 2, 2, 1, 1, 1, 1, 1);

   e_widget_list_object_append(o2, of, 1, 0, 0.0);

  ob = evas_object_textblock_add(evas_object_evas_get(nv->dialog.dia->bg_object));
   tb_style = evas_textblock_style_new();
   if (news->config->viewer.vcontent.font_shadow)
     {
        snprintf(buf2, sizeof(buf2), " style=soft_shadow shadow_color=%s",
                 news->config->viewer.vcontent.font_shadow_color);
     }
   snprintf(buf, sizeof(buf),
            "DEFAULT='font=Vera font_size=%d align=left color=%s%s wrap=word'"
            "br='\n'"
            "hilight='+ font=Vera-Bold font_size=%d'"
            "small='+ font_size=%d'"
            "italic='+ font=Vera-Italic'",
            news->config->viewer.vcontent.font_size,
            news->config->viewer.vcontent.font_color,
            buf2,
            news->config->viewer.vcontent.font_size + 1,
            news->config->viewer.vcontent.font_size - 1);
   evas_textblock_style_set(tb_style, buf);
   evas_object_textblock_style_set(ob, tb_style);
   evas_textblock_style_free(tb_style);
   evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN,
                                  _vcontent_cb_mouse_down, nv);
   nv->vcontent.tb = ob;

   ob = e_widget_scrollframe_simple_add(evas, nv->vcontent.tb);
   e_widget_size_min_set(ob, 290, 200);
   evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN,
                                  _vcontent_cb_mouse_down, nv);
   nv->vcontent.scrollframe = ob;

   e_widget_list_object_append(o2, ob, 1, 1, 0.5);
   e_widget_list_object_append(o, o2, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, D_("Articles in selected feed"), 0);
   nv->dialog.ftab_articles = of;

   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_selector_set(ob, 1);
   e_widget_size_min_set(ob, 250, 140);
   nv->varticles.ilist = ob;
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o, of, 1, 1, 1.0);
 
   /* apply */
   e_widget_size_min_get(o, &w, &h);
   e_dialog_content_set(nv->dialog.dia, o, w, h);

   return 1;
}

static void
_dialog_content_destroy(News_Viewer *nv)
{
   if (nv->dialog.tab)
     {
        evas_object_del(nv->dialog.tab);
        nv->dialog.tab = NULL;
     }

   if (nv->dialog.ftab_feeds)
     {
        evas_object_del(nv->dialog.ftab_feeds);
        nv->dialog.ftab_feeds = NULL;
     }
   if (nv->dialog.ftab_articles)
     {
        evas_object_del(nv->dialog.ftab_articles);
        nv->dialog.ftab_articles = NULL;
     }
   if (nv->vfeeds.list_own && nv->vfeeds.list)
     {
        eina_list_free(nv->vfeeds.list);
        nv->vfeeds.list = NULL;
     }
   if (nv->vfeeds.ilist)
     {
        evas_object_del(nv->vfeeds.ilist);
        nv->vfeeds.ilist = NULL;
        nv->vfeeds.selected = NULL;
     }
   if (nv->vfeeds.button_feed)
     {
        evas_object_del(nv->vfeeds.button_feed);
        nv->vfeeds.button_feed = NULL;
     }
   if (nv->vfeeds.button_feed_icon)
     {
        evas_object_del(nv->vfeeds.button_feed_icon);
        nv->vfeeds.button_feed_icon = NULL;
     }
   if (nv->vfeeds.button_refresh)
     {
        evas_object_del(nv->vfeeds.button_refresh);
        nv->vfeeds.button_refresh = NULL;
     }
   if (nv->vfeeds.button_setasread)
     {
        evas_object_del(nv->vfeeds.button_setasread);
        nv->vfeeds.button_setasread = NULL;
     }

   if (nv->varticles.list_own && nv->varticles.list)
     {
        eina_list_free(nv->varticles.list);
        nv->varticles.list = NULL;
        nv->varticles.list_own = 0;
     }
   if (nv->varticles.ilist)
     {
        evas_object_del(nv->varticles.ilist);
        nv->varticles.ilist = NULL;
        nv->varticles.selected = NULL;
     }

   if (nv->vcontent.tb)
     {
        evas_object_del(nv->vcontent.tb);
        nv->vcontent.tb = NULL;
     }
   if (nv->vcontent.scrollframe)
     {
        evas_object_del(nv->vcontent.scrollframe);
        nv->vcontent.scrollframe = NULL;
     }
}

static void
_dialog_cb_feed_selected(void *data)
{
   News_Viewer *nv;
   News_Feed *feed;
   int changed = 0;

   feed = data;
   if (!feed) return;
   nv = feed->item->viewer;
   DD(("TRIGGERED cb feed ilist (%s)", feed->name));

   if ((nv->vfeeds.selected == feed) &&
       !feed->doc->ui_needrefresh)
     {
        /* show feed infos in vcontent */
        _vcontent_feed_infos_set(nv);
        _vfeeds_buttons_state_refresh(nv);
        if (nv->varticles.selected)
          {
             e_widget_ilist_unselect(nv->varticles.ilist);
             nv->varticles.selected = NULL;
          }
        return;
     }

   if (nv->vfeeds.selected != feed)
     {
        changed = 1;
        nv->vfeeds.selected = feed;
     }
   if (feed->icon && feed->icon[0])
     {
        Evas_Object *ic;

        ic = e_icon_add(evas_object_evas_get(nv->vfeeds.ilist));
        e_icon_file_set(ic, feed->icon);
        e_icon_fill_inside_set(ic, 1);
        if (nv->vfeeds.button_feed_icon)
          evas_object_del(nv->vfeeds.button_feed_icon);
        e_widget_button_icon_set(nv->vfeeds.button_feed, ic);
        nv->vfeeds.button_feed_icon = ic;
     }
   _vfeeds_buttons_state_refresh(nv);

   if (changed)
     nv->varticles.selected = NULL;
   _varticles_refresh(nv);
}

static void
_dialog_cb_article_selected(void *data)
{
   News_Viewer *nv;
   News_Feed_Article *art;
   char buf[4096];
   char buf_date[4096] = "Not dated";

   art = data;
   if (!art) return;
   nv = art->doc->feed->item->viewer;
   DD(("TRIGGERED cb article ilist (%s)", art->title));
   nv->varticles.selected = art;

   if (art->date.tm_year != 0)
     strftime(buf_date, sizeof(buf_date), "%Y-%m-%d %H:%M:%S", &art->date);
   snprintf(buf, sizeof(buf),
            "<hilight>%s</hilight><br>"
            "<small>%s</small><br><br>"
            "%s<br><br>"
            "<small>%s</small>",
            (art->title && art->title[0]) ? art->title : "No title",
            buf_date,
            (art->description && art->description[0])? art->description : "No description text",
            (art->url) ? "||click on the text to open article in a browser||" : "");
   _vcontent_text_set(nv, buf);

   if (art->unread)
     news_feed_article_unread_set(art, 0);
}

static void
_dialog_cb_del(void *obj)
{
   E_Dialog *dia;
   News_Viewer *nv;

   dia = obj;
   nv = dia->data;
   if (!nv) return;

   nv->dialog.dia = NULL;
   news_viewer_destroy(nv);
}

static void
_dialog_cb_key_down(void *data, Evas *e, Evas_Object *obj, void *event)
{
   Evas_Event_Key_Down *ev;
   News_Viewer *nv;

   ev = event;
   nv = data;
   if (!strcmp(ev->keyname, "Escape"))
     news_viewer_destroy(nv);
}

static void
_vfeeds_buttons_state_refresh(News_Viewer *nv)
{
   News_Feed *f;

   f = nv->vfeeds.selected;
   if (nv->vfeeds.button_feed)
     {
        if (f && f->url_home && f->url_home[0])
          e_widget_disabled_set(nv->vfeeds.button_feed, 0);
        else
          e_widget_disabled_set(nv->vfeeds.button_feed, 1);
     }
   if (nv->vfeeds.button_refresh)
     {
        if (f)
          e_widget_disabled_set(nv->vfeeds.button_refresh, 0);
        else
          e_widget_disabled_set(nv->vfeeds.button_refresh, 1);
     }
   if (nv->vfeeds.button_setasread)
     {
        if (f && f->doc && f->doc->unread_count)
          e_widget_disabled_set(nv->vfeeds.button_setasread, 0);
        else
          e_widget_disabled_set(nv->vfeeds.button_setasread, 1);
     }
}

static void
_vfeeds_cb_button_feed(void *data, void *data2)
{
   News_Viewer *nv;

   nv = data;
   if (!nv->vfeeds.selected) return;
   if (nv->vfeeds.selected->config_dialog) return;

   news_util_browser_open(nv->vfeeds.selected->url_home);
}

static void
_vfeeds_cb_button_refresh(void *data, void *data2)
{
   News_Viewer *nv;
   News_Feed *f;

   nv = data;
   f = nv->vfeeds.selected;
   if (!f || !f->doc) return;

   news_feed_update(f);
}

static void
_vfeeds_cb_button_setasread(void *data, void *data2)
{
   News_Viewer *nv;
   News_Feed *f;

   nv = data;
   f = nv->vfeeds.selected;
   if (!f || !f->doc) return;

   news_feed_read_set(f);
}


static void
_varticles_refresh(News_Viewer *nv)
{
   News_Feed *feed;
   Evas_Object *ilist;
   Eina_List *articles, *l;
   int articles_own = 0;
   int pos, toselect_pos;

   ilist = nv->varticles.ilist;
   e_widget_ilist_clear(ilist);

   feed = nv->vfeeds.selected;
   if (!feed || !feed->doc) return;
   e_widget_ilist_freeze(ilist);

   if (news->config->viewer.varticles.unread_first)
     {
        if (news->config->viewer.varticles.sort_date)
          {
             Eina_List *tmp;

             tmp = _sort_articles_date_list_get(feed);
             articles = _sort_articles_unreadfirst(tmp);
             eina_list_free(tmp);
          }
        else
          articles = _sort_articles_unreadfirst_list_get(feed);
        articles_own = 1;
     }
   else if (news->config->viewer.varticles.sort_date)
     {
        articles = _sort_articles_date_list_get(feed);
        articles_own = 1;
     }
   else articles = feed->doc->articles;

   pos = 0;
   toselect_pos = -1;
   if (eina_list_count(articles))
     {
        for (l=articles; l; l=eina_list_next(l))
          {
             News_Feed_Article *art;
             char label[4096];
             char buf_date[4096] = "";
             
             art = l->data;
             /* append the article to the article ilist */
             if (art->date.tm_year)
               strftime(buf_date, sizeof(buf_date), "%d %H:%M", &art->date);
             snprintf(label, sizeof(label), "%s %s", buf_date, art->title);
             e_widget_ilist_append(ilist,
                                   _article_icon_get(art, evas_object_evas_get(ilist)), label,
                                   _dialog_cb_article_selected, art, NULL);
             if (nv->varticles.selected == art)
               toselect_pos = pos;

             pos++;
          }
     }
   e_widget_ilist_go(ilist);
   e_widget_ilist_thaw(ilist);

   /* select a feed,
      of if no articles are selected
      show feed informations in vcontent */
   if (toselect_pos != -1)
     {
        e_widget_ilist_selected_set(ilist, toselect_pos);
        _dialog_cb_article_selected(nv->varticles.selected);
     }
   else if (!nv->varticles.selected)
     _vcontent_feed_infos_set(nv);

   if (nv->varticles.list_own)
     {
        eina_list_free(nv->varticles.list);
        nv->varticles.list = NULL;
        nv->varticles.list_own = 0;
     }
   nv->varticles.list = articles;
   nv->varticles.list_own = articles_own;

   feed->doc->ui_needrefresh = 0;
}

static void
_vcontent_text_set(News_Viewer *nv, char *text)
{
   int w, h, sw, sh;

   evas_object_geometry_get(nv->vcontent.scrollframe, NULL, NULL, &sw, &sh);
   evas_object_resize(nv->vcontent.tb, sw-20, sh-20);

   evas_object_textblock_clear(nv->vcontent.tb);
   if (text)
     evas_object_textblock_text_markup_set(nv->vcontent.tb, text);

   evas_object_textblock_size_formatted_get(nv->vcontent.tb, &w, &h);
   if (h < sh - 20) h = sh - 20;
   if (w > sw - 20) w = sw - 20;
   evas_object_resize(nv->vcontent.tb, w, h);
}

static void
_vcontent_feed_infos_set(News_Viewer *nv)
{
   News_Feed *f;
   char buf_error[1024] = "";
   char buf_infos[1024] = "";
   char buf_conn[200] = "";
   char buf_unread[1024] = "No unread articles";
   char buf[4096];

   evas_object_textblock_clear(nv->vcontent.tb);
   f = nv->vfeeds.selected;
   if (!f) return;

   if (f->doc)
     {
        switch(f->doc->parse.error)
          {
	   case NEWS_PARSE_ERROR_NO:
	   case NEWS_PARSE_ERROR_NOT_IMPLEMENTED:
             break;
	   case NEWS_PARSE_ERROR_BROKEN_FEED:
	   case NEWS_PARSE_ERROR_TYPE_UNKNOWN:
             snprintf(buf_error, sizeof(buf_error),
                      "<br><color=#ff0000>An error happend during the parse of this feed !<br>"
                      "You can report error at ooookiwi@gmail.com to get it fixed</><br><br>");
             break;
          }
        if (f->doc->parse.last_time)
          {
             char buf_mtime[200];
             char *type;

             switch (f->doc->parse.type)
               {
		case NEWS_FEED_TYPE_RSS:
                  type = "RSS"; break;
		case NEWS_FEED_TYPE_ATOM:
                  type = "ATOM <color=#ff0000>(not supported for now)</>"; break;
		default:
                  type = "UNKNOWN";
               }
             snprintf(buf_mtime, sizeof(buf_mtime),
                      "Last check of the feed : %s",
                      ctime(&f->doc->parse.last_time));
             buf_mtime[strlen(buf_mtime)-1] = '\0';
             snprintf(buf_infos, sizeof(buf_infos),
                      "%s<br>Type: %s, Version %.2f<br>",
                      buf_mtime, type, f->doc->parse.version);
          }
        if (f->doc->server.conn)
          {
             if (f->doc->server.nb_tries > 1)
               snprintf(buf_conn, sizeof(buf_conn),
                        "<color=#ff0000>Trying to update the feed ... (%d)</><br>",
                        f->doc->server.nb_tries);
             else
               snprintf(buf_conn, sizeof(buf_conn),
                        "<color=#ff0000>Trying to update the feed ...</><br>");
          }
        if (f->doc->unread_count)
          {
             snprintf(buf_unread, sizeof(buf_unread), "<color=#ff0000>%d unread articles</>%s",
                      f->doc->unread_count,
                      (f->doc->unread_count >= NEWS_FEED_UNREAD_COUNT_MAX) ? "<br>"
                      "<color=#ff0000>Its the maximum number of articles a feed can have.<br>"
                      "The oldest articles you didn't red are not keeped anymore.</>"
                      : "");
          }
     }
   snprintf(buf, sizeof(buf),
            "<hilight>%s</hilight> <small>in %s</small><br><br>"
            "%s"
            "%s<br><br>"
            "<italic>%s%s<br>"
            "%s</italic>",
            f->name,
            f->category->name,
            buf_error,
            (f->description && f->description[0]) ? f->description : "No description for this feed",
            buf_infos, buf_conn,
            buf_unread);
   _vcontent_text_set(nv, buf);
}

static void
_vcontent_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   News_Viewer *nv;
   Evas_Event_Mouse_Down *ev;

   nv = data;
   ev = event_info;

   if (nv->varticles.selected)
     {
        if (nv->varticles.selected->url)
          news_util_browser_open(nv->varticles.selected->url);
     }
   else if (nv->vfeeds.selected)
     {
        if (nv->vfeeds.selected->url_home)
          news_util_browser_open(nv->vfeeds.selected->url_home);
     }
}

static Evas_Object *
_article_icon_get(News_Feed_Article *art, Evas *evas)
{
   Evas_Object *ic;

   ic = e_icon_add(evas);
   e_icon_fill_inside_set(ic, 1);
   if (art->unread)
     news_theme_icon_set(ic, NEWS_THEME_ICON_ARTICLE_UNREAD);
   else
     news_theme_icon_set(ic, NEWS_THEME_ICON_ARTICLE_READ);

   return ic;
}

static Eina_List *
_sort_feedrefs_unreadfirst_list_get(News_Item *ni)
{
   Eina_List *list, *reads, *l;
   News_Feed_Ref *ref;

   list = NULL;
   reads = NULL;
   NEWS_ITEM_FEEDS_FOREACH_BEG(ni);
   if (_feed->doc && _feed->doc->unread_count)
     list = eina_list_append(list, _ref);
   else
     reads = eina_list_append(reads, _ref);
   NEWS_ITEM_FEEDS_FOREACH_END();

   for (l=reads; l; l=eina_list_next(l))
     {
        ref = l->data;
        list = eina_list_append(list, ref);
     }
   eina_list_free(reads);

   return list;
}

static Eina_List *
_sort_articles_unreadfirst_list_get(News_Feed *f)
{
   Eina_List *list;

   if (!f->doc) return NULL;
   list = _sort_articles_unreadfirst(f->doc->articles);

   return list;
}

static Eina_List *
_sort_articles_unreadfirst(Eina_List *articles)
{
   Eina_List *list, *reads, *l;
   News_Feed_Article *art;

   list = NULL;
   reads = NULL;
   for (l=articles; l; l=eina_list_next(l))
     {
        art = l->data;
        
        if (art->unread)
          list = eina_list_append(list, art);
        else
          reads = eina_list_append(reads, art);
     }

   for (l=reads; l; l=eina_list_next(l))
     {
        art = l->data;
        list = eina_list_append(list, art);
     }
   eina_list_free(reads);

   return list;
}

static Eina_List *
_sort_articles_date_list_get(News_Feed *f)
{
   Eina_List *list, *l;
   News_Feed_Article *art;

   if (!f->doc) return NULL;

   list = NULL;
   for (l=f->doc->articles; l; l=eina_list_next(l))
     {
        art = l->data;
        list = eina_list_append(list, art);
     }
   list = eina_list_sort(list, eina_list_count(list), _sort_articles_date_list_cb);

   return list;
}

static int
_sort_articles_date_list_cb(const void *d1, const void *d2)
{
   const News_Feed_Article *a1, *a2;

   a1 = d1;
   a2 = d2;

   /* sort reverse order : more recent (greater) first */
   return - (news_util_datecmp(&a1->date, &a2->date));
}
