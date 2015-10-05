#include "Photo.h"

#define POPI_BORDER 6

#define POPI_ORIENT_DOWN() \
  x = (gx+ow/2) - w/2;  \
  y = gh + POPI_BORDER;
#define POPI_ORIENT_UP()  \
  x = (gx+ow/2) - w/2; \
  y = (gy - h) - POPI_BORDER;
#define POPI_ORIENT_LEFT() \
  x = (gx - w) - POPI_BORDER; \
  y = (gy+oh/2) - h/2;
#define POPI_ORIENT_RIGHT() \
  x = gw + POPI_BORDER;    \
  y = (gy+oh/2) - h/2;
#define POPI_ORIENT_CENTER() \
  x = (ew/2) - (w/2);        \
  y = (eh/2) - (h/2);
/* TODO : real orient auto */
#define POPI_ORIENT_AUTO() \
  x = (ew/2) - (w/2);      \
  y = (eh/2) - (h/2);


static Eina_List *_popups_info;


static void _placement(Photo_Item *pi, int placement, int popi_w, int popi_h, int *popi_x, int *popi_y);
static void _close(Popup_Info *popi);
static Eina_Bool  _cb_timer(void *data);;
static void _cb_edje_close(void *data, Evas_Object *obj, const char *emission, const char *source);

/*
 * Public functions
 */

int photo_popup_info_init(void)
{
   _popups_info = NULL;

   return 1;
}

void photo_popup_info_shutdown(void)
{
   Eina_List *l;

   for (l = _popups_info; l; l = eina_list_next(l))
     {
        Popup_Info *p;
        p = eina_list_data_get(l);
        photo_popup_info_del(p);
     }
   eina_list_free(_popups_info);
   _popups_info = NULL;
}

Popup_Info *photo_popup_info_add(Photo_Item *pi, const char *title, const char *text, Picture *picture, int timer, int placement, void (*cb_func) (void *data), void *data)
{
  E_Zone *zone;
  Popup_Info *popi;
  Evas_Object *tb;
  Evas_Textblock_Style *tb_style;
  int fw, fh, tw, th;

  popi = E_NEW(Popup_Info, 1);

  popi->pi = pi;
  popi->timer_org = timer;

  zone = e_util_zone_current_get(e_manager_current_get());

  /* pop */
  popi->pop = e_popup_new(zone, 0, 0, 1, 1);
  if (!popi->pop)
    {
      photo_popup_info_del(popi);
      return 0;
    }
  evas_event_freeze(popi->pop->evas);
  e_popup_layer_set(popi->pop, 255);

  /* textblock */
  tb = evas_object_textblock_add(popi->pop->evas);
  tb_style = evas_textblock_style_new();
  evas_textblock_style_set(tb_style,
                           "DEFAULT='font=Vera font_size=10 align=left color=#000000ff wrap=line'" "br='\n'");
  evas_object_textblock_style_set(tb, tb_style);
  evas_textblock_style_free(tb_style);

  evas_object_textblock_clear(tb);
  evas_object_textblock_text_markup_set(tb, text);
  evas_object_textblock_size_formatted_get(tb, &tw, &th);
  evas_object_resize(tb, tw, th);
  popi->tb = tb;

  /* face, title, icon and text */
  popi->face = edje_object_add(popi->pop->evas);
  photo_util_edje_set(popi->face, PHOTO_THEME_POPI);
  edje_object_part_text_set(popi->face, "title", title);
  edje_object_part_swallow(popi->face, "text", tb);
  evas_object_pass_events_set(tb, 1);
   if (picture)
   {
      Evas_Object *im;
      im = photo_picture_object_get(picture,
                                    popi->pop->evas);
      if (im)
        {
           popi->icon = im;
           edje_object_part_swallow(popi->face, "icon", im);
           evas_object_pass_events_set(im, 1);
        }
   }
  edje_object_signal_callback_add(popi->face, "close", "popup",
				  _cb_edje_close, popi);
  edje_object_part_geometry_get(popi->face, "background",
				NULL, NULL, &fw, &fh);
  evas_object_move(popi->face, 0, 0);

  /* pos and size */
  popi->w = fw + tw;
  popi->h = fh + th + 20;
  evas_object_resize(popi->face, popi->w, popi->h);
  _placement(pi, placement, popi->w, popi->h, &popi->x, &popi->y);
  e_popup_move_resize(popi->pop, popi->x, popi->y, popi->w, popi->h);
  DPOPI(("New: %dx%d : %dx%d", popi->x, popi->y, popi->w, popi->h));
  DPOPI(("New face: %dx%d", fw, fh));
  DPOPI(("New tb: %dx%d", tw, th));

  /* timer */
  if (timer)
    popi->timer = ecore_timer_add(timer, _cb_timer, popi);

  /* callback function and data / pi */
  popi->cb_func = cb_func;
  popi->data = data;

  /* go ! */
  evas_object_show(popi->face);
  e_popup_edje_bg_object_set(popi->pop, popi->face);
  evas_event_thaw(popi->pop->evas);
  e_popup_show(popi->pop);
  
  _popups_info = eina_list_append(_popups_info, popi);
	
   return popi;
}

void photo_popup_info_del(Popup_Info *popi)
{
  if (popi->timer)
    ecore_timer_del(popi->timer);
  if (popi->tb)
    evas_object_del(popi->tb);
  if (popi->icon)
    evas_object_del(popi->icon);
  if (popi->face)
    evas_object_del(popi->face);
  if (popi->pop)
    e_object_del(E_OBJECT(popi->pop));

  _popups_info = eina_list_remove(_popups_info, popi);

  free(popi);
}


/*
 * Private functions
 *
 */

static void
_placement(Photo_Item *pi, int placement, int popi_w, int popi_h, int *popi_x, int *popi_y)
{
   int x, y, w, h;
   int ox, oy, ow, oh;
   int gx, gy, gw, gh;
   int ew, eh;
	
   w = popi_w;
   h = popi_h;

   ew = photo->canvas_w;
   eh = photo->canvas_h;

   if (!pi || (placement == POPUP_INFO_PLACEMENT_CENTERED))
     {
        POPI_ORIENT_CENTER();
        *popi_x = x;
        *popi_y = y;
        return;
     }

   evas_object_geometry_get(pi->obj,
                            &ox, &oy, &ow, &oh);
   e_gadcon_canvas_zone_geometry_get(pi->gcc->gadcon,
                                     &gx, &gy, &gw, &gh);
   gx += ox;
   gy += oy;

   DPOPI(("Placement pre :\n"
          "x:%d y:%d w:%d, h:%d\n"
          "ox:%d oy:%d ow:%d, oh:%d\n"
          "gx:%d gy:%d gw:%d, gh:%d\n"
          "ew:%d eh:%d",
          x, y, w, h,
          ox, oy, ow, oh,
          gx, gy, gw, gh,
          ew, eh));

   switch (pi->gcc->gadcon->orient)
     {
     case E_GADCON_ORIENT_TOP:
     case E_GADCON_ORIENT_CORNER_TL:
     case E_GADCON_ORIENT_CORNER_TR:
        POPI_ORIENT_DOWN();
        break;
     case E_GADCON_ORIENT_BOTTOM:
     case E_GADCON_ORIENT_CORNER_BL:
     case E_GADCON_ORIENT_CORNER_BR:
        POPI_ORIENT_UP();
        break;
     case E_GADCON_ORIENT_LEFT:
     case E_GADCON_ORIENT_CORNER_LT:
     case E_GADCON_ORIENT_CORNER_LB:
        POPI_ORIENT_RIGHT();
        break;
     case E_GADCON_ORIENT_RIGHT:
     case E_GADCON_ORIENT_CORNER_RT:
     case E_GADCON_ORIENT_CORNER_RB:
        POPI_ORIENT_LEFT();
        break;
     case E_GADCON_ORIENT_FLOAT:
     case E_GADCON_ORIENT_HORIZ:
     case E_GADCON_ORIENT_VERT:
     default:
        POPI_ORIENT_AUTO();
        break;
     }

   if (x<POPI_BORDER) x=POPI_BORDER;
   if ((x+w)>(ew-POPI_BORDER)) x=(ew-w)-POPI_BORDER;
   if (y<POPI_BORDER) y=POPI_BORDER;
   if ((y+h)>(eh-POPI_BORDER)) y=(eh-h)-POPI_BORDER;

   DPOPI(("Placement post :\n"
          "x:%d y:%d w:%d, h:%d",
          x, y, w, h));

   *popi_x = x;
   *popi_y = y;
}

static void
_close(Popup_Info *popi)
{
   DPOPI(("close"));
   
   if(popi->cb_func)
     {
        if (popi->data)
          popi->cb_func(popi->data);
        else
          popi->cb_func(popi->pi);
     }

   photo_popup_info_del(popi);
}

static Eina_Bool
_cb_timer(void *data)
{
   Popup_Info *popi;

   popi = data;
   _close(popi);

   return EINA_FALSE;
}

static void
_cb_edje_close(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Popup_Info *popi;

   popi = data;
   _close(popi);
}
