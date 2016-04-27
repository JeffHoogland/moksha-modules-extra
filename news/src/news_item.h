#ifdef E_MOD_NEWS_TYPEDEFS

typedef struct _News_Item News_Item;

typedef enum _News_Item_View_Mode
   {
      NEWS_ITEM_VIEW_MODE_ONE,
      NEWS_ITEM_VIEW_MODE_FEED,
      NEWS_ITEM_VIEW_MODE_FEED_UNREAD,
      NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT,
      NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT_UNREAD
   } News_Item_View_Mode;

typedef enum _News_Item_Openmethod
   {
      NEWS_ITEM_OPENMETHOD_VIEWER,
      NEWS_ITEM_OPENMETHOD_BROWSER,
   } News_Item_View_Openmethod;


#else

#ifndef NEWS_ITEM_H_INCLUDED
#define NEWS_ITEM_H_INCLUDED

#define NEWS_ITEM_VIEW_MODE_DEFAULT NEWS_ITEM_VIEW_MODE_FEED
#define NEWS_ITEM_OPENMETHOD_DEFAULT NEWS_ITEM_OPENMETHOD_VIEWER
#define NEWS_ITEM_BROWSER_OPEN_HOME_DEFAULT 0

#define NEWS_ITEM_FEEDS_FOREACH_BEG_LIST(list) \
{ \
Eina_List *_l_feeds; \
News_Feed_Ref *_ref; \
News_Feed *_feed; \
\
for (_l_feeds=list; _l_feeds; _l_feeds=eina_list_next(_l_feeds)) \
{ \
   _ref = _l_feeds->data; \
   _feed = _ref->feed; \
   if (!_feed) continue;
#define NEWS_ITEM_FEEDS_FOREACH_BEG(item) \
NEWS_ITEM_FEEDS_FOREACH_BEG_LIST(item->config->feed_refs);
#define NEWS_ITEM_FEEDS_FOREACH_END() \
} \
}

struct _News_Item
{
   E_Gadcon_Client *gcc;
   Evas_Object     *obj;

   News_Config_Item *config;
   E_Config_Dialog  *config_dialog;
   E_Config_Dialog  *config_dialog_content;
   E_Menu           *menu;
   E_Menu           *menu_browser;

   struct
   {
      Evas_Object *box;
      Evas_Object *obj_mode_one;
   } view;

   /* TODO: a counter from 10 to 0 incremented every retry
      for any feed linked to this item.
      when it reaches 0, we look at the feed(s) wich failed
      and we alert the user via a popup
      (if all feeds have failed, special popup "connection pb") */
   int loading_state;

   int unread_count;

   News_Viewer *viewer;
};


News_Item   *news_item_new(E_Gadcon_Client *gcc, Evas_Object *obj, const char *id);
void         news_item_free(News_Item *ni);

void         news_item_refresh(News_Item *ni, int changed_order, int changed_content, int changed_state);
void         news_item_orient_set(News_Item *ni, int horizontal);

void         news_item_loadingstate_refresh(News_Item *ni);

void         news_item_unread_count_change(News_Item *ni, int nb);

#endif
#endif
