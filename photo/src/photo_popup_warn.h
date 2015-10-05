#ifdef E_MOD_PHOTO_TYPEDEFS

typedef struct _Popup_Warn Popup_Warn;

#else

#ifndef PHOTO_POPUP_WARN_H_INCLUDED
#define PHOTO_POPUP_WARN_H_INCLUDED

#define POPUP_WARN_TYPE_NEWS 0
#define POPUP_WARN_TYPE_ERROR 1

#define POPUP_WARN_EDJE_MESSAGE_SHOW_DESACTIVATE 0
#define POPUP_WARN_EDJE_MESSAGE_TYPE 1

#define POPUP_WARN_TIMER_DEFAULT 8
#define POPUP_WARN_TIMER_MIN 2
#define POPUP_WARN_TIMER_MAX 60

#define POPUP_WARN_OVERLAP_BORDER 15
#define POPUP_WARN_OVERLAP_CHECK_MAX 50

struct _Popup_Warn
{
   int type;
   E_Popup *pop;
   Evas_Object *face;

   Ecore_Timer *timer;
   int (*func_close) (Popup_Warn *popw, void *data);
   void (*func_desactivate) (Popup_Warn *popw, void *data);
  void *data;

   int timer_org;
   int x, y, w, h;
};

int         photo_popup_warn_init(void);
void        photo_popup_warn_shutdown(void);

Popup_Warn *photo_popup_warn_add(int type, const char *text, int timer, int (*func_close) (Popup_Warn *popw, void *data), void (func_desactivate) (Popup_Warn *popw, void *data), void *data);
void        photo_popup_warn_del(Popup_Warn *popw);

#endif
#endif
