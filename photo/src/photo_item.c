#include "Photo.h"

#define PIC_LOCAL_GET()     \
from_final = PICTURE_LOCAL; \
picture = photo_picture_local_get(PICTURE_LOCAL_GET_RANDOM);
#define PIC_NET_GET()       \
from_final = PICTURE_NET;
//picture = photo_picture_net_get(PICTURE_NET_GET_RANDOM); //add end line backslash upstairs =)

#define ITEM_ACTION_CALL(var, parent)                                       \
if (UTIL_TEST_PARENT(var, parent, ITEM_ACTION_LABEL , ITEM_ACTION_PARENT))  \
  photo_item_action_label(pi);                                              \
if (UTIL_TEST_PARENT(var, parent, ITEM_ACTION_INFOS , ITEM_ACTION_PARENT))  \
  photo_item_action_infos(pi);                                              \
if (UTIL_TEST_PARENT(var, parent, ITEM_ACTION_PREV , ITEM_ACTION_PARENT))   \
{                                                                           \
   photo_item_action_change(pi, -1);                                        \
   photo_item_timer_set(pi, pi->config->timer_active, 0);                   \
}                                                                           \
if (UTIL_TEST_PARENT(var, parent, ITEM_ACTION_NEXT , ITEM_ACTION_PARENT))   \
{                                                                           \
   photo_item_action_change(pi, 1);                                         \
   photo_item_timer_set(pi, pi->config->timer_active, 0);                   \
}                                                                           \
if (UTIL_TEST_PARENT(var, parent, ITEM_ACTION_PAUSE , ITEM_ACTION_PARENT))  \
  photo_item_action_pause_toggle(pi);                                       \
if (UTIL_TEST_PARENT(var, parent, ITEM_ACTION_SETBG , ITEM_ACTION_PARENT))  \
  photo_item_action_setbg(pi);                                              \
if (UTIL_TEST_PARENT(var, parent, ITEM_ACTION_VIEWER , ITEM_ACTION_PARENT)) \
  photo_item_action_viewer(pi);                                             \
if (UTIL_TEST_PARENT(var, parent, ITEM_ACTION_MENU , ITEM_ACTION_PARENT))   \
  photo_item_action_menu(pi, NULL);

#define ITEM_TRANSITION_GO(way)                                  \
if ( !photo->config->nice_trans )                                \
{                                                                \
   edje_object_signal_emit(pi->obj,                              \
            STRINGIFY(picture_transition_q_ ## way ## _go), ""); \
}                                                                \
else                                                             \
{                                                                \
   edje_object_signal_emit(pi->obj,                              \
            STRINGIFY(picture_transition_ ## way ## _go), "");   \
}

#define STRINGIFY(str) #str


static Picture *_picture_new_get(Photo_Item *pi);
static void     _picture_detach(Photo_Item *pi, int part);

static Eina_Bool      _cb_timer_change(void *data);
static void     _cb_edje_change(void *data, Evas_Object *obj, const char *emission, const char *source);
static void     _cb_edje_mouseover(void *data, Evas_Object *obj, const char *emission, const char *source);
static void     _cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void     _cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void     _cb_mouse_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void     _cb_popi_close(void *data);


/*
 * Public functions
 */

Photo_Item *photo_item_add(E_Gadcon_Client *gcc, Evas_Object *obj, const char *id)
{
   Photo_Item *pi;
   Photo_Config_Item *pic;

   pi = E_NEW(Photo_Item, 1);

   photo_util_edje_set(obj, PHOTO_THEME_ITEM);

   pic = photo_config_item_new(id);
   pi->gcc = gcc;
   pi->obj = obj;
   pi->config = pic;

   if(!pic)
     {
        DD(("Item add : NULL config !!!"));
     }

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
				  _cb_mouse_down, pi);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_OUT,
				  _cb_mouse_out, pi);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_WHEEL,
                                  _cb_mouse_wheel, pi);

   edje_object_signal_callback_add(obj,
                                   "mouseover", "item",
                                   _cb_edje_mouseover, pi);
   edje_object_signal_callback_add(obj, "picture_transition_0_1_end", "picture",
                                   _cb_edje_change, pi);
   edje_object_signal_callback_add(obj, "picture_transition_1_0_end", "picture",
                                   _cb_edje_change, pi);

   photo_item_timer_set(pi, pic->timer_active, 0);
   evas_object_color_set(pi->obj, 255, 255, 255, pic->alpha);
   photo_item_label_mode_set(pi);
   photo_picture_histo_init(pi);

   photo_item_action_change(pi, 1);

   return pi;
}

void photo_item_del(Photo_Item *pi)
{
   DITEM(("Item del"));
   
   if (pi->picture0)
     _picture_detach(pi, 0);
   if (pi->picture1)
     _picture_detach(pi, 1);

   evas_object_del(pi->obj);

   photo_config_item_free(pi->config);
   if (pi->config_dialog)
     photo_config_dialog_item_hide(pi);
   if (pi->menu)
     photo_menu_hide(pi);

   if (pi->timer)
     ecore_timer_del(pi->timer);

   if (pi->popw)
     photo_popup_warn_del(pi->popw);
   if (pi->popi)
     photo_popup_info_del(pi->popi);

   if (pi->timer)
     ecore_timer_del(pi->timer);

   photo_picture_histo_shutdown(pi);

   if (pi->local_ev_fill_handler)
     ecore_event_handler_del(pi->local_ev_fill_handler);
   if (pi->net_ev_fill_handler)
     ecore_event_handler_del(pi->net_ev_fill_handler);

   free(pi);
}

void photo_item_timer_set(Photo_Item *pi, int active, int time)
{
   if (time && (time < ITEM_TIMER_S_MIN))
     return;

   pi->config->timer_active = active;
   if (!time)
     time = pi->config->timer_s;
   else
     pi->config->timer_s = time;

   photo_config_save();

   if (!active)
     {
        if (pi->timer)
          {
             ecore_timer_del(pi->timer);
             pi->timer = NULL;
          }
        return;
     }

   if (pi->timer)
     ecore_timer_del(pi->timer);
   pi->timer = ecore_timer_add(time, _cb_timer_change, pi);
}

void photo_item_label_mode_set(Photo_Item *pi)
{
   Photo_Config_Item *pic;
   Eina_List *l = NULL;
   int action;

   DD(("Mode set (%d items)", eina_list_count(photo->items)));

   if (!pi)
     {
        l = photo->items;
        pi = eina_list_data_get(l);
     }

   do
     {
        pic = pi->config;
  
        if ( UTIL_TEST_PARENT(pic->show_label, photo->config->show_label,
                              ITEM_SHOW_LABEL_YES, ITEM_SHOW_LABEL_PARENT) )
          action = 1;
        else
          action = 0;
       
        edje_object_message_send(pi->obj, EDJE_MESSAGE_INT,
                                 ITEM_EDJE_MSG_SEND_LABEL_ALWAYS,
                                 &action);

        DITEM(("Set label mode %d", action));
       
        if (action)
          photo_item_action_label(pi);
     } while ( l && (l = eina_list_next(l)) && (pi = eina_list_data_get(l)) );
}

Picture *photo_item_picture_current_get(Photo_Item *pi)
{
   Picture *p = NULL;

   if (!pi->edje_part && pi->picture0)
     {
        p = pi->picture0;
     }
   if (pi->edje_part && pi->picture1)
     {
        p = pi->picture1;
     }

   return p;
}

Evas_Object *photo_item_picture_object_current_get(Photo_Item *pi)
{
   Evas_Object *p = NULL;

   if (!pi->edje_part && pi->picture0)
     {
        p = pi->picture0->picture;
     }
   if (pi->edje_part && pi->picture1)
     {
        p = pi->picture1->picture;
     }

   return p;
}

int photo_item_action_label(Photo_Item *pi)
{
   DITEM(("Label show emit !"));

   edje_object_signal_emit(pi->obj, "label_show", "program");

   return 1;
}

int photo_item_action_infos(Photo_Item *pi)
{
   Picture *p;
   char *text;

   DITEM(("Action info go"));

   p = photo_item_picture_current_get(pi);
   if (!p) return 0;

   text = photo_picture_infos_get(p);

   if (pi->popi)
     photo_popup_info_del(pi->popi);

   pi->popi = photo_popup_info_add(pi, p->infos.name, text, p,
                                   ITEM_INFOS_TIMER,
                                   POPUP_INFO_PLACEMENT_SHELF,
                                   _cb_popi_close, NULL);

   free(text);

   return 0;
}

int photo_item_action_change(Photo_Item *pi, int position)
{
   Picture *picture;

   DITEM(("picture change %d", position));

   if (!position)
     return 0;

   /* 1. get the picture to change to */

   if ( (position < 0) || (pi->histo.pos) )
     {
        /* from histo */

        picture = photo_picture_histo_change(pi, -position);
        if (!picture) /* should no append */
          {
             DITEM(("Action change : Histo get NULL !!!"));
             return 0;
          }
     }
   else
     {
        /* from list */

        /* if already waiting for pictures, dont change */
        if (pi->local_ev_fill_handler || pi->net_ev_fill_handler)
          return 0;

        picture = _picture_new_get(pi);
        if (!picture)
          return 0;

        photo_picture_histo_attach(pi, picture);
     }

   /* 2. load it */

   photo_picture_load(picture, pi->gcc->gadcon->evas);
   picture->pi = pi;

   /* 3. set the label */

   edje_object_part_text_set(pi->obj, "label", picture->infos.name);

   /* 4. transition to the new picture */

   if (pi->in_transition)
     {
        DITEM(("Already in transition, restarting =)"));
        _picture_detach(pi, !pi->edje_part);
     }

   pi->in_transition = 1;
   if (!pi->edje_part)
     {
        pi->edje_part = 1;
        pi->picture1 = picture;
        edje_object_part_swallow(pi->obj, "picture1",
                                 pi->picture1->picture);
        evas_object_show(pi->picture1->picture);
        ITEM_TRANSITION_GO(0_1);
     }
   else
     {
        pi->edje_part = 0;
        pi->picture0 = picture;
        edje_object_part_swallow(pi->obj, "picture0",
                                 pi->picture0->picture);
        evas_object_show(pi->picture0->picture);
        ITEM_TRANSITION_GO(1_0);
     }

   /* 5. if there were a popup info, update it */

   if (pi->popi)
     photo_item_action_infos(pi);

   return 1;
}

int photo_item_action_pause_toggle(Photo_Item *pi)
{
   photo_item_timer_set(pi, !pi->config->timer_active, 0);

   return 1;
}

int  photo_item_action_setbg(Photo_Item *pi)
{
  Picture *p;
   E_Zone *zone;
   Ecore_Exe *exe;
   const char *file;
   const char *name;
   char buf[4096];

   zone = e_zone_current_get(e_container_current_get(e_manager_current_get()));
   if (!zone) return 0;

   p = photo_item_picture_current_get(pi);
   if (!p) return 0;

   file = p->path;
   name = p->infos.name;

   if (photo->config->pictures_set_bg_purge)
     photo_picture_setbg_purge(0);

   if (!ecore_file_exists(file))
     {
        snprintf(buf, sizeof(buf),
                 D_("<hilight>File %s doesn't exist.</hilight><br><br>"
                   "This file is in the picture list, but it seems you removed<br>"
                   "it from disk. It can't be set as background, sorry."), file);
        e_module_dialog_show(photo->module, D_("Photo Module Error"), buf);
        return 0;
     }

   if (!strstr(file, ".edj"))
     {
        if (ecore_file_app_installed("e17setroot"))
          {
             snprintf(buf, 4096, "e17setroot -s \"%s\"", file);
             DITEM(("Set background with %s", buf));
	     exe = ecore_exe_pipe_run(buf, ECORE_EXE_USE_SH, NULL);
             if (exe > 0)
               {
                  ecore_exe_free(exe);
                  if (photo->config->pictures_set_bg_purge)
                    photo_picture_setbg_add(name);
               }
          }
        else
          {
             snprintf(buf, sizeof(buf),
                      D_("<hilight>e17setroot needed</hilight><br><br>"
                        "%s is not an edje file!<br>"
                        "Photo module needs e17setroot util from e_utils package to set your picture as background. "
                        "Please install it and try again."), file);
             e_module_dialog_show(photo->module, D_("Photo Module Error"), buf);
             return 0;
          }
     }
   else
     {
        DITEM(("Set edje background %s", file));

	snprintf(buf, 4096, "enlightenment_remote -default-bg-set \"%s\"", file);
	exe = ecore_exe_pipe_run(buf, ECORE_EXE_USE_SH, NULL);
	if (exe > 0)
	  {
	    ecore_exe_free(exe);
	    if (photo->config->pictures_set_bg_purge)
	      photo_picture_setbg_add(name);
	  }
     }

   return 1;
}

int photo_item_action_viewer(Photo_Item *pi)
{
  Picture *p;
   const char *file = NULL;
   char buf[4096];

   p = photo_item_picture_current_get(pi);
   if (!p) return 0;

   file = p->path;

   if (!ecore_file_exists(file))
     {
        snprintf(buf, sizeof(buf),
                 D_("<hilight>File %s doesn't exist.</hilight><br><br>"
                   "This file is in the picture list, but it seems you removed<br>"
                   "it from disk. It can't be set as background, sorry."), file);
        e_module_dialog_show(photo->module, D_("Photo Module Error"), buf);
        return 0;
     }

   if (ecore_file_app_installed(photo->config->pictures_viewer))
     {
        Ecore_Exe *exe;

        snprintf(buf, 4096, "%s \"%s\"", photo->config->pictures_viewer, file);
        DITEM(("Action viewer: %s", buf));
        exe = ecore_exe_pipe_run(buf, ECORE_EXE_USE_SH, NULL);
        if (exe > 0)
          ecore_exe_free(exe);
     }
   else
     {
        snprintf(buf, sizeof(buf),
                 D_("<hilight>Viewer %s not found !</hilight><br><br>"
                   "You can change the viewer for images in the Photo module configuration panel (Advanced view)"),
                 photo->config->pictures_viewer);
        e_module_dialog_show(photo->module, D_("Photo Module Error"), buf);
        return 0;
     }

   return 1;
}

int photo_item_action_menu(Photo_Item *pi, Evas_Event_Mouse_Down *ev)
{
   int cx, cy, cw, ch;

   if (pi->menu) return 0;

   if (!photo_menu_show(pi))
     return 0;

   if (ev)
     {
        e_gadcon_canvas_zone_geometry_get(pi->gcc->gadcon,
                                          &cx, &cy, &cw, &ch);
        e_menu_activate_mouse(pi->menu,
                              e_util_zone_current_get(e_manager_current_get()),
                              cx + ev->output.x, cy + ev->output.y, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(pi->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
   else
     {
        E_Manager *man;
        man = e_manager_current_get();
        ecore_x_pointer_xy_get(man->root, &cx, &cy);
        e_menu_activate(pi->menu,
                        e_util_zone_current_get(e_manager_current_get()),
                        cx, cy, 1, 1,
                        E_MENU_POP_DIRECTION_DOWN);
     }

   return 1;
}


/*
 * Private functions
 */

Picture *_picture_new_get(Photo_Item *pi)
{
   Picture *picture;
   int from_rand, from_conf, from_final;

   from_conf = photo->config->pictures_from;
   switch (from_conf)
     {
     case PICTURE_BOTH:
        from_rand = rand()%2;
        if (!from_rand)
          { PIC_LOCAL_GET(); }
        else
          { PIC_NET_GET(); }
        if (!picture)
          {
             if (from_rand)
               { PIC_LOCAL_GET(); }
             else
               { PIC_NET_GET(); }
          }
        break;
      
     case PICTURE_LOCAL:
        PIC_LOCAL_GET();
        break;
      
     case PICTURE_NET:
        PIC_NET_GET();
        break;
     }

   /* set fill event handler to catch a picture when it comes */
   if (!picture)
     {
        DITEM(("Can't get a picture ! set fill handler"));
        switch(from_final)
          {
          case PICTURE_LOCAL:
             photo_picture_local_ev_set(pi);
             break;
          case PICTURE_NET:
             //photo_picture_net_ev_set(pi);
             break;
          }
     }

   return picture;
}

static void
_picture_detach(Photo_Item *pi, int part)
{
   Picture *picture;

   if (!part) picture = pi->picture0;
   else picture = pi->picture1;

   if (!picture) return;

   evas_object_hide(picture->picture);
   edje_object_part_unswallow(pi->obj, picture->picture);
   photo_picture_unload(picture);

   picture->pi = NULL;

   if (!part)
     pi->picture0 = NULL;
   else
     pi->picture1 = NULL;

   photo_picture_local_ev_raise(1);
}

static Eina_Bool
_cb_timer_change(void *data)
{
   Photo_Item *pi;

   pi = data;
   photo_item_action_change(pi, 1);

   return EINA_TRUE;
}

static void
_cb_edje_change(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Photo_Item *pi;

   DITEM(("Cb picture change (%s)", emission));

   pi = data;

   pi->in_transition = 0;

   if ( !strcmp(emission, "picture_transition_0_1_end") )
     {
        _picture_detach(pi, 0);
        return;
     }
   if ( !strcmp(emission, "picture_transition_1_0_end") )
     {
        _picture_detach(pi, 1);
        return;
     }
}

static void
_cb_edje_mouseover(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Photo_Item *pi;

   DITEM(("Cb edje mouseover"));

   pi = data;
   ITEM_ACTION_CALL(pi->config->action_mouse_over,
                    photo->config->action_mouse_over);
}

void _cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Photo_Item *pi;
   Evas_Event_Mouse_Down *ev;

   pi = data;
   ev = event_info;

   DITEM(("Mouse down %d", ev->button));

   switch(ev->button)
     {
     case 1:
        ITEM_ACTION_CALL(pi->config->action_mouse_left,
                         photo->config->action_mouse_left);
        break;
     case 2:
        ITEM_ACTION_CALL(pi->config->action_mouse_middle,
                         photo->config->action_mouse_middle);
        break;
     case 3:
        if (pi->menu)
          break;
        photo_item_action_menu(pi, ev);
        break;
     }
}

static void _cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Photo_Item *pi;

   pi = data;

   if (pi->popi)
     photo_popup_info_del(pi->popi);
   pi->popi = NULL;
}

static void
_cb_mouse_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   Photo_Item *pi;

   ev = event_info;
   pi = data;

   photo_item_action_change(pi, ev->z);
   photo_item_timer_set(pi, pi->config->timer_active, 0);
}

static void
_cb_popi_close(void *data)
{
   Photo_Item *pi;

   pi = data;
   pi->popi = NULL;
}
