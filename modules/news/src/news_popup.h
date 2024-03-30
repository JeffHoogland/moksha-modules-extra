#ifdef E_MOD_NEWS_TYPEDEFS

typedef struct _News_Popup News_Popup;

typedef enum _News_Popup_Active
   {
      NEWS_POPUP_ACTIVE_NO,
      NEWS_POPUP_ACTIVE_SUM,
      NEWS_POPUP_ACTIVE_VERBOSE
   } News_Popup_Active;

typedef enum _News_Popup_Type
   {
      NEWS_POPUP_TYPE_NEWS,
      NEWS_POPUP_TYPE_INFO,
      NEWS_POPUP_TYPE_ERROR
   } News_Popup_Type;

#else

#ifndef NEWS_POPUP_H_INCLUDED
#define NEWS_POPUP_H_INCLUDED

#define NEWS_POPUP_TIMER_S_MIN 2
#define NEWS_POPUP_TIMER_S_MAX 60

#define NEWS_POPUP_EDJE_MESSAGE_SHOW_DESACTIVATE 0
#define NEWS_POPUP_EDJE_MESSAGE_TYPE 1

#define NEWS_POPUP_OVERLAP_BORDER 15
#define NEWS_POPUP_OVERLAP_CHECK_MAX 50

struct _News_Popup
{
   News_Popup_Type type;
   E_Popup *pop;
   Evas_Object *face;
  Evas_Object *tb;
  Eina_List *log; /* 2 consecutive strings for each popup */

   Ecore_Timer *timer;
   int (*func_close) (News_Popup *popw, void *data);
   void (*func_desactivate) (News_Popup *popw, void *data);
  void *data;

   int timer_org;
   int x, y, w, h;
};

int              news_popup_init(void);
void             news_popup_shutdown(void);

News_Popup *news_popup_add(int type, const char *title, const char *text, int timer, int (*func_close) (News_Popup *popw, void *data), void (func_desactivate) (News_Popup *popw, void *data), void *data);
void             news_popup_del(News_Popup *popw);

#endif
#endif
