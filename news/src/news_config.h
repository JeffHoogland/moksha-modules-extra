#ifdef E_MOD_NEWS_TYPEDEFS

typedef struct _News_Config News_Config;
typedef struct _News_Config_Item News_Config_Item;

#else

#ifndef NEWS_CONFIG_H_INCLUDED
#define NEWS_CONFIG_H_INCLUDED

#define NEWS_CONFIG_VERSION 12

struct _News_Config
{
   int version;

   struct
   {
      Eina_List *categories;
      int        timer_m;
      int        sort_name;
      Eina_List *langs;
      int        langs_all;
      int        langs_notset;
   } feed;

   struct
   {
      int enable;
      const char *host;
      int port;
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
         int         font_size;
         const char *font_color;
         int         font_shadow;
         const char *font_shadow_color;
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

   Eina_List *items;
};

struct _News_Config_Item
{
   const char *id;

   /* list of Feed_Ref */
   Eina_List *feed_refs;

   int view_mode;
   int openmethod;
   int browser_open_home;

   int apply_to_all;
};

int               news_config_init(void);
int               news_config_shutdown(void);

int               news_config_save(void);

News_Config_Item *news_config_item_add(const char *id);
void              news_config_item_del(News_Config_Item *nic);

#endif
#endif
