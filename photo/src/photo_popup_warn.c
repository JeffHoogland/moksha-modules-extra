#include "Photo.h"

static Eina_List *_popups_warn;

static void _check_overlap(int *px, int *py, int *pw, int *ph, int tries, int org_x, int org_y);
static void _try_close(Popup_Warn *popw);

static Eina_Bool  _cb_timer(void *data);
static void _cb_edje_close(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _cb_edje_desactivate(void *data, Evas_Object *obj, const char *emission, const char *source);
/*
 * Public functions
 */

int photo_popup_warn_init(void)
{
   _popups_warn = NULL;

   return 1;
}

void photo_popup_warn_shutdown(void)
{
   Eina_List *l;

   for (l = _popups_warn; l; l = eina_list_next(l))
     {
        Popup_Warn *p;
        p = eina_list_data_get(l);
        if (p->func_close)
          p->func_close(p, p->data);
        photo_popup_warn_del(p);
     }
   eina_list_free(_popups_warn);
   _popups_warn = NULL;
}

Popup_Warn *photo_popup_warn_add(int type, const char *text, int timer, int (*func_close) (Popup_Warn *popw, void *data), void (*func_desactivate) (Popup_Warn *popw, void *data), void *data)
{
  E_Zone *zone;
  Popup_Warn *popw;
  int fw, fh;

  popw = E_NEW(Popup_Warn, 1);

  popw->type = type;
  popw->timer_org = timer;

  zone = e_util_zone_current_get(e_manager_current_get());

  /* pop */
  popw->pop = e_popup_new(zone, 0, 0, 1, 1);
  if (!popw->pop)
    {
      photo_popup_warn_del(popw);
      return 0;
    }
  evas_event_freeze(popw->pop->evas);
  e_popup_layer_set(popw->pop, 255);



  /* face and text */
  popw->face = edje_object_add(popw->pop->evas);
  photo_util_edje_set(popw->face, PHOTO_THEME_POPW);
  edje_object_signal_callback_add(popw->face, "close", "popup",
				  _cb_edje_close, popw);
  edje_object_signal_callback_add(popw->face, "desactivate", "popup",
				  _cb_edje_desactivate, popw);
  edje_object_part_text_set(popw->face, "text", text);
  edje_object_part_geometry_get(popw->face, "text_border",
				NULL, NULL, &fw, NULL);
  edje_object_part_geometry_get(popw->face, "background",
				NULL, NULL, NULL, &fh);
  evas_object_resize(popw->face, fw, fh);
  evas_object_move(popw->face, 0, 0);

  /* type */
  edje_object_message_send(popw->face, EDJE_MESSAGE_INT,
			   POPUP_WARN_EDJE_MESSAGE_TYPE,
			   &type);

  /* pos */
  popw->x = photo->canvas_w - (fw + 20);;
  popw->y = photo->canvas_h - (fh + 20);;
  popw->w = fw;
  popw->h = fh;

  /* timer */
  if (timer)
    popw->timer = ecore_timer_add(timer, _cb_timer, popw);

  /* close and desactivate functions */
  popw->func_close = func_close;
  if (func_desactivate)
    {
      int show_desactivate = 1;

      popw->func_desactivate = func_desactivate;
      edje_object_message_send(popw->face, EDJE_MESSAGE_INT,
			       POPUP_WARN_EDJE_MESSAGE_SHOW_DESACTIVATE,
			       &show_desactivate);
    }

  /* attach data */
  popw->data = data;

  /* check for popup overlaps */
  _check_overlap(&popw->x, &popw->y, &popw->w, &popw->h, 0, popw->x, popw->y);
  e_popup_move_resize(popw->pop, popw->x, popw->y, popw->w, popw->h);
  DPOPW(("New: %dx%d : %dx%d", popw->x, popw->y, popw->w, popw->h));
  DPOPW(("New face: %dx%d", fw, fh));

  /* go ! */
  evas_object_show(popw->face);
  e_popup_edje_bg_object_set(popw->pop, popw->face);
  evas_event_thaw(popw->pop->evas);
  e_popup_show(popw->pop);
  
  _popups_warn = eina_list_append(_popups_warn, popw);
	
   return popw;
}

void photo_popup_warn_del(Popup_Warn *popw)
{
  if (popw->timer)
    ecore_timer_del(popw->timer);
  if (popw->face)
    evas_object_del(popw->face);
  if (popw->pop)
    e_object_del(E_OBJECT(popw->pop));

  _popups_warn = eina_list_remove(_popups_warn, popw);

  free(popw);
}


/*
 * Private functions
 *
 */

static void
_check_overlap(int *px, int *py, int *pw, int *ph, int tries, int org_x, int org_y)
{
   Eina_List *l;
   Popup_Warn *p;
   int pxw, pyh;
   int p_xw, p_yh;

   pxw = *px + *pw;
   pyh = *py + *ph;
   for (l = _popups_warn; l; l = eina_list_next(l))
     {
        p = eina_list_data_get(l);
        p_xw = p->x + p->w;
        p_yh = p->y + p->h;
        if (((p->x >= *px) && (p->x <= pxw) &&
             (p->y >= *py) && (p->y <= pyh))
            ||
            ((*px >= p->x) && (*px <= p_xw) &&
             (*py >= p->y) && (*py <= p_yh)))
          {
             /* overlap ! correct coords */
             /* try upper, and then on the left */
             /* TODO...: try down and right, maybe placement policy ? */
             DPOPW(("Overlap !"));
             *py = p->y - (*ph + POPUP_WARN_OVERLAP_BORDER);
             if (*py < 0)
               {
                  *py = org_y;
                  *px = *px - (*px + POPUP_WARN_OVERLAP_BORDER);
                  if (*px < 0) break;
               }
             tries++;
             if (tries > POPUP_WARN_OVERLAP_CHECK_MAX)
               return;
             else
               _check_overlap(px, py, pw, ph, tries, org_x, org_y);
             break;
          }
     }
}

static void
_try_close(Popup_Warn *popw)
{
  int del = 1;

   if (popw->func_close)
     {
       if (!popw->func_close(popw, popw->data))
	 del = 0;
     }

   if (del)
     photo_popup_warn_del(popw);
}

static Eina_Bool
_cb_timer(void *data)
{
   Popup_Warn *popw;

   popw = data;
   _try_close(popw);

   return EINA_FALSE;
}

static void
_cb_edje_close(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Popup_Warn *popw;

   popw = data;
   _try_close(popw);
}

static void
_cb_edje_desactivate(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Popup_Warn *popw;

   popw = data;
   if (popw->func_desactivate)
     popw->func_desactivate(popw, popw->data);
}
