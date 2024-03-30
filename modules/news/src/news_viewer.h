#ifdef E_MOD_NEWS_TYPEDEFS

typedef struct _News_Viewer News_Viewer;

#else

#ifndef NEWS_VIEWER_H_INCLUDED
#define NEWS_VIEWER_H_INCLUDED

#define NEWS_VIEWER_VFEEDS_UNREAD_FIRST_DEFAULT 1
#define NEWS_VIEWER_VARTICLES_UNREAD_FIRST_DEFAULT 1
#define NEWS_VIEWER_VARTICLES_SORT_DATE_DEFAULT 1
#define NEWS_VIEWER_VCONTENT_FONT_SIZE_DEFAULT 10
#define NEWS_VIEWER_VCONTENT_FONT_SIZE_MIN 3
#define NEWS_VIEWER_VCONTENT_FONT_SIZE_MAX 20
#define NEWS_VIEWER_VCONTENT_FONT_COLOR_DEFAULT "#000000"
#define NEWS_VIEWER_VCONTENT_FONT_SHADOW_DEFAULT 1
#define NEWS_VIEWER_VCONTENT_FONT_SHADOW_COLOR_DEFAULT "#EFEFEF"

struct _News_Viewer
{
   News_Item *item;
   News_Feed_Document *doc;

   /* main dialog */
   struct
   {
      E_Dialog    *dia;
      int x, y, w, h;
      
      Evas_Object *tab;
      Evas_Object *ftab_feeds, *ftab_articles, *ftab_content;
   } dialog;
  
   /* view of the feeds list */
   struct
   {
      Eina_List   *list;
      int          list_own;
      Evas_Object *ilist;
      News_Feed   *selected;
      Evas_Object *button_feed;
      Evas_Object *button_feed_icon;
      Evas_Object *button_refresh;
      Evas_Object *button_setasread;
   } vfeeds;

   /* view of the articles list */
   struct
   {
      Eina_List         *list;
      int                list_own;
      Evas_Object       *ilist;
      News_Feed_Article *selected;
   } varticles;

   /* view of article content */
   struct
   {
      Evas_Object *scrollframe;
      Evas_Object *tb;
   } vcontent;
};

int   news_viewer_init(void);
void  news_viewer_shutdown(void);
void  news_viewer_all_refresh(int force, int recreate);

int   news_viewer_create(News_Item *ni);
void  news_viewer_destroy(News_Viewer *nv);
void  news_viewer_refresh(News_Viewer *nv);
int   news_viewer_feed_select(News_Viewer *nv, News_Feed *feed);
void  news_viewer_feed_selected_infos_refresh(News_Viewer *nv);
void  news_viewer_article_state_refresh(News_Viewer *nv, News_Feed_Article *art);

#endif
#endif
