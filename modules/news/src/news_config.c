#include "News.h"

static E_Config_DD *_news_edd = NULL;
static E_Config_DD *_news_feed_edd = NULL;
static E_Config_DD *_news_feed_ref_edd = NULL;
static E_Config_DD *_news_feed_category_edd = NULL;
static E_Config_DD *_news_feed_lang_edd = NULL;
static E_Config_DD *_news_item_edd = NULL;


/*
 * Public functions
 */

int
news_config_init(void)
{
   _news_feed_edd = E_CONFIG_DD_NEW("News_Feed", News_Feed);
#undef T
#undef D
#define T News_Feed
#define D _news_feed_edd
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, name_ovrw, SHORT);
   E_CONFIG_VAL(D, T, language, STR);
   E_CONFIG_VAL(D, T, language_ovrw, SHORT);
   E_CONFIG_VAL(D, T, description, STR);
   E_CONFIG_VAL(D, T, description_ovrw, SHORT);
   E_CONFIG_VAL(D, T, url_home, STR);
   E_CONFIG_VAL(D, T, url_home_ovrw, SHORT);
   E_CONFIG_VAL(D, T, url_feed, STR);
   E_CONFIG_VAL(D, T, icon, STR);
   E_CONFIG_VAL(D, T, icon_ovrw, SHORT);
   E_CONFIG_VAL(D, T, important, SHORT);
   _news_feed_ref_edd = E_CONFIG_DD_NEW("News_Feed_Ref", News_Feed_Ref);
#undef T
#undef D
#define T News_Feed_Ref
#define D _news_feed_ref_edd
   E_CONFIG_VAL(D, T, category, STR);
   E_CONFIG_VAL(D, T, name, STR);
   _news_feed_category_edd = E_CONFIG_DD_NEW("News_Feed_Category", News_Feed_Category);
#undef T
#undef D
#define T News_Feed_Category
#define D _news_feed_category_edd
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, icon, STR);
   E_CONFIG_LIST(D, T, feeds, _news_feed_edd);
   _news_feed_lang_edd = E_CONFIG_DD_NEW("News_Feed_Lang", News_Feed_Lang);
#undef T
#undef D
#define T News_Feed_Lang
#define D _news_feed_lang_edd
   E_CONFIG_VAL(D, T, key, STR);
   E_CONFIG_VAL(D, T, name, STR);
   _news_item_edd = E_CONFIG_DD_NEW("News_Item", News_Config_Item);
#undef T
#undef D
#define T News_Config_Item
#define D _news_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_LIST(D, T, feed_refs, _news_feed_ref_edd);
   E_CONFIG_VAL(D, T, view_mode, SHORT);
   E_CONFIG_VAL(D, T, openmethod, SHORT);
   E_CONFIG_VAL(D, T, browser_open_home, SHORT);
   _news_edd = E_CONFIG_DD_NEW("News", News_Config);
#undef T
#undef D
#define T News_Config
#define D _news_edd
   E_CONFIG_VAL(D, T, version, SHORT);
   E_CONFIG_LIST(D, T, feed.categories, _news_feed_category_edd);
   E_CONFIG_VAL(D, T, feed.timer_m, INT);
   E_CONFIG_VAL(D, T, feed.sort_name, INT);
   E_CONFIG_LIST(D, T, feed.langs, _news_feed_lang_edd);
   E_CONFIG_VAL(D, T, feed.langs_all, SHORT);
   E_CONFIG_VAL(D, T, feed.langs_notset, SHORT);
   E_CONFIG_VAL(D, T, proxy.enable, SHORT);
   E_CONFIG_VAL(D, T, proxy.host, STR);
   E_CONFIG_VAL(D, T, proxy.port, INT);
   E_CONFIG_VAL(D, T, viewer.vfeeds.unread_first, SHORT);
   E_CONFIG_VAL(D, T, viewer.varticles.unread_first, SHORT);
   E_CONFIG_VAL(D, T, viewer.varticles.sort_date, SHORT);
   E_CONFIG_VAL(D, T, viewer.vcontent.font_size, SHORT);
   E_CONFIG_VAL(D, T, viewer.vcontent.font_color, STR);
   E_CONFIG_VAL(D, T, viewer.vcontent.font_shadow, SHORT);
   E_CONFIG_VAL(D, T, viewer.vcontent.font_shadow_color, STR);
   E_CONFIG_VAL(D, T, popup_news.active, SHORT);
   E_CONFIG_VAL(D, T, popup_news.timer_s, INT);
   E_CONFIG_VAL(D, T, popup_other.on_timeout, SHORT);
   E_CONFIG_VAL(D, T, popup_other.timer_s, INT);
   E_CONFIG_LIST(D, T, items, _news_item_edd);
   news->config = e_config_domain_load("module.news", _news_edd);

   if (news->config)
     {
        if (news->config->version < NEWS_CONFIG_VERSION)
          {
             news_util_message_show(D_("<hilight>News module : Configuration Upgraded</hilight><br><br>"
                                       "Your configuration of news module<br>"
                                       "has been upgraded<br>"
                                       "Your settings were removed<br>"
                                       "Sorry for the inconvenience<br><br>"
                                       "(%d -> %d)"), news->config->version, NEWS_CONFIG_VERSION);
             news->config = NULL;
          }
        else
          {
             if (news->config->version > NEWS_CONFIG_VERSION)
               {
                  news_util_message_show(D_("<hilight>News module : Configuration Downgraded</hilight><br><br>"
                                            "Your configuration of News module<br>"
                                            "has been downgraded<br>"
                                            "Your settings were removed<br>"
                                            "Sorry for the inconvenience<br><br>"
                                            "(%d ->%d)"), news->config->version, NEWS_CONFIG_VERSION);
                  news->config = NULL;
               }
          }
     }

   if (!news->config)
     {
        News_Config *c;

        c = E_NEW(News_Config, 1);
        news->config = c;
        c->version = NEWS_CONFIG_VERSION;

        news_feed_all_restore();

        c->feed.timer_m = NEWS_FEED_TIMER_DEFAULT;
        c->feed.sort_name = NEWS_FEED_SORT_NAME_DEFAULT;
        c->feed.langs = news_util_lang_detect();
        c->feed.langs_all = NEWS_FEED_LANG_ALL_DEFAULT;
        c->feed.langs_notset = 1;
        if (news_util_proxy_detect())
          c->proxy.enable = 1;
        else
          {
             c->proxy.host = NULL;
             c->proxy.port = NEWS_FEED_PROXY_PORT_DEFAULT;
          }

        c->viewer.vfeeds.unread_first = NEWS_VIEWER_VFEEDS_UNREAD_FIRST_DEFAULT;
        c->viewer.varticles.unread_first = NEWS_VIEWER_VARTICLES_UNREAD_FIRST_DEFAULT;
        c->viewer.varticles.sort_date = NEWS_VIEWER_VARTICLES_SORT_DATE_DEFAULT;
        c->viewer.vcontent.font_size = NEWS_VIEWER_VCONTENT_FONT_SIZE_DEFAULT;
        c->viewer.vcontent.font_color = eina_stringshare_add(NEWS_VIEWER_VCONTENT_FONT_COLOR_DEFAULT);
        c->viewer.vcontent.font_shadow = NEWS_VIEWER_VCONTENT_FONT_SHADOW_DEFAULT;
        c->viewer.vcontent.font_shadow_color = eina_stringshare_add(NEWS_VIEWER_VCONTENT_FONT_SHADOW_COLOR_DEFAULT);
        c->popup_news.active =  NEWS_FEED_POPUP_NEWS_ACTIVE_DEFAULT;
        c->popup_news.timer_s = NEWS_FEED_POPUP_NEWS_TIMER_S_DEFAULT;
        c->popup_other.on_timeout =  NEWS_FEED_POPUP_OTHER_ON_TIMEOUT_DEFAULT;
        c->popup_other.timer_s = NEWS_FEED_POPUP_OTHER_TIMER_S_DEFAULT;
     }

   E_CONFIG_LIMIT(news->config->feed.timer_m, NEWS_FEED_TIMER_MIN, NEWS_FEED_TIMER_MAX);
   E_CONFIG_LIMIT(news->config->feed.sort_name, 0, 1);
   E_CONFIG_LIMIT(news->config->proxy.enable, 0, 1);
   E_CONFIG_LIMIT(news->config->proxy.port, 1, 65535);
   E_CONFIG_LIMIT(news->config->viewer.vfeeds.unread_first, 0, 1);
   E_CONFIG_LIMIT(news->config->viewer.varticles.unread_first, 0, 1);
   E_CONFIG_LIMIT(news->config->viewer.varticles.sort_date, 0, 1);
   E_CONFIG_LIMIT(news->config->viewer.vcontent.font_size, NEWS_VIEWER_VCONTENT_FONT_SIZE_MIN, NEWS_VIEWER_VCONTENT_FONT_SIZE_MAX);
   E_CONFIG_LIMIT(news->config->viewer.vcontent.font_shadow, 0, 1);
   E_CONFIG_LIMIT(news->config->popup_news.timer_s, NEWS_POPUP_TIMER_S_MIN, NEWS_POPUP_TIMER_S_MAX);
   E_CONFIG_LIMIT(news->config->popup_other.on_timeout, 0, 1);
   E_CONFIG_LIMIT(news->config->popup_other.timer_s, NEWS_POPUP_TIMER_S_MIN, NEWS_POPUP_TIMER_S_MAX);

   return 1;
}

int
news_config_shutdown(void)
{
   News_Config *c;
   News_Config_Item *nic;

   c = news->config;

   news_feed_all_delete();

   news_feed_lang_list_free(news->config->feed.langs);

   if (c->proxy.host) eina_stringshare_del(c->proxy.host);

   while ( (nic = eina_list_data_get(c->items)) )
     news_config_item_del(nic);

   eina_stringshare_del(c->viewer.vcontent.font_color);
   eina_stringshare_del(c->viewer.vcontent.font_shadow_color);

   E_FREE(news->config);

   E_CONFIG_DD_FREE(_news_edd);
   E_CONFIG_DD_FREE(_news_feed_edd);
   E_CONFIG_DD_FREE(_news_feed_ref_edd);
   E_CONFIG_DD_FREE(_news_feed_category_edd);
   E_CONFIG_DD_FREE(_news_item_edd);

   return 1;
}

int
news_config_save(void)
{
   e_config_domain_save("module.news", _news_edd, news->config);

   return 1;
}

News_Config_Item *
news_config_item_add(const char *id)
{
   News_Config_Item *nic;
   Eina_List *l;
   char buf[128];

   DCONF(("Item new config"));

   if (!id)
     {
	int  num = 0;

	/* Create id */
	if (news->config->items)
	  {
	     const char *p;
	     nic = eina_list_last(news->config->items)->data;
	     p = strrchr(nic->id, '.');
	     if (p) num = atoi(p + 1) + 1;
	  }
	snprintf(buf, sizeof(buf), "%s.%d", _gc_name(), num);
	id = buf;
     }
   else
     {
	/* is there already an item config for this id ? */
	for (l=news->config->items; l; l=eina_list_next(l))
	  {
	     nic = eina_list_data_get(l);
	     if (!strcmp(nic->id, id))
	       {
		  DCONF(("config found ! %s", nic->id));
		  return nic;
	       }
	  }
     }

   DCONF(("config NOT found ! creating new one %s", id));
   /* no, create a new item config */
   nic = E_NEW(News_Config_Item, 1);

   nic->id = eina_stringshare_add(id);
   nic->view_mode = NEWS_ITEM_VIEW_MODE_DEFAULT;
   nic->openmethod = NEWS_ITEM_OPENMETHOD_DEFAULT;
   nic->browser_open_home = NEWS_ITEM_BROWSER_OPEN_HOME_DEFAULT;

   news->config->items = eina_list_append(news->config->items, nic);

   return nic;
}

void
news_config_item_del(News_Config_Item *nic)
{
   eina_stringshare_del(nic->id);

   while(eina_list_count(nic->feed_refs))
     {
        News_Feed_Ref *fr;
        fr = eina_list_data_get(nic->feed_refs);
        eina_stringshare_del(fr->name);
        nic->feed_refs = eina_list_remove(nic->feed_refs, fr);
     }

   news->config->items = eina_list_remove(news->config->items, nic);
   free(nic);
}


/*
 * Private functions
 *
 */
