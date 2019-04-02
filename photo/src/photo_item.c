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
#define IMPORT_STRETCH          0
#define IMPORT_TILE             1
#define IMPORT_CENTER           2
#define IMPORT_SCALE_ASPECT_IN  3
#define IMPORT_SCALE_ASPECT_OUT 4
#define IMPORT_PAN              5

typedef struct _Photo_Exe_Data Photo_Exe_Data;
struct _Photo_Exe_Data
{   
   Ecore_End_Cb          ok;
};
Ecore_Event_Handler  *exe_handler = NULL;
static Picture *_picture_new_get(Photo_Item *pi);
static void     _picture_detach(Photo_Item *pi, int part);

static Eina_Bool      _cb_timer_change(void *data);
static void     _cb_edje_change(void *data, Evas_Object *obj, const char *emission, const char *source);
static void     _cb_edje_mouseover(void *data, Evas_Object *obj, const char *emission, const char *source);
static void     _cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void     _cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void     _cb_mouse_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void     _cb_popi_close(void *data);

static const char*    _edj_gen(const char *path, Eina_Bool external, int quality, int method, Evas *evas, Ecore_End_Cb ok);
static void           _cb_import_ok(void *data, void *dia);
static Eina_Bool      _cb_edje_cc_exit(void *data, int type, void *event);
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
     E_FN_DEL(ecore_timer_del,pi->timer);

   if (pi->popw)
     photo_popup_warn_del(pi->popw);
   if (pi->popi)
     photo_popup_info_del(pi->popi);

   if (pi->timer)
     E_FN_DEL(ecore_timer_del,pi->timer);

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
        if (pi->timer) E_FN_DEL(ecore_timer_del,pi->timer);
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
        file = _edj_gen(file, 0, 100, IMPORT_SCALE_ASPECT_OUT, pi->gcc->gadcon->evas, (Ecore_End_Cb) _cb_import_ok);
        sleep(1);
        while (e_config->desktop_backgrounds)
          {
             E_Config_Desktop_Background *cfbg;

             cfbg = e_config->desktop_backgrounds->data;
             e_bg_del(cfbg->container, cfbg->zone, cfbg->desk_x, cfbg->desk_y);
          }
        e_bg_default_set(file);

        eina_stringshare_del(file);
        return 1;
     }
   
    DITEM(("Set edje background %s", file));
    // FIX ME: This is broken below
    // Uses an old version of e17 enlightenment_remote no longer has such an option
    //   altho I have thought about adding it. regardless should use native e code here
	snprintf(buf, 4096, "enlightenment_remote -default-bg-set \"%s\"", file);
	exe = ecore_exe_pipe_run(buf, ECORE_EXE_USE_SH, NULL);
	if (exe > 0)
	  {
	    ecore_exe_free(exe);
	    if (photo->config->pictures_set_bg_purge)
	      photo_picture_setbg_add(name);
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

static const char *
_edj_gen(const char *path, Eina_Bool external, int quality, int method, Evas* evas, Ecore_End_Cb ok)
{
   Evas_Object *img;
   Eina_Bool anim = EINA_FALSE;
   int fd, num = 1;
   int w = 0, h = 0;
   const char *file, *locale;
   char buf[PATH_MAX], cmd[PATH_MAX], tmpn[PATH_MAX], ipart[PATH_MAX], enc[128];
   char *imgdir = NULL, *fstrip;
   int cr, cg, cb, ca;
   FILE *f;
   size_t len, off;
   Ecore_Exe            *exe;
   const char *ret;
   Photo_Exe_Data *id;
   
   id = E_NEW(Photo_Exe_Data, 1);
   if (!id) return NULL;
   id->ok = ok;
   
   file = ecore_file_file_get(path);
   fstrip = ecore_file_strip_ext(file);
   if (!fstrip) return NULL;
   len = e_user_dir_snprintf(buf, sizeof(buf), "backgrounds/%s.edj", fstrip);
   if (len >= sizeof(buf))
     {
        free(fstrip);
        return NULL;
     }
   off = len - (sizeof(".edj") - 1);
   for (num = 1; ecore_file_exists(buf) && num < 100; num++)
     snprintf(buf + off, sizeof(buf) - off, "-%d.edj", num);
   free(fstrip);
   cr = 0;
   cg = 0;
   cb = 0;
   ca = 0;

   if (num == 100)
     {
        printf("Couldn't come up with another filename for %s\n", buf);
        return NULL;
     }

   strcpy(tmpn, "/tmp/e_bgdlg_new.edc-tmp-XXXXXX");
   fd = mkstemp(tmpn);
   if (fd < 0)
     {
        printf("Error Creating tmp file: %s\n", strerror(errno));
        return NULL;
     }

   f = fdopen(fd, "w");
   if (!f)
     {
        printf("Cannot open %s for writing\n", tmpn);
        return NULL;
     }

   anim = eina_str_has_extension(path, "gif");
   imgdir = ecore_file_dir_get(path);
   if (!imgdir) ipart[0] = '\0';
   else
     {
        snprintf(ipart, sizeof(ipart), "-id %s", e_util_filename_escape(imgdir));
        free(imgdir);
     }

   img = evas_object_image_add(evas);
   evas_object_image_file_set(img, path, NULL);
   evas_object_image_size_get(img, &w, &h);
   evas_object_del(img);

   if (external)
     {
        fstrip = strdupa(e_util_filename_escape(path));
        snprintf(enc, sizeof(enc), "USER");
     }
   else
     {
        fstrip = strdupa(e_util_filename_escape(file));
        if (quality == 100)
          snprintf(enc, sizeof(enc), "COMP");
        else
          snprintf(enc, sizeof(enc), "LOSSY %i", quality);
     }
   switch (method)
     {
      case IMPORT_STRETCH:
        fprintf(f,
                "images { image: \"%s\" %s; }\n"
                "collections {\n"
                "group { name: \"e/desktop/background\";\n"
                "%s"
                "data { item: \"style\" \"0\"; }\n"
                "max: %i %i;\n"
                "parts {\n"
                "part { name: \"bg\"; mouse_events: 0;\n"
                "description { state: \"default\" 0.0;\n"
                "image { normal: \"%s\"; scale_hint: STATIC; }\n"
                "} } } } }\n"
                , fstrip, enc, anim ? "" : "data.item: \"noanimation\" \"1\";\n", w, h, fstrip);
        break;

      case IMPORT_TILE:
        fprintf(f,
                "images { image: \"%s\" %s; }\n"
                "collections {\n"
                "group { name: \"e/desktop/background\";\n"
                "data { item: \"style\" \"1\"; }\n"
                "%s"
                "max: %i %i;\n"
                "parts {\n"
                "part { name: \"bg\"; mouse_events: 0;\n"
                "description { state: \"default\" 0.0;\n"
                "image { normal: \"%s\"; }\n"
                "fill { size {\n"
                "relative: 0.0 0.0;\n"
                "offset: %i %i;\n"
                "} } } } } } }\n"
                , fstrip, enc, anim ? "" : "data.item: \"noanimation\" \"1\";\n", w, h, fstrip, w, h);
        break;

      case IMPORT_CENTER:
        fprintf(f,
                "images { image: \"%s\" %s; }\n"
                "collections {\n"
                "group { name: \"e/desktop/background\";\n"
                "data { item: \"style\" \"2\"; }\n"
                "%s"
                "max: %i %i;\n"
                "parts {\n"
                "part { name: \"col\"; type: RECT; mouse_events: 0;\n"
                "description { state: \"default\" 0.0;\n"
                "color: %i %i %i %i;\n"
                "} }\n"
                "part { name: \"bg\"; mouse_events: 0;\n"
                "description { state: \"default\" 0.0;\n"
                "min: %i %i; max: %i %i;\n"
                "image { normal: \"%s\"; }\n"
                "} } } } }\n"
                , fstrip, enc, anim ? "" : "data.item: \"noanimation\" \"1\";\n", w, h, cr, cg, cb, ca, w, h, w, h, fstrip);
        break;

      case IMPORT_SCALE_ASPECT_IN:
        locale = e_intl_language_get();
        setlocale(LC_NUMERIC, "C");
        fprintf(f,
                "images { image: \"%s\" %s; }\n"
                "collections {\n"
                "group { name: \"e/desktop/background\";\n"
                "data { item: \"style\" \"3\"; }\n"
                "%s"
                "max: %i %i;\n"
                "parts {\n"
                "part { name: \"col\"; type: RECT; mouse_events: 0;\n"
                "description { state: \"default\" 0.0;\n"
                "color: %i %i %i %i;\n"
                "} }\n"
                "part { name: \"bg\"; mouse_events: 0;\n"
                "description { state: \"default\" 0.0;\n"
                "aspect: %1.9f %1.9f; aspect_preference: BOTH;\n"
                "image { normal: \"%s\";  scale_hint: STATIC; }\n"
                "} } } } }\n"
                , fstrip, enc, anim ? "" : "data.item: \"noanimation\" \"1\";\n",
                w, h, cr, cg, cb, ca, (double)w / (double)h, (double)w / (double)h, fstrip);
        setlocale(LC_NUMERIC, locale);
        break;

      case IMPORT_SCALE_ASPECT_OUT:
        locale = e_intl_language_get();
        setlocale(LC_NUMERIC, "C");
        fprintf(f,
                "images { image: \"%s\" %s; }\n"
                "collections {\n"
                "group { name: \"e/desktop/background\";\n"
                "data { item: \"style\" \"4\"; }\n"
                "%s"
                "max: %i %i;\n"
                "parts {\n"
                "part { name: \"bg\"; mouse_events: 0;\n"
                "description { state: \"default\" 0.0;\n"
                "aspect: %1.9f %1.9f; aspect_preference: NONE;\n"
                "image { normal: \"%s\";  scale_hint: STATIC; }\n"
                "} } } } }\n"
                , fstrip, enc, anim ? "" : "data.item: \"noanimation\" \"1\";\n",
                w, h, (double)w / (double)h, (double)w / (double)h, fstrip);
        setlocale(LC_NUMERIC, locale);
        break;

      case IMPORT_PAN:
        locale = e_intl_language_get();
        setlocale(LC_NUMERIC, "C");
        fprintf(f,
                "images { image: \"%s\" %s; }\n"
                "collections {\n"
                "group { name: \"e/desktop/background\";\n"
                "data { item: \"style\" \"4\"; }\n"
                "%s"
                "max: %i %i;\n"
                "script {\n"
                "public cur_anim; public cur_x; public cur_y; public prev_x;\n"
                "public prev_y; public total_x; public total_y; \n"
                "public pan_bg(val, Float:v) {\n"
                "new Float:x, Float:y, Float:px, Float: py;\n"

                "px = get_float(prev_x); py = get_float(prev_y);\n"
                "if (get_int(total_x) > 1) {\n"
                "x = float(get_int(cur_x)) / (get_int(total_x) - 1);\n"
                "x = px - (px - x) * v;\n"
                "} else { x = 0.0; v = 1.0; }\n"
                "if (get_int(total_y) > 1) {\n"
                "y = float(get_int(cur_y)) / (get_int(total_y) - 1);\n"
                "y = py - (py - y) * v;\n"
                "} else { y = 0.0; v = 1.0; }\n"

                "set_state_val(PART:\"bg\", STATE_ALIGNMENT, x, y);\n"

                "if (v >= 1.0) {\n"
                "set_int(cur_anim, 0); set_float(prev_x, x);\n"
                "set_float(prev_y, y); return 0;\n"
                "}\n"
                "return 1;\n"
                "}\n"
                "public message(Msg_Type:type, id, ...) {\n"
                "if ((type == MSG_FLOAT_SET) && (id == 0)) {\n"
                "new ani;\n"

                "get_state_val(PART:\"bg\", STATE_ALIGNMENT, prev_x, prev_y);\n"
                "set_int(cur_x, round(getfarg(3))); set_int(total_x, round(getfarg(4)));\n"
                "set_int(cur_y, round(getfarg(5))); set_int(total_y, round(getfarg(6)));\n"

                "ani = get_int(cur_anim); if (ani > 0) cancel_anim(ani);\n"
                "ani = anim(getfarg(2), \"pan_bg\", 0); set_int(cur_anim, ani);\n"
                "} } }\n"
                "parts {\n"
                "part { name: \"bg\"; mouse_events: 0;\n"
                "description { state: \"default\" 0.0;\n"
                "aspect: %1.9f %1.9f; aspect_preference: NONE;\n"
                "image { normal: \"%s\";  scale_hint: STATIC; }\n"
                "} } }\n"
                "programs { program {\n"
                " name: \"init\";\n"
                " signal: \"load\";\n"
                " source: \"\";\n"
                " script { custom_state(PART:\"bg\", \"default\", 0.0);\n"
                " set_state(PART:\"bg\", \"custom\", 0.0);\n"
                " set_float(prev_x, 0.0); set_float(prev_y, 0.0);\n"
                "} } } } }\n"
                , fstrip, enc, anim ? "" : "data.item: \"noanimation\" \"1\";\n",
                w, h, (double)w / (double)h, (double)w / (double)h, fstrip);
        setlocale(LC_NUMERIC, locale);
        break;

      default:
        /* won't happen */
        break;
     }

   fclose(f);

   snprintf(cmd, sizeof(cmd), "edje_cc -v %s %s %s",
            ipart, tmpn, e_util_filename_escape(buf));

   if (exe_handler) ecore_event_handler_del(exe_handler);
   exe_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                             _cb_edje_cc_exit, id);
   exe = ecore_exe_run(cmd, id);
   ret = eina_stringshare_add(buf);
   return ret;
}

static void
_cb_import_ok(void *data, void *dia)
{
   if (exe_handler)
     {
       ecore_event_handler_del(exe_handler);
       exe_handler = NULL;
     }
   e_bg_update();
   e_config_save_queue();
}

static Eina_Bool
_cb_edje_cc_exit(void *data, int type, void *event)
{
   Photo_Exe_Data *import;
   Ecore_Exe_Event_Del *ev;
   int r = 1;

   ev = event;
   import = data;
   if (ecore_exe_data_get(ev->exe) != import) return ECORE_CALLBACK_PASS_ON;

   if (ev->exit_code != 0)
     {
        e_util_dialog_show(D_("Picture Import Error"),
                           D_("Moksha was unable to import the picture<br>"
                             "due to conversion errors."));
        r = 0;
     }

   if (r && import->ok)
     {
        import->ok(NULL, import);
        E_FREE(import);
     }
   else
     E_FREE(import);
   return ECORE_CALLBACK_DONE;
}
