#ifdef E_MOD_PHOTO_TYPEDEFS

typedef struct _Popup_Info Popup_Info;

#else

#ifndef PHOTO_POPUP_INFO_H_INCLUDED
#define PHOTO_POPUP_INFO_H_INCLUDED

#define POPUP_INFO_PLACEMENT_SHELF 0
#define POPUP_INFO_PLACEMENT_CENTERED 1

#define POPUP_INFO_TIMER_DEFAULT 8
#define POPUP_INFO_TIMER_MAX 60

struct _Popup_Info
{
   Photo_Item *pi;

   E_Popup *pop;
   Evas_Object *face;
  Evas_Object *tb;
   Evas_Object *icon;

   Ecore_Timer *timer;
   int timer_org;
   int x, y, w, h;

   void (*cb_func) (void *data);
   void *data;
};

int         photo_popup_info_init(void);
void        photo_popup_info_shutdown(void);

Popup_Info *photo_popup_info_add(Photo_Item *pi, const char *title, const char *text, Picture *picture, int timer, int placement, void (cb_func) (void *data), void *data);
void        photo_popup_info_del(Popup_Info *popi);

#endif
#endif
