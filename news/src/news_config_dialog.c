#include "News.h"

#define DIALOG_CLASS "_e_modules_news_config_dialog"

struct _E_Config_Dialog_Data
{
   Evas_Object *color_viewer_font;
   Evas_Object *color_viewer_font_shadow;

   struct
   {
      int timer_m;
      int sort_name;
   } feed;

   struct
   {
      int enable;
      char *host;
      char *port;
   } proxy;

   struct
   {
      struct
      {
         int unread_first;
      } vfeeds;
      struct
      {
         int unread_first;
         int sort_date;
      } varticles;
      struct
      {
         int      font_size;
         E_Color *font_color;
         int      font_shadow;
         E_Color *font_shadow_color;
         int      color_changed;
      } vcontent;
   } viewer;

   struct
   {
      int active;
      int timer_s;
   } popup_news;

   struct
   {
      int on_timeout;
      int timer_s;
   } popup_other;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void         _cb_color_viewer_font_change(void *data, Evas_Object *obj);
static void         _cb_color_viewer_font_shadow_change(void *data, Evas_Object *obj);

/*
 * Public functions
 */

int
news_config_dialog_show(void)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("News", DIALOG_CLASS))
     return 0;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _advanced_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;
   
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("News Main Configuration"),
                             "News", DIALOG_CLASS,
                             news_theme_file_get(NEWS_THEME_CAT_ICON), 0, v, NULL);

   return 1;
}

void
news_config_dialog_hide(void)
{
   e_object_del(E_OBJECT(news->config_dialog));
   news->config_dialog = NULL;
}

/*
 * Private functions
 *
 */

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;

   news->config_dialog = cfd;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   cfd->cfdata = cfdata;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   free(cfdata->proxy.host);

   news->config_dialog = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   News_Config *c;

   c = news->config;

   cfdata->feed.timer_m = c->feed.timer_m;
   cfdata->feed.sort_name = c->feed.sort_name;

   cfdata->proxy.enable = c->proxy.enable;
   if (c->proxy.host)
     cfdata->proxy.host = strdup(c->proxy.host);
   else
     cfdata->proxy.host = strdup("");
   if (c->proxy.port)
     {
        char buf[6];
        snprintf(buf, sizeof(buf), "%d", c->proxy.port);
        cfdata->proxy.port = strdup(buf);
     }
   else
     cfdata->proxy.port = strdup("");

   cfdata->viewer.vfeeds.unread_first = c->viewer.vfeeds.unread_first;
   cfdata->viewer.varticles.unread_first = c->viewer.varticles.unread_first;
   cfdata->viewer.varticles.sort_date = c->viewer.varticles.sort_date;
   cfdata->viewer.vcontent.font_size = c->viewer.vcontent.font_size;
   cfdata->viewer.vcontent.font_color = E_NEW(E_Color, 1);
   sscanf(c->viewer.vcontent.font_color ? : NEWS_VIEWER_VCONTENT_FONT_COLOR_DEFAULT,
          "#%2x%2x%2x",
          &cfdata->viewer.vcontent.font_color->r,
          &cfdata->viewer.vcontent.font_color->g,
          &cfdata->viewer.vcontent.font_color->b);
   cfdata->viewer.vcontent.font_color->a = 255;
   e_color_update_rgb(cfdata->viewer.vcontent.font_color);
   cfdata->viewer.vcontent.font_shadow = c->viewer.vcontent.font_shadow;
   cfdata->viewer.vcontent.font_shadow_color = E_NEW(E_Color, 1);
   sscanf(c->viewer.vcontent.font_shadow_color ? : NEWS_VIEWER_VCONTENT_FONT_SHADOW_COLOR_DEFAULT,
          "#%2x%2x%2x",
          &cfdata->viewer.vcontent.font_shadow_color->r,
          &cfdata->viewer.vcontent.font_shadow_color->g,
          &cfdata->viewer.vcontent.font_shadow_color->b);
   cfdata->viewer.vcontent.font_shadow_color->a = 255;
   e_color_update_rgb(cfdata->viewer.vcontent.font_shadow_color);
   cfdata->viewer.vcontent.color_changed = 0;

   cfdata->popup_news.active = c->popup_news.active;
   cfdata->popup_news.timer_s = c->popup_news.timer_s;

   cfdata->popup_other.on_timeout = c->popup_other.on_timeout;
   cfdata->popup_other.timer_s = c->popup_other.timer_s;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ob;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_frametable_add(evas, D_("Feeds"), 0);

   ob = e_widget_check_add(evas, D_("Sort lists by name (disable Move action)"), &(cfdata->feed.sort_name));
   e_widget_frametable_object_append(of, ob, 0, 1, 2, 1, 1, 1, 1, 0);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, D_("News Viewer"), 0);

   ob = e_widget_label_add(evas, D_("Font size"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);

   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f"), NEWS_VIEWER_VCONTENT_FONT_SIZE_MIN, NEWS_VIEWER_VCONTENT_FONT_SIZE_MAX, 1.0, 0, NULL, &(cfdata->viewer.vcontent.font_size), 50);
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 0, 1, 0);

   ob = e_widget_label_add(evas, D_("Font color"));
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);

   ob = e_widget_color_well_add(evas, cfdata->viewer.vcontent.font_color, 1);
   cfdata->color_viewer_font = ob;
   e_widget_on_change_hook_set(ob, _cb_color_viewer_font_change, cfdata);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);

   e_widget_list_object_append(o, of, 1, 1, 1.0);

   return o;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *o2, *of, *of2, *ob;
   E_Radio_Group *rg;

   o = e_widget_table_add(evas, 0);

   o2 = e_widget_list_add(evas, 0, 0);

   of = e_widget_frametable_add(evas, D_("Feeds"), 0);

   ob = e_widget_label_add(evas, D_("Timer"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 0);
   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f min"),
			    (float)NEWS_FEED_TIMER_MIN, (float)NEWS_FEED_TIMER_MAX,
			    1.0, 0, NULL, &(cfdata->feed.timer_m), 70);
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 0);
   ob = e_widget_check_add(evas, D_("Sort lists by name (disable Move action)"), &(cfdata->feed.sort_name));
   e_widget_frametable_object_append(of, ob, 0, 1, 2, 1, 1, 1, 1, 0);

   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, D_("Browser"), 0);

   ob = e_widget_label_add(evas, D_("Uses xdg-open script, from freedesktop.org"));
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, D_("Popup On News"), 0);

   rg = e_widget_radio_group_new(&(cfdata->popup_news.active));
   ob = e_widget_radio_add(evas, D_("Never"),
			   NEWS_FEED_POPUP_NEWS_ACTIVE_NO, rg);
   //TODO: NOT IMPLEMENTED YET
   e_widget_radio_toggle_set(ob, 0);
   e_widget_disabled_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 0);
   ob = e_widget_radio_add(evas, D_("Only on urgent feeds"),
			   NEWS_FEED_POPUP_NEWS_ACTIVE_URGENT, rg);
   //TODO: NOT IMPLEMENTED YET
   e_widget_radio_toggle_set(ob, 0);
   e_widget_disabled_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 0);
   ob = e_widget_radio_add(evas, D_("On all feeds"),
			   NEWS_FEED_POPUP_NEWS_ACTIVE_ALL, rg);
   //TODO: NOT IMPLEMENTED YET
   e_widget_radio_toggle_set(ob, 0);
   e_widget_disabled_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 0);

   ob = e_widget_label_add(evas, D_("Timer"));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 0);
   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f s"),
			    (float)NEWS_POPUP_TIMER_S_MIN,
			    (float)NEWS_POPUP_TIMER_S_MAX,
			    1.0, 0, NULL, &(cfdata->popup_news.timer_s), 70);
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 1, 1, 0);

   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, D_("Popup On Warning / Error"), 0);

   //TODO: NOT IMPLEMENTED YET
   news->config->popup_other.on_timeout = 0;
   cfdata->popup_other.on_timeout = 0;
   ob = e_widget_check_add(evas, D_("Show on timeout ?"), &(cfdata->popup_other.on_timeout));
   //TODO: NOT IMPLEMENTED YET
   e_widget_disabled_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 1, 1, 0, 1, 0);

   ob = e_widget_label_add(evas, D_("Timer"));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 0);
   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f s"),
			    (float)NEWS_POPUP_TIMER_S_MIN,
			    (float)NEWS_POPUP_TIMER_S_MAX,
			    1.0, 0, NULL, &(cfdata->popup_other.timer_s), 70);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 0);

   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   e_widget_table_object_append(o, o2, 0, 0, 1, 1, 1, 1, 1, 1);


   o2 = e_widget_list_add(evas, 0, 0);

   of = e_widget_frametable_add(evas, D_("News Viewer"), 0);

   ob = e_widget_check_add(evas, D_("Unread feeds first"), &(cfdata->viewer.vfeeds.unread_first));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 0, 1, 0);
   ob = e_widget_check_add(evas, D_("Unread articles first"), &(cfdata->viewer.varticles.unread_first));
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 0, 1, 0);
   ob = e_widget_check_add(evas, D_("Sort articles by date"), &(cfdata->viewer.varticles.sort_date));
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 0, 1, 0);

   of2 = e_widget_frametable_add(evas, D_("Font"), 0);

   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f"), NEWS_VIEWER_VCONTENT_FONT_SIZE_MIN, NEWS_VIEWER_VCONTENT_FONT_SIZE_MAX, 1.0, 0, NULL, &(cfdata->viewer.vcontent.font_size), 70);
   e_widget_frametable_object_append(of2, ob, 0, 0, 1, 1, 1, 0, 1, 0);

   ob = e_widget_color_well_add(evas, cfdata->viewer.vcontent.font_color, 1);
   cfdata->color_viewer_font = ob;
   e_widget_on_change_hook_set(ob, _cb_color_viewer_font_change, cfdata);
   e_widget_frametable_object_append(of2, ob, 0, 1, 1, 1, 1, 0, 1, 0);

   ob = e_widget_check_add(evas, D_("Font Shadow"), &cfdata->viewer.vcontent.font_shadow);
   e_widget_frametable_object_append(of2, ob, 1, 0, 1, 1, 1, 0, 1, 0);

   ob = e_widget_color_well_add(evas, cfdata->viewer.vcontent.font_shadow_color, 1);
   cfdata->color_viewer_font_shadow = ob;
   e_widget_on_change_hook_set(ob, _cb_color_viewer_font_shadow_change, cfdata);
   e_widget_frametable_object_append(of2, ob, 1, 1, 1, 1, 1, 0, 1, 0);

   e_widget_frametable_object_append(of, of2, 0, 2, 2, 1, 1, 0, 1, 0);

   e_widget_list_object_append(o2, of, 1, 1, 0.0);

   of = e_widget_frametable_add(evas, D_("Proxy"), 0);

   ob = e_widget_check_add(evas, D_("Enable"), &(cfdata->proxy.enable));
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 1, 1, 0, 0, 0);
   ob = e_widget_label_add(evas, D_("Host"));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 0, 0, 0);
   ob = e_widget_entry_add(evas, &(cfdata->proxy.host), NULL, NULL, NULL);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 0, 1, 0);
   ob = e_widget_label_add(evas, D_("Port"));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 0, 0, 0);
   ob = e_widget_entry_add(evas, &(cfdata->proxy.port), NULL, NULL, NULL);
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 0, 1, 0);

   e_widget_list_object_append(o2, of, 1, 1, 1.0);

   e_widget_table_object_append(o, o2, 1, 0, 1, 1, 1, 1, 1, 1);

   return o;
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   News_Config *c;
   int update_feeds = 0;

   c = news->config;

   if (c->feed.timer_m != cfdata->feed.timer_m)
     news_feed_timer_set(cfdata->feed.timer_m);
   if (c->feed.sort_name != cfdata->feed.sort_name)
     {
        c->feed.sort_name = cfdata->feed.sort_name;
        if (c->feed.sort_name)
          news_feed_lists_refresh(1);
     }

   if (c->proxy.enable != cfdata->proxy.enable)
     {
        c->proxy.enable = cfdata->proxy.enable;
        update_feeds = 1;
     }
   if (c->proxy.host)
     {
        eina_stringshare_del(c->proxy.host);
        c->proxy.host = NULL;
     }
   if (cfdata->proxy.host && cfdata->proxy.host[0])
     c->proxy.host = eina_stringshare_add(cfdata->proxy.host);
   if (cfdata->proxy.port && cfdata->proxy.port[0])
     sscanf(cfdata->proxy.port, "%d", &c->proxy.port);

   if ( (c->viewer.vfeeds.unread_first != cfdata->viewer.vfeeds.unread_first) ||
        (c->viewer.varticles.unread_first != cfdata->viewer.varticles.unread_first) ||
        (c->viewer.varticles.sort_date != cfdata->viewer.varticles.sort_date) ||
        (c->viewer.vcontent.font_size != cfdata->viewer.vcontent.font_size) ||
        (c->viewer.vcontent.font_shadow != cfdata->viewer.vcontent.font_shadow) ||
        (cfdata->viewer.vcontent.color_changed) )
   {
      int recreate = 0;
      char buf[8];
      
      if ( (c->viewer.vcontent.font_size != cfdata->viewer.vcontent.font_size) ||
           (c->viewer.vcontent.font_shadow != cfdata->viewer.vcontent.font_shadow) ||
           (cfdata->viewer.vcontent.color_changed) )
        recreate = 1;

      c->viewer.vfeeds.unread_first = cfdata->viewer.vfeeds.unread_first;
      c->viewer.varticles.unread_first = cfdata->viewer.varticles.unread_first;
      c->viewer.varticles.sort_date = cfdata->viewer.varticles.sort_date;
      c->viewer.vcontent.font_size = cfdata->viewer.vcontent.font_size;

      if (cfdata->viewer.vcontent.font_color->r < 16) cfdata->viewer.vcontent.font_color->r = 16;
      if (cfdata->viewer.vcontent.font_color->g < 16) cfdata->viewer.vcontent.font_color->g = 16;
      if (cfdata->viewer.vcontent.font_color->b < 16) cfdata->viewer.vcontent.font_color->b = 16;
      snprintf(buf, sizeof(buf), "#%x%x%x",
               cfdata->viewer.vcontent.font_color->r,
               cfdata->viewer.vcontent.font_color->g,
               cfdata->viewer.vcontent.font_color->b);
      eina_stringshare_del(news->config->viewer.vcontent.font_color);
      news->config->viewer.vcontent.font_color = eina_stringshare_add(buf);

      c->viewer.vcontent.font_shadow = cfdata->viewer.vcontent.font_shadow;

      if (cfdata->viewer.vcontent.font_shadow_color->r < 16) cfdata->viewer.vcontent.font_shadow_color->r = 16;
      if (cfdata->viewer.vcontent.font_shadow_color->g < 16) cfdata->viewer.vcontent.font_shadow_color->g = 16;
      if (cfdata->viewer.vcontent.font_shadow_color->b < 16) cfdata->viewer.vcontent.font_shadow_color->b = 16;
      snprintf(buf, sizeof(buf), "#%x%x%x",
               cfdata->viewer.vcontent.font_shadow_color->r,
               cfdata->viewer.vcontent.font_shadow_color->g,
               cfdata->viewer.vcontent.font_shadow_color->b);
      eina_stringshare_del(news->config->viewer.vcontent.font_shadow_color);
      news->config->viewer.vcontent.font_shadow_color = eina_stringshare_add(buf);

      news_viewer_all_refresh(1, recreate);
      cfdata->viewer.vcontent.color_changed = 0;
   }

   c->popup_news.active = cfdata->popup_news.active;
   c->popup_news.timer_s = cfdata->popup_news.timer_s;

   c->popup_other.on_timeout = cfdata->popup_other.on_timeout;
   c->popup_other.timer_s = cfdata->popup_other.timer_s;

   /* update feeds */
   if (update_feeds)
     {
        NEWS_FEED_FOREACH_BEG();
        if (_feed->doc)
          news_feed_update(_feed);
        NEWS_FEED_FOREACH_END();
     }

   news_config_save();
   return 1;
}

static void
_cb_color_viewer_font_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   cfdata->viewer.vcontent.color_changed = 1;
}

static void
_cb_color_viewer_font_shadow_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   cfdata->viewer.vcontent.color_changed = 1;
}
