#include "e.h"
#include "e_mod_main.h"

/* currently default bind is alt+esc but alt+space has been suggested */

/* local subsystem functions */
typedef struct _E_Exebuf_Exe E_Exebuf_Exe;
   
struct _E_Exebuf_Exe
{
   Evas_Object    *bg_object;
   Evas_Object    *icon_object;
   Efreet_Desktop *desktop;
   char           *file;
};

typedef struct _E_Exe E_Exe;
typedef struct _E_Exe_List E_Exe_List;

struct _E_Exe
{
   const char *path;
};

struct _E_Exe_List
{
   Eina_List *list;
};

static void _e_exebuf_exe_free(E_Exebuf_Exe *exe);
static void _e_exebuf_matches_clear(void);
static int _e_exebuf_cb_sort_eap(const void *data1, const void *data2);
static int _e_exebuf_cb_sort_exe(const void *data1, const void *data2);
static void _e_exebuf_update(void);
static void _e_exebuf_exec(void);
static void _e_exebuf_exec_term(void);
static void _e_exebuf_exe_sel(E_Exebuf_Exe *exe);
static void _e_exebuf_exe_desel(E_Exebuf_Exe *exe);
static void _e_exebuf_exe_scroll_to(int i);
static void _e_exebuf_eap_scroll_to(int i);
static void _e_exebuf_next(void);
static void _e_exebuf_prev(void);
static void _e_exebuf_complete(void);
static void _e_exebuf_backspace(void);
static void _e_exebuf_clear(void);
static void _e_exebuf_matches_update(void);
static void _e_exebuf_hist_update(Eina_List *hist_matches);
static void _e_exebuf_hist_clear(void);
static void _e_exebuf_cb_eap_item_mouse_in(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_exebuf_cb_eap_item_mouse_out(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_exebuf_cb_exe_item_mouse_in(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_exebuf_cb_exe_item_mouse_out(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static Eina_Bool _e_exebuf_cb_key_down(void *data, int type, void *event);
static Eina_Bool _e_exebuf_cb_mouse_down(void *data, int type, void *event);
static Eina_Bool _e_exebuf_cb_mouse_up(void *data, int type, void *event);
static Eina_Bool _e_exebuf_cb_mouse_move(void *data, int type, void *event);
static Eina_Bool _e_exebuf_cb_mouse_wheel(void *data, int type, void *event);
static Eina_Bool _e_exebuf_exe_scroll_timer(void *data);
static Eina_Bool _e_exebuf_eap_scroll_timer(void *data);
static Eina_Bool _e_exebuf_animator(void *data);
static Eina_Bool _e_exebuf_idler(void *data);
static Eina_Bool _e_exebuf_update_timer(void *data);

/* local subsystem globals */
static E_Config_DD *exelist_exe_edd = NULL;
static E_Config_DD *exelist_edd = NULL;
static E_Popup *exebuf = NULL;
static Evas_Object *bg_object = NULL;
static Evas_Object *icon_object = NULL;
static Evas_Object *exe_list_object = NULL;
static Evas_Object *eap_list_object = NULL;
static Eina_List *handlers = NULL;
static Ecore_X_Window input_window = 0;
static char *cmd_buf = NULL;
static Eina_List *history = NULL;
static Eina_List *eap_matches = NULL;
static Eina_List *exe_matches = NULL;
static Eina_List *exe_path = NULL;
static DIR       *exe_dir = NULL;
static Eina_List *exe_list = NULL;
static Eina_List *exe_list2 = NULL;
static Ecore_Idler *exe_list_idler = NULL;
static Eina_List *exes = NULL;
static Eina_List *eaps = NULL;
#define NO_LIST 0
#define EAP_LIST 1
#define EXE_LIST 2
#define HIST_LIST 3
static int which_list = NO_LIST;
static E_Exebuf_Exe *exe_sel = NULL;
static int exe_scroll_to = 0;
static double exe_scroll_align_to = 0.0;
static double exe_scroll_align = 0.0;
static Ecore_Timer *exe_scroll_timer = NULL;
static int eap_scroll_to = 0;
static double eap_scroll_align_to = 0.0;
static double eap_scroll_align = 0.0;
static Ecore_Timer *eap_scroll_timer = NULL;
static Ecore_Animator *animator = NULL;
static Ecore_Timer *update_timer = NULL;
static int ev_last_is_mouse = 1;
static E_Exebuf_Exe *ev_last_mouse_exe = NULL;
static int ev_last_which_list = NO_LIST;

#define MATCH_LAG 0.33
#define EXEBUFLEN 2048

/* externally accessible functions */
int
e_exebuf_init(void)
{
   exelist_exe_edd = E_CONFIG_DD_NEW("E_Exe", E_Exe);
#undef T
#undef D
#define T E_Exe
#define D exelist_exe_edd
   E_CONFIG_VAL(D, T, path, STR);
   
   exelist_edd = E_CONFIG_DD_NEW("E_Exe_List", E_Exe_List);
#undef T
#undef D
#define T E_Exe_List
#define D exelist_edd
   E_CONFIG_LIST(D, T, list, exelist_exe_edd);
   
   return 1;
}

int
e_exebuf_shutdown(void)
{
   E_CONFIG_DD_FREE(exelist_edd);
   E_CONFIG_DD_FREE(exelist_exe_edd);
   e_exebuf_hide();
   return 1;
}

int
e_exebuf_show(E_Zone *zone)
{
   Evas_Object *o;
   int x, y, w, h;
   Evas_Coord mw, mh;
   char *path, *p, *last;
   E_Exe_List *el;
   
   E_OBJECT_CHECK_RETURN(zone, 0);
   E_OBJECT_TYPE_CHECK_RETURN(zone, E_ZONE_TYPE, 0);

   if (exebuf) return 0;

   input_window = ecore_x_window_input_new(zone->container->win, zone->x,
	 zone->y, zone->w, zone->h);
   ecore_x_window_show(input_window);
   if (!e_grabinput_get(input_window, 1, input_window))
     {
        ecore_x_window_free(input_window);
	input_window = 0;
	return 0;
     }

   x = zone->x + 20;
   y = zone->y + 20 + ((zone->h - 20 - 20 - 20) / 2);
   w = zone->w - 20 - 20;
   h = 20;
   
   exebuf = e_popup_new(zone, x, y, w, h); 
   if (!exebuf) return 0;

   cmd_buf = malloc(EXEBUFLEN);
   if (!cmd_buf)
     {
	e_object_del(E_OBJECT(exebuf));
	return 0;
     }
   cmd_buf[0] = 0;

   e_popup_layer_set(exebuf, 255);
   evas_event_freeze(exebuf->evas);
   evas_event_feed_mouse_in(exebuf->evas, ecore_x_current_time_get(), NULL);
   evas_event_feed_mouse_move(exebuf->evas, -1000000, -1000000, ecore_x_current_time_get(), NULL);
   o = edje_object_add(exebuf->evas);
   bg_object = o;
   e_theme_edje_object_set(o, "base/theme/exebuf",
			   "e/widgets/exebuf/main");
   edje_object_part_text_set(o, "e.text.label", cmd_buf);
   
   o = e_box_add(exebuf->evas);
   exe_list_object = o;
   e_box_align_set(o, 0.5, 1.0);
   e_box_orientation_set(o, 0);
   e_box_homogenous_set(o, 1);
   edje_object_part_swallow(bg_object, "e.swallow.exe_list", o);
   evas_object_show(o);
   
   o = e_box_add(exebuf->evas);
   eap_list_object = o;
   e_box_align_set(o, 0.5, 0.0);
   e_box_orientation_set(o, 0);
   e_box_homogenous_set(o, 1);
   edje_object_part_swallow(bg_object, "e.swallow.eap_list", o);
   evas_object_show(o);
   
   o = bg_object;
   edje_object_size_min_calc(o, &mw, &mh);
   
   w = (double)zone->w * exebuf_conf->pos_size_w;
   if (w > exebuf_conf->pos_max_w) w = exebuf_conf->pos_max_w;
   else if (w < exebuf_conf->pos_min_w) w = exebuf_conf->pos_min_w;
   if (w < mw) w = mw;
   if (w > zone->w) w = zone->w;
   x = (double)(zone->w - w) * exebuf_conf->pos_align_x;
   
   h = (double)zone->h * exebuf_conf->pos_size_h;
   if (h > exebuf_conf->pos_max_h) h = exebuf_conf->pos_max_h;
   else if (h < exebuf_conf->pos_min_h) h = exebuf_conf->pos_min_h;
   if (h < mh) h = mh;
   if (h > zone->h) h = zone->h;
   y = (double)(zone->h - h) * exebuf_conf->pos_align_y;
   
   e_popup_move_resize(exebuf, x, y, w, h);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, w, h);
   evas_object_show(o);
   e_popup_edje_bg_object_set(exebuf, o);

   evas_event_thaw(exebuf->evas);

   handlers = eina_list_append
     (handlers, ecore_event_handler_add
	 (ECORE_EVENT_KEY_DOWN, _e_exebuf_cb_key_down, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
	 (ECORE_EVENT_MOUSE_BUTTON_DOWN, _e_exebuf_cb_mouse_down, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
	 (ECORE_EVENT_MOUSE_BUTTON_UP, _e_exebuf_cb_mouse_up, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
	 (ECORE_EVENT_MOUSE_MOVE, _e_exebuf_cb_mouse_move, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
	 (ECORE_EVENT_MOUSE_WHEEL, _e_exebuf_cb_mouse_wheel, NULL));

   el = e_config_domain_load("exebuf_exelist_cache", exelist_edd);
   if (el)
     {
        E_Exe *ee;

	EINA_LIST_FREE(el->list, ee)
	  {
	     exe_list = eina_list_append(exe_list, strdup(ee->path));
	     eina_stringshare_del(ee->path);
	     free(ee);
	  }
	free(el);
     }
   path = getenv("PATH");
   if (path)
     {
	path = strdup(path);
	last = path;
	for (p = path; p[0]; p++)
	  {
	     if (p[0] == ':') p[0] = '\0';
	     if (p[0] == 0)
	       {
		  exe_path = eina_list_append(exe_path, strdup(last));
		  last = p + 1;
	       }
	  }
	if (p > last)
	  exe_path = eina_list_append(exe_path, strdup(last));
	free(path);
     }
   exe_list_idler = ecore_idler_add(_e_exebuf_idler, NULL);
   
   e_popup_show(exebuf);
   return 1;
}

void
e_exebuf_hide(void)
{
   Ecore_Event_Handler *ev;
   char *str;

   if (!exebuf) return;
   
   evas_event_freeze(exebuf->evas);
   _e_exebuf_matches_clear();
   e_popup_hide(exebuf);
   if (exe_scroll_timer) ecore_timer_del(exe_scroll_timer);
   exe_scroll_timer = NULL;
   if (eap_scroll_timer) ecore_timer_del(eap_scroll_timer);
   eap_scroll_timer = NULL;
   if (animator) ecore_animator_del(animator);
   if (update_timer) ecore_timer_del(update_timer);
   update_timer = NULL;
   animator = NULL;
   exe_scroll_to = 0;
   exe_scroll_align_to = 0.0;
   exe_scroll_align = 0.0;
   eap_scroll_to = 0;
   eap_scroll_align_to = 0.0;
   eap_scroll_align = 0.0;
   evas_object_del(eap_list_object);
   eap_list_object = NULL;
   evas_object_del(exe_list_object);
   exe_list_object = NULL;
   evas_object_del(bg_object);
   bg_object = NULL;
   if (icon_object) evas_object_del(icon_object);
   icon_object = NULL;
   evas_event_thaw(exebuf->evas);
   e_object_del(E_OBJECT(exebuf));
   exebuf = NULL;

   EINA_LIST_FREE(handlers, ev)
     ecore_event_handler_del(ev);

   ecore_x_window_free(input_window);
   e_grabinput_release(input_window, input_window);
   input_window = 0;
   free(cmd_buf);
   cmd_buf = NULL;
   if (exe_dir)
     {
	closedir(exe_dir);
	exe_dir = NULL;
     }
   EINA_LIST_FREE(exe_path, str)
     free(str);

   if (exe_list_idler)
     {
	ecore_idler_del(exe_list_idler);
	exe_list_idler = NULL;
     }
   EINA_LIST_FREE(exe_list, str)
     free(str);
   EINA_LIST_FREE(exe_list2, str)
     free(str);
   history = eina_list_free(history);
   which_list = NO_LIST;
   exe_sel = NULL;
}

/* local subsystem functions */

static void
_e_exebuf_exe_free(E_Exebuf_Exe *exe)
{
   if (ev_last_mouse_exe == exe)
     ev_last_mouse_exe = NULL;

   evas_object_del(exe->bg_object);
   if (exe->icon_object) evas_object_del(exe->icon_object);
   if (exe->desktop) efreet_desktop_free(exe->desktop);
   free(exe);
}

static void
_e_exebuf_matches_clear(void)
{
   Efreet_Desktop *desktop;
   E_Exebuf_Exe *exe;
   char *file;

   EINA_LIST_FREE(eap_matches, desktop)
     efreet_desktop_free(desktop);
   EINA_LIST_FREE(exe_matches, file)
     free(file);

   evas_event_freeze(exebuf->evas);
   e_box_freeze(eap_list_object);
   e_box_freeze(exe_list_object);
   EINA_LIST_FREE(exes, exe)
     _e_exebuf_exe_free(exe);
   EINA_LIST_FREE(eaps, exe)
     _e_exebuf_exe_free(exe);
   e_box_thaw(exe_list_object);
   e_box_thaw(eap_list_object);
   evas_event_thaw(exebuf->evas);
   
   e_box_align_set(eap_list_object, 0.5, 0.0);
   e_box_align_set(exe_list_object, 0.5, 1.0);
   exe_sel = NULL;
   which_list = NO_LIST;
}

static void
_e_exebuf_update(void)
{
   Efreet_Desktop *desktop;
   Evas_Object *o;
   
   edje_object_part_text_set(bg_object, "e.text.label", cmd_buf);
   if (icon_object) evas_object_del(icon_object);
   icon_object = NULL;

   if (!cmd_buf[0]) return;

   desktop = efreet_util_desktop_exec_find(cmd_buf);
   fprintf(stderr, "0 %p\n", desktop);
   if (desktop) fprintf(stderr, "[%s]\n", desktop->icon);
   if (!desktop) desktop = efreet_util_desktop_name_find(cmd_buf);
   fprintf(stderr, "1 %p\n", desktop);
   if (desktop) fprintf(stderr, "[%s]\n", desktop->icon);
   if (!desktop) desktop = efreet_util_desktop_generic_name_find(cmd_buf);
   fprintf(stderr, "2 %p\n", desktop);
   if (desktop) fprintf(stderr, "[%s]\n", desktop->icon);
   if (desktop)
     {
	o = e_util_desktop_icon_add(desktop, 24, exebuf->evas);
	icon_object = o;
	edje_object_part_swallow(bg_object, "e.swallow.icons", o);
	evas_object_show(o);
	efreet_desktop_free(desktop);
     }
}

static void
_e_exebuf_exec(void)
{
   if (exe_sel)
     {
	if (exe_sel->desktop && which_list != EXE_LIST)
	  e_exec(exebuf->zone, exe_sel->desktop, NULL, NULL, "exebuf");
	else
	  e_exec(exebuf->zone, NULL, exe_sel->file, NULL, "exebuf");
     }
   else
     e_exec(exebuf->zone, NULL, cmd_buf, NULL, "exebuf");
   
   e_exebuf_hide();
}

static void
_e_exebuf_exec_term(void)
{
   char tmp[EXEBUFLEN];
   const char *active_cmd = NULL;

   if (exe_sel)
     {
	if (exe_sel->desktop)
	  e_exec(exebuf->zone, exe_sel->desktop, NULL, NULL, "exebuf");
	else 
	  active_cmd = exe_sel->file;
     }
   else
     active_cmd = cmd_buf;

   if (active_cmd && active_cmd[0])
     {
	/* Copy the terminal command to the start of the string...
	 * making sure it has a null terminator if greater than EXEBUFLEN */
	snprintf(tmp, EXEBUFLEN, "%s %s", exebuf_conf->term_cmd, active_cmd);
	e_exec(exebuf->zone, NULL, tmp, NULL, "exebuf");
     }

   e_exebuf_hide();
}

static void
_e_exebuf_exe_sel(E_Exebuf_Exe *exe)
{
   edje_object_signal_emit(exe->bg_object, "e,state,selected", "e");
   if (exe->icon_object)
     edje_object_signal_emit(exe->icon_object, "e,state,selected", "e");
}

static void
_e_exebuf_exe_desel(E_Exebuf_Exe *exe)
{
   edje_object_signal_emit(exe->bg_object, "e,state,unselected", "e");
   if (exe->icon_object)
     edje_object_signal_emit(exe->icon_object, "e,state,unselected", "e");
}

static void
_e_exebuf_exe_scroll_to(int i)
{
   int n;
   
   n = eina_list_count(exes);
   if (n > 1)
     {
	exe_scroll_align_to = (double)i / (double)(n - 1);
	if (exebuf_conf->scroll_animate)
	  {	
	     exe_scroll_to = 1;
	     if (!exe_scroll_timer)
	       exe_scroll_timer = ecore_timer_add(0.01, _e_exebuf_exe_scroll_timer, NULL);
	     if (!animator)
	       animator = ecore_animator_add(_e_exebuf_animator, NULL);
	  }
	else
	  {
	     exe_scroll_align = exe_scroll_align_to;
	     e_box_align_set(exe_list_object, 0.5, 1.0 - exe_scroll_align);
	  }
     }
   else
     e_box_align_set(exe_list_object, 0.5, 1.0);
}

static void
_e_exebuf_eap_scroll_to(int i)
{
   int n;
   
   n = eina_list_count(eaps);
   if (n > 1)
     {
	eap_scroll_align_to = (double)i / (double)(n - 1);
	if (exebuf_conf->scroll_animate)
	  {	
	     eap_scroll_to = 1;
	     if (!eap_scroll_timer)
	       eap_scroll_timer = ecore_timer_add(0.01, _e_exebuf_eap_scroll_timer, NULL);
	     if (!animator)
	       animator = ecore_animator_add(_e_exebuf_animator, NULL);
	  }
	else
	  {
	     eap_scroll_align = eap_scroll_align_to;
	     e_box_align_set(eap_list_object, 0.5, eap_scroll_align);
	  }
     }
   else
     e_box_align_set(eap_list_object, 0.5, 0.0);
}

static void
_e_exebuf_next(void)
{
   Eina_List *l;
   int i;
   
   if (which_list == NO_LIST)
     {
	if (exes)
	  {
	     exe_sel = exes->data;
	     which_list = EXE_LIST;
	     if (exe_sel)
	       {
		  _e_exebuf_exe_sel(exe_sel);
		  _e_exebuf_exe_scroll_to(0);
	       }
	  }
     }
   else
     {
	if (which_list == EXE_LIST)
	  {
	     if (exe_sel)
	       {
		  for (i = 0, l = exes; l; l = l->next, i++)
		    {
		       if (l->data == exe_sel)
			 {
			    if (l->next)
			      {
				 _e_exebuf_exe_desel(exe_sel);
				 exe_sel = l->next->data;
				 _e_exebuf_exe_sel(exe_sel);
				 _e_exebuf_exe_scroll_to(i + 1);
			      }
			    break;
			 }
		    }
	       }
	  }
	else if (which_list == EAP_LIST)
	  {
	     if (exe_sel)
	       {
		  for (i = 0, l = eaps; l; l = l->next, i++)
		    {
		       if (l->data == exe_sel)
			 {
			    _e_exebuf_exe_desel(exe_sel);
			    if (l->prev)
			      {
				 exe_sel = l->prev->data;
				 _e_exebuf_exe_sel(exe_sel);
				 _e_exebuf_eap_scroll_to(i - 1);
			      }
			    else
			      {
				 exe_sel = NULL;
				 which_list = NO_LIST;
			      }
			    break;
			 }
		    }
	       }
	  }
	else if (which_list == HIST_LIST)
	  {
	     if (exe_sel)
	       {
		  for (i = 0, l = eaps; l; l = l->next, i++)
		    {
		       if (l->data == exe_sel)
			 {
			    _e_exebuf_exe_desel(exe_sel);
			    if (l->prev)
			      {
				 exe_sel = l->prev->data;
				 _e_exebuf_exe_sel(exe_sel);
				 _e_exebuf_eap_scroll_to(i - 1);
			      }
			    else
			      {
				 exe_sel = NULL;
				 which_list = NO_LIST;
				 _e_exebuf_hist_clear();
			      }
			    break;
			 }
		    }
	       }
	  }
     }
}

static void
_e_exebuf_prev(void)
{
   Eina_List *l;
   int i;

   if (which_list == NO_LIST)
     {
	if (eaps)
	  {
	     exe_sel = eaps->data;
	     which_list = EAP_LIST;
	     if (exe_sel)
	       {
		  _e_exebuf_exe_sel(exe_sel);
		  _e_exebuf_eap_scroll_to(0);
	       }
	  }
	else
	  {
	     _e_exebuf_hist_update(NULL);
	     if (eaps)
	       {     
		  which_list = HIST_LIST;
		  ev_last_which_list = HIST_LIST;
		  exe_sel = eaps->data;
		  if (exe_sel)
		    {
		       _e_exebuf_exe_sel(exe_sel);
		       _e_exebuf_eap_scroll_to(0);
		    }
	       }
	  }
     }
   else
     {
	if (which_list == EXE_LIST)
	  {
	     if (exe_sel)
	       {
		  for (i = 0, l = exes; l; l = l->next, i++)
		    {
		       if (l->data == exe_sel)
			 {
			    _e_exebuf_exe_desel(exe_sel);
			    if (l->prev)
			      {
				 exe_sel = l->prev->data;
				 _e_exebuf_exe_sel(exe_sel);
				 _e_exebuf_exe_scroll_to(i - 1);
			      }
			    else
			      {
				 exe_sel = NULL;
				 which_list = NO_LIST;
			      }
			    break;
			 }
		    }
	       }
	  }
	else if (which_list == EAP_LIST)
	  {
	     if (exe_sel)
	       {
		  for (i = 0, l = eaps; l; l = l->next, i++)
		    {
		       if (l->data == exe_sel)
			 {
			    if (l->next)
			      {
				 _e_exebuf_exe_desel(exe_sel);
				 exe_sel = l->next->data;
				 _e_exebuf_exe_sel(exe_sel);
				 _e_exebuf_eap_scroll_to(i + 1);
			      }
			    break;
			 }
		    }
	       }
	  }
	else if (which_list == HIST_LIST)
	  {
	     if (exe_sel)
	       {
		  for (i = 0, l = eaps; l; l = l->next, i++)
		    {
		       if (l->data == exe_sel)
			 {
			    if (l->next)
			      {
				 _e_exebuf_exe_desel(exe_sel);
				 exe_sel = l->next->data;
				 _e_exebuf_exe_sel(exe_sel);
				 _e_exebuf_eap_scroll_to(i + 1);
			      }
			    break;
			 }
		    }
	       }
	  }
     }
}

static void
_e_exebuf_complete(void)
{
   char common[EXEBUFLEN], *exe = NULL;
   Eina_List *l;
   int orig_len = 0, common_len = 0, exe_len, next_char, val, matches;
   int clear_hist = 0;
   
   if (!(strlen(cmd_buf)))
     clear_hist = 1;
   if (exe_sel)
     {
	if (exe_sel->desktop)
	  {
	     char *exe;

	     exe = ecore_file_app_exe_get(exe_sel->desktop->exec);
	     if (exe)
	       {
		  eina_strlcpy(cmd_buf, exe, EXEBUFLEN);
		  free(exe);
	       }
	  }
	else if (exe_sel->file)
	  {
	     eina_strlcpy(cmd_buf, exe_sel->file, EXEBUFLEN);
	  }
     }
   else
     {
	strcpy(common, cmd_buf);
	orig_len = common_len = strlen(common);
	matches = 1;
	while (matches)
	  {
	     next_char = 0;
	     matches = 0;
	     for (l = exe_matches; l; l = l->next)
	       {
		  matches = 1;
		  exe = l->data;
		  exe_len = strlen(exe);
		  if (exe_len > common_len)
		    {
		       val = 0;
		       if (!next_char)
			 next_char = val;
		       else if (next_char != val)
			 {
			    matches = 0;
			    break;
			 }
		    }
		  else
		    {
		       matches = 0;
		       break;
		    }
	       }
	     if (matches) common_len++;
	  }
     }
   if ((exe) && (orig_len < common_len) && (common_len < (EXEBUFLEN - 1)))
     {
	eina_strlcpy(cmd_buf, exe, common_len + 1);
     }
   if (clear_hist)
     _e_exebuf_hist_clear();
   _e_exebuf_update();
   if (!update_timer)
     update_timer = ecore_timer_add(MATCH_LAG, _e_exebuf_update_timer, NULL);
}

static void
_e_exebuf_backspace(void)
{
   int len, val, pos;
   
   len = strlen(cmd_buf);
   if (len > 0)
     {
	pos = evas_string_char_prev_get(cmd_buf, len, &val);
	if ((pos < len) && (pos >= 0))
	  {
	     cmd_buf[pos] = 0;
	     _e_exebuf_update();
	     if (!update_timer)
	       update_timer = ecore_timer_add(MATCH_LAG, _e_exebuf_update_timer, NULL);
	  }
     }
}

/* unused */
#if 0
static void
_e_exebuf_delete(void)
{
   E_Exebuf_Exe *exe_del, *exe_l = NULL, *exe_p = NULL;
   Eina_List *list = NULL, *l = NULL;
   int i;

   if ((which_list != HIST_LIST) || !exe_sel) return;

   exe_del = exe_sel;
   e_exehist_del(exe_del->file);

   list = e_exehist_list_get();
   if (!list)
     {
	_e_exebuf_hist_clear();
	return;
     }

   l = eina_list_last(eaps);
   if (l)
     {
	exe_l = l->data;
	l = l->prev;
	if (l) exe_p = l->data;
     }
   l = eina_list_last(list);

   if ((!exe_l) || (strcmp(exe_l->file, l->data) &&
		   ((!exe_p) || strcmp(exe_p->file, l->data))))
     {
	E_Exebuf_Exe *exe;
	Evas_Coord mw, mh;
	Evas_Object *o;
	
	exe = calloc(1, sizeof(E_Exebuf_Exe));
	exe->file = l->data;
        eaps = eina_list_append(eaps, exe);
	o = edje_object_add(exebuf->evas);
        exe->bg_object = o;
	e_theme_edje_object_set(o, "base/theme/exebuf",
				"e/widgets/exebuf/item");
	edje_object_part_text_set(o, "e.text.title", exe->file);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,
				       _e_exebuf_cb_exe_item_mouse_in, exe);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT,
				       _e_exebuf_cb_exe_item_mouse_out, exe);
	evas_object_show(o);
	if (edje_object_part_exists(exe->bg_object, "e.swallow.icons"))
	  {
	     Efreet_Desktop *desktop;

	     desktop = efreet_util_desktop_exec_find(exe->file);
	     if (desktop)
	       {
		  o = e_util_desktop_icon_add(desktop, 24, exebuf->evas);
		  exe->icon_object = o;
		  edje_object_part_swallow(exe->bg_object, "e.swallow.icons",
					   o);
		  evas_object_show(o);
		  exe->desktop = desktop;
	       }
	  }
	edje_object_size_min_calc(exe->bg_object, &mw, &mh);
	e_box_pack_start(eap_list_object, exe->bg_object);
	e_box_pack_options_set(exe->bg_object,
			       1, 1, /* fill */
			       1, 0, /* expand */
			       0.5, 0.5, /* align */
			       mw, mh, /* min */
			       9999, mh /* max */
			       );
     }
   eina_list_free(list);

   _e_exebuf_prev();
   if (exe_sel == exe_del) _e_exebuf_next();
   if (exe_sel)
     {
	evas_event_freeze(exebuf->evas);
	e_box_freeze(eap_list_object);
	e_box_freeze(exe_list_object);
	_e_exebuf_exe_free(exe_del);
	eaps = eina_list_remove(eaps, exe_del);
	e_box_thaw(exe_list_object);
	e_box_thaw(eap_list_object);
	evas_event_thaw(exebuf->evas);
	for (l = eaps, i = 0; l; l = l->next, i++)
	  {
	     if (l->data == exe_sel)
	       {
		  _e_exebuf_eap_scroll_to(i);
		  break;
	       }
	  }
     }
}
#endif

static void
_e_exebuf_clear(void)
{
   if (cmd_buf[0] != 0)
     {
	cmd_buf[0] = 0;
	_e_exebuf_update();
	if (!update_timer)
	   update_timer = ecore_timer_add(MATCH_LAG, _e_exebuf_update_timer, NULL);
     }
}

static int
_e_exebuf_cb_sort_eap(const void *data1, const void *data2)
{
   const Efreet_Desktop *a1, *a2;
   const char *e1, *e2;
   double t1, t2;
   
   a1 = data1;
   a2 = data2;
   e1 = a1->exec;
   e2 = a2->exec;
   t1 = e_exehist_newest_run_get(e1);
   t2 = e_exehist_newest_run_get(e2);
   return (int)(t2 - t1);
}

static int
_e_exebuf_cb_sort_exe(const void *data1, const void *data2)
{
   const char *e1, *e2;
   double t1, t2;
   
   e1 = data1;
   e2 = data2;
   t1 = e_exehist_newest_run_get(e1);
   t2 = e_exehist_newest_run_get(e2);
   return (int)(t2 - t1);
}

static void
_e_exebuf_matches_update(void)
{
   char *path, *file, buf[4096];
   Efreet_Desktop *desktop, *desktop2;
   Eina_Hash *added = NULL;
   Eina_List *list;
   Eina_List *l;
   Eina_Bool hist = EINA_FALSE;
   char *exe;
   int i, max;

   hist = (which_list == HIST_LIST);
   _e_exebuf_matches_clear();
   if (!cmd_buf[0]) return;

   if (hist && eina_list_count(history))
     {
	Eina_List *hist_matches = NULL;

	snprintf(buf, sizeof(buf), "%s*", cmd_buf);
	EINA_LIST_FOREACH(history, l, file)
	  {
	     if (e_util_glob_match(file, buf))
	       {
		  if (!eina_hash_find(added, file))
		    {
		       hist_matches = eina_list_append(hist_matches, file);
		       if (!added)
			 added = eina_hash_string_superfast_new(NULL);
		       eina_hash_direct_add(added, file, file);
		    }
	       }
	  }
	if (added) eina_hash_free(added);
	added = NULL;

	if (hist_matches)
	  {
	     _e_exebuf_hist_update(hist_matches);
	     eina_list_free(hist_matches);
	     exe_sel = eaps->data;
	     if (exe_sel)
	       {
		  _e_exebuf_exe_sel(exe_sel);
		  _e_exebuf_eap_scroll_to(0);
	       }
	  }

	which_list = HIST_LIST;
	return;
     }

   snprintf(buf, sizeof(buf), "*%s*", cmd_buf);
   list = efreet_util_desktop_name_glob_list(buf);
   EINA_LIST_FREE(list, desktop)
     {
        exe = ecore_file_app_exe_get(desktop->exec);
        if (exe)
          {
             desktop2 = eina_hash_find(added, exe);
             if ((!desktop2) || (desktop2 != desktop))
               {
                  eap_matches = eina_list_append(eap_matches, desktop);
                  if (!added)
                    added = eina_hash_string_superfast_new(NULL);
                  eina_hash_add(added, exe, desktop);
               }
	     else
	       efreet_desktop_free(desktop);
	     free(exe);
          }
	else
	  efreet_desktop_free(desktop);
     }

   snprintf(buf, sizeof(buf), "%s*", cmd_buf);
   list = efreet_util_desktop_exec_glob_list(buf);
   EINA_LIST_FREE(list, desktop)
     {
        exe = ecore_file_app_exe_get(desktop->exec);
        if (exe)
          {
             desktop2 = eina_hash_find(added, exe);
             if ((!desktop2) || (desktop2 != desktop))
               {
                  eap_matches = eina_list_append(eap_matches, desktop);
                  if (!added)
                    added = eina_hash_string_superfast_new(NULL);
                  eina_hash_add(added, exe, desktop);
               }
	     else
	       efreet_desktop_free(desktop);
             free(exe);
	  }
	else
	  efreet_desktop_free(desktop);
     }

   snprintf(buf, sizeof(buf), "*%s*", cmd_buf);
   list = efreet_util_desktop_generic_name_glob_list(buf);
   EINA_LIST_FREE(list, desktop)
     {
        exe = ecore_file_app_exe_get(desktop->exec);
        if (exe)
          {
             desktop2 = eina_hash_find(added, exe);
             if ((!desktop2) || (desktop2 != desktop))
               {
                  eap_matches = eina_list_append(eap_matches, desktop);
                  if (!added)
                    added = eina_hash_string_superfast_new(NULL);
                  eina_hash_add(added, exe, desktop);
               }
	     else
	       efreet_desktop_free(desktop);
             free(exe);
          }
	else
	  efreet_desktop_free(desktop);
     }

   snprintf(buf, sizeof(buf), "*%s*", cmd_buf);
   list = efreet_util_desktop_comment_glob_list(buf);
   EINA_LIST_FREE(list, desktop)
     {
        exe = ecore_file_app_exe_get(desktop->exec);
        if (exe)
          {
             desktop2 = eina_hash_find(added, exe);
             if ((!desktop2) || (desktop2 != desktop))
               {
                  eap_matches = eina_list_append(eap_matches, desktop);
                  if (!added)
                    added = eina_hash_string_superfast_new(NULL);
                  eina_hash_add(added, exe, desktop);
               }
	     else
	       efreet_desktop_free(desktop);
             free(exe);
          }
	else
	  efreet_desktop_free(desktop);
     }

   if (added) eina_hash_free(added);
   added = NULL;

   snprintf(buf, sizeof(buf), "%s*", cmd_buf);
   EINA_LIST_FOREACH(exe_list, l, path)
     {
	     file = (char *)ecore_file_file_get(path);
	     if (file)
	       {
		  if (e_util_glob_match(file, buf))
		    {
		       if (!eina_hash_find(added, file))
			 {
			    exe_matches = eina_list_append(exe_matches, strdup(file));
			    if (!added)
			      added = eina_hash_string_superfast_new(NULL);
			    eina_hash_direct_add(added, file, file);
			 }
		    }
	       }
	  }
   if (added) eina_hash_free(added);
   added = NULL;

   eap_matches = eina_list_sort(eap_matches, eina_list_count(eap_matches), _e_exebuf_cb_sort_eap);

   max = exebuf_conf->max_eap_list;
   evas_event_thaw(exebuf->evas);
   e_box_freeze(eap_list_object);
   for (i = 0, l = eap_matches; l && (i < max); l = l->next, i++)
     {
	Efreet_Desktop *desktop;
	E_Exebuf_Exe *exe;
	Evas_Coord mw, mh;
	Evas_Object *o;
	int opt = 0;

	exe = calloc(1, sizeof(E_Exebuf_Exe));
        eaps = eina_list_append(eaps, exe);
	desktop = eina_list_data_get(l);
	exe->desktop = desktop;
	efreet_desktop_ref(desktop);

	o = edje_object_add(exebuf->evas);
        exe->bg_object = o;
	e_theme_edje_object_set(o, "base/theme/exebuf",
				"e/widgets/exebuf/item");
	if (e_config->menu_eap_name_show && exe->desktop->name) opt |= 0x4;
	if (e_config->menu_eap_generic_show && exe->desktop->generic_name) opt |= 0x2;
	if (e_config->menu_eap_comment_show && exe->desktop->comment) opt |= 0x1;
	if      (opt == 0x7) snprintf(buf, sizeof(buf), "%s (%s) [%s]", exe->desktop->name, exe->desktop->generic_name, exe->desktop->comment);
	else if (opt == 0x6) snprintf(buf, sizeof(buf), "%s (%s)", exe->desktop->name, exe->desktop->generic_name);
	else if (opt == 0x5) snprintf(buf, sizeof(buf), "%s [%s]", exe->desktop->name, exe->desktop->comment);
	else if (opt == 0x4) snprintf(buf, sizeof(buf), "%s", exe->desktop->name);
	else if (opt == 0x3) snprintf(buf, sizeof(buf), "%s [%s]", exe->desktop->generic_name, exe->desktop->comment);
	else if (opt == 0x2) snprintf(buf, sizeof(buf), "%s", exe->desktop->generic_name);
	else if (opt == 0x1) snprintf(buf, sizeof(buf), "%s", exe->desktop->comment);
	else snprintf(buf, sizeof(buf), "%s", exe->desktop->name);
	edje_object_part_text_set(o, "e.text.title", buf);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,
	      _e_exebuf_cb_eap_item_mouse_in, exe);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT,
	      _e_exebuf_cb_eap_item_mouse_out, exe);
	evas_object_show(o);
	if (edje_object_part_exists(exe->bg_object, "e.swallow.icons"))
	  {
	     o = e_util_desktop_icon_add(exe->desktop, 24, exebuf->evas);
	     exe->icon_object = o;
	     edje_object_part_swallow(exe->bg_object, "e.swallow.icons", o);
	     evas_object_show(o);
	  }
	edje_object_size_min_calc(exe->bg_object, &mw, &mh);
	e_box_pack_start(eap_list_object, exe->bg_object);
	e_box_pack_options_set(exe->bg_object,
			       1, 1, /* fill */
			       1, 0, /* expand */
			       0.5, 0.5, /* align */
			       mw, mh, /* min */
			       9999, mh /* max */
			       );
     }
   e_box_thaw(eap_list_object);
   
   exe_matches = eina_list_sort(exe_matches, eina_list_count(exe_matches), _e_exebuf_cb_sort_exe);
   
   max = exebuf_conf->max_exe_list;
   e_box_freeze(exe_list_object);
   for (i = 0, l = exe_matches; l && (i < max); l = l->next, i++)
     {
	E_Exebuf_Exe *exe;
	Evas_Coord mw, mh;
	Evas_Object *o;
	
	exe = calloc(1, sizeof(E_Exebuf_Exe));
	exe->file = l->data;
        exes = eina_list_append(exes, exe);
	o = edje_object_add(exebuf->evas);
        exe->bg_object = o;
	e_theme_edje_object_set(o, "base/theme/exebuf",
				"e/widgets/exebuf/item");
	edje_object_part_text_set(o, "e.text.title", exe->file);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,
	      _e_exebuf_cb_exe_item_mouse_in, exe);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT,
	      _e_exebuf_cb_exe_item_mouse_out, exe);
	evas_object_show(o);
	if (edje_object_part_exists(exe->bg_object, "e.swallow.icons"))
	  {
	     Efreet_Desktop *desktop;
	     
	     desktop = efreet_util_desktop_exec_find(exe->file);
	     if (desktop)
	       {
		  o = e_util_desktop_icon_add(desktop, 24, exebuf->evas);
		  exe->icon_object = o;
		  edje_object_part_swallow(exe->bg_object, "e.swallow.icons", o);
		  evas_object_show(o);
		  exe->desktop = desktop;
	       }
	  }
	edje_object_size_min_calc(exe->bg_object, &mw, &mh);
	e_box_pack_end(exe_list_object, exe->bg_object);
	e_box_pack_options_set(exe->bg_object,
			       1, 1, /* fill */
			       1, 0, /* expand */
			       0.5, 0.5, /* align */
			       mw, mh, /* min */
			       9999, mh /* max */
			       );
     }
   e_box_thaw(exe_list_object);
   evas_event_thaw(exebuf->evas);
}

static void
_e_exebuf_hist_update(Eina_List *hist_matches)
{
   Eina_List *list, *l;
   char *file;

   edje_object_signal_emit(bg_object, "e,action,show,history", "e");
   if (hist_matches)
     list = hist_matches;
   else
     {
	history = eina_list_free(history);
	history = list = eina_list_reverse(e_exehist_list_get());
     }
   EINA_LIST_FOREACH(list, l, file)
     {
	E_Exebuf_Exe *exe;
	Evas_Coord mw, mh;
	Evas_Object *o;
	Efreet_Desktop *desktop;
	Efreet_Desktop *found = NULL;
	int len;
	char *tmp;
	char match[4096];
	Eina_List *ll;
	
	exe = calloc(1, sizeof(E_Exebuf_Exe));
	exe->file = file;
        eaps = eina_list_prepend(eaps, exe);
	o = edje_object_add(exebuf->evas);
        exe->bg_object = o;
	e_theme_edje_object_set(o, "base/theme/exebuf",
				"e/widgets/exebuf/item");
	edje_object_part_text_set(o, "e.text.title", exe->file);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,
				       _e_exebuf_cb_exe_item_mouse_in, exe);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT,
				       _e_exebuf_cb_exe_item_mouse_out, exe);
	evas_object_show(o);
	     
	len = strlen(file);
	tmp = ecore_file_app_exe_get(exe->file);
	snprintf(match, sizeof(match), "%s*", tmp);
	ll = efreet_util_desktop_exec_glob_list(match);

	EINA_LIST_FREE(ll, desktop)
	  {
	     if (desktop->exec && !strncmp(file, desktop->exec, len))
	       found = desktop;
	     else
	       efreet_desktop_free(desktop);
	  }

	if (found)
	  {
	     exe->desktop = found;
	     edje_object_part_text_set(o, "e.text.title", exe->desktop->name);

	     if (edje_object_part_exists(exe->bg_object, "e.swallow.icons"))
	       {
		  o = e_util_desktop_icon_add(exe->desktop, 24, exebuf->evas);
		  exe->icon_object = o;
		  edje_object_part_swallow(exe->bg_object, "e.swallow.icons", o);
		  evas_object_show(o);
	       }
	  }

	edje_object_size_min_calc(exe->bg_object, &mw, &mh);
	e_box_pack_end(eap_list_object, exe->bg_object);
	e_box_pack_options_set(exe->bg_object,
			       1, 1, /* fill */
			       1, 0, /* expand */
			       0.5, 0.5, /* align */
			       mw, mh, /* min */
			       9999, mh /* max */
			       );
     }
}

static void
_e_exebuf_hist_clear(void)
{
   E_Exebuf_Exe *exe;

   edje_object_signal_emit(bg_object, "e,action,hide,history", "e");
   evas_event_freeze(exebuf->evas);
   e_box_freeze(eap_list_object);
   e_box_freeze(exe_list_object);
   EINA_LIST_FREE(eaps, exe)
     _e_exebuf_exe_free(exe);
   e_box_thaw(exe_list_object);
   e_box_thaw(eap_list_object);
   evas_event_thaw(exebuf->evas);
   
   e_box_align_set(eap_list_object, 0.5, 0.0);
   e_box_align_set(exe_list_object, 0.5, 1.0);
   exe_sel = NULL;
   which_list = NO_LIST;
   _e_exebuf_clear();
}

static void 
_e_exebuf_cb_eap_item_mouse_in(void *data, Evas *evas, Evas_Object *obj, 
      void *event_info)
{
   ev_last_mouse_exe = data;
   ev_last_which_list = EAP_LIST;
   if (!ev_last_is_mouse) return;

   if (exe_sel) _e_exebuf_exe_desel(exe_sel);
   if (!(exe_sel = data)) return;
   which_list = EAP_LIST;
   _e_exebuf_exe_sel(exe_sel);
}

static void 
_e_exebuf_cb_eap_item_mouse_out(void *data, Evas *evas, Evas_Object *obj, 
      void *event_info)
{
   ev_last_mouse_exe = NULL;
}

static void 
_e_exebuf_cb_exe_item_mouse_in(void *data, Evas *evas, Evas_Object *obj, 
      void *event_info)
{
   ev_last_mouse_exe = data;

   if (which_list == HIST_LIST)
     ev_last_which_list = HIST_LIST;
   else
     ev_last_which_list = EXE_LIST;
   if (!ev_last_is_mouse) return;

   if (exe_sel) _e_exebuf_exe_desel(exe_sel);
   if (!(exe_sel = data)) return;
   if (which_list != HIST_LIST)
     which_list = EXE_LIST;
   _e_exebuf_exe_sel(exe_sel);
}

static void 
_e_exebuf_cb_exe_item_mouse_out(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   ev_last_mouse_exe = NULL;
}

static Eina_Bool
_e_exebuf_cb_key_down(void *data, int type, void *event)
{
   Ecore_Event_Key *ev;

   ev_last_is_mouse = 0;

   ev = event;
   if (ev->event_window != input_window) return 1;
   if (!strcmp(ev->key, "Up"))
     _e_exebuf_prev();
   else if (!strcmp(ev->key, "Down"))
     _e_exebuf_next();
   else if (!strcmp(ev->key, "Prior"))
     _e_exebuf_prev();
   else if (!strcmp(ev->key, "Next"))
     _e_exebuf_next();
   else if (!strcmp(ev->key, "Left"))
     _e_exebuf_prev();
   else if (!strcmp(ev->key, "Right"))
     _e_exebuf_complete();
   else if (!strcmp(ev->key, "Tab"))
     _e_exebuf_complete();
   else if (!strcmp(ev->key, "Return") && (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL))
     _e_exebuf_exec_term();
   else if (!strcmp(ev->key, "Return"))
     _e_exebuf_exec();
   else if (!strcmp(ev->key, "KP_Enter") && (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL))
     _e_exebuf_exec_term();
   else if (!strcmp(ev->key, "KP_Enter"))
     _e_exebuf_exec();
   else if (!strcmp(ev->key, "u") && (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL))
     _e_exebuf_clear();
   else if (!strcmp(ev->key, "Escape"))
     e_exebuf_hide();
   else if (!strcmp(ev->key, "BackSpace"))
     _e_exebuf_backspace();
   else if (!strcmp(ev->key, "Delete"))
     _e_exebuf_backspace();
   else
     {
	if (ev->compose)
	  {
	     if ((strlen(cmd_buf) < (EXEBUFLEN - strlen(ev->compose))))
	       {
		  strcat(cmd_buf, ev->compose);
		  _e_exebuf_update();
		  if (!update_timer)
		    update_timer = ecore_timer_add(MATCH_LAG, _e_exebuf_update_timer, NULL);
	       }
	  }
     }
   return EINA_TRUE;
}

static Eina_Bool
_e_exebuf_cb_mouse_down(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Button *ev;
   
   ev = event;
   if (ev->event_window != input_window) return EINA_TRUE;

   if (ev_last_mouse_exe && (exe_sel != ev_last_mouse_exe))
     {
        if (exe_sel) _e_exebuf_exe_desel(exe_sel);
        exe_sel = ev_last_mouse_exe;
        _e_exebuf_exe_sel(exe_sel); 
     }   

   return EINA_TRUE;
}

static Eina_Bool
_e_exebuf_cb_mouse_up(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Button *ev;
   
   ev = event;
   if (ev->event_window != input_window) return EINA_TRUE;
   if (ev->buttons == 1) 
     _e_exebuf_exec();
   else if (ev->buttons == 2)
     _e_exebuf_complete();

   return EINA_TRUE;
}

static Eina_Bool 
_e_exebuf_cb_mouse_move(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Move *ev;

   ev = event;
   if (ev->event_window != input_window) return EINA_TRUE;

   if (!ev_last_is_mouse)
     {
        ev_last_is_mouse = 1;
        if (ev_last_mouse_exe)
          {
             if (exe_sel && (exe_sel != ev_last_mouse_exe))
               _e_exebuf_exe_desel(exe_sel);
             if (!exe_sel || (exe_sel != ev_last_mouse_exe))
               {
                  exe_sel = ev_last_mouse_exe;
                  which_list = ev_last_which_list;
                  _e_exebuf_exe_sel(exe_sel); 
               }
          }
     }

   evas_event_feed_mouse_move(exebuf->evas, ev->x - exebuf->x,
			      ev->y - exebuf->y, ev->timestamp, NULL);

   return EINA_TRUE;
}

static Eina_Bool
_e_exebuf_cb_mouse_wheel(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Wheel *ev;
   
   ev = event;
   if (ev->event_window != input_window) return EINA_TRUE;

   ev_last_is_mouse = 0;

   if (ev->z < 0) /* up */
     {
	int i;
	
	for (i = ev->z; i < 0; i++) _e_exebuf_prev();
     }
   else if (ev->z > 0) /* down */
     {
	int i;
	
	for (i = ev->z; i > 0; i--) _e_exebuf_next();
     }
   return EINA_TRUE;
}

static Eina_Bool
_e_exebuf_exe_scroll_timer(void *data)
{
   if (exe_scroll_to)
     {
	double spd;

	spd = exebuf_conf->scroll_speed;
	exe_scroll_align = (exe_scroll_align * (1.0 - spd)) + (exe_scroll_align_to * spd);
	return EINA_TRUE;
     }
   exe_scroll_timer = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_e_exebuf_eap_scroll_timer(void *data)
{
   if (eap_scroll_to)
     {
	double spd;

	spd = exebuf_conf->scroll_speed;
	eap_scroll_align = (eap_scroll_align * (1.0 - spd)) + (eap_scroll_align_to * spd);
	return EINA_TRUE;
     }
   eap_scroll_timer = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_e_exebuf_animator(void *data)
{
   if (exe_scroll_to)
     {
	double da;
	
	da = exe_scroll_align - exe_scroll_align_to;
	if (da < 0.0) da = -da;
	if (da < 0.01)
	  {
	     exe_scroll_align = exe_scroll_align_to;
	     exe_scroll_to = 0;
	  }
	e_box_align_set(exe_list_object, 0.5, 1.0 - exe_scroll_align);
     }
   if (eap_scroll_to)
     {
	double da;
	
	da = eap_scroll_align - eap_scroll_align_to;
	if (da < 0.0) da = -da;
	if (da < 0.01)
	  {
	     eap_scroll_align = eap_scroll_align_to;
	     eap_scroll_to = 0;
	  }
	e_box_align_set(eap_list_object, 0.5, eap_scroll_align);
     }
   if ((exe_scroll_to) || (eap_scroll_to)) return EINA_TRUE;
   animator = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_e_exebuf_idler(void *data)
{
   struct stat st;
   struct dirent *dp;
   char *dir;
   char buf[PATH_MAX];

   /* no more path items left - stop scanning */
   if (!exe_path)
     {
	Eina_List *l, *l2;
	E_Exe_List *el;
	E_Exe *ee;
	int different = 0;
	
	/* FIXME: check theat they match or not */
	for (l = exe_list, l2 = exe_list2; l && l2; l = l->next, l2 = l2->next)
	  {
	     if (strcmp(l->data, l2->data))
	       {
		  different = 1;
		  break;
	       }
	  }
	if ((l) || (l2)) different = 1;
	if (exe_list2)
	  {
	     while (exe_list)
	       {
		  free(eina_list_data_get(exe_list));
		  exe_list = eina_list_remove_list(exe_list, exe_list);
	       }
	     exe_list = exe_list2;
	     exe_list2 = NULL;
	  }
	if (different)
	  {
	     el = calloc(1, sizeof(E_Exe_List));
	     if (el)
	       {
		  el->list = NULL;
		  for (l = exe_list; l; l = l->next)
		    {
		       ee = malloc(sizeof(E_Exe));
		       if (ee)
			 {
			    ee->path = eina_stringshare_add(l->data);
			    el->list = eina_list_append(el->list, ee);
			 }
		    }
		  e_config_domain_save("exebuf_exelist_cache", exelist_edd, el);
		  while (el->list)
		    {
		       ee = eina_list_data_get(el->list);
		       eina_stringshare_del(ee->path);
		       free(ee);
		       el->list = eina_list_remove_list(el->list, el->list);
		    }
		  free(el);
	       }
	  }
	exe_list_idler = NULL;
	return EINA_FALSE;
     }
   /* no dir is open - open the first path item */
   if (!exe_dir)
     {
	dir = exe_path->data;
	exe_dir = opendir(dir);
     }
   /* if we have an opened dir - scan the next item */
   if (exe_dir)
     {
	dir = exe_path->data;
	
	dp = readdir(exe_dir);
	if (dp)
	  {
	     if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
	       {
		  snprintf(buf, sizeof(buf), "%s/%s", dir, dp->d_name);
		  if ((stat(buf, &st) == 0) &&
		      ((!S_ISDIR(st.st_mode)) &&
		       (!access(buf, X_OK))))
		    {
		       if (!exe_list)
			 exe_list = eina_list_append(exe_list, strdup(buf));
		       else
			 exe_list2 = eina_list_append(exe_list2, strdup(buf));
		    }
	       }
	  }
	else
	  {
	     /* we reached the end of a dir - remove the dir at the head
	      * of the path list so we advance and next loop we will pick up
	      * the next item, or if null- abort
	      */
	     closedir(exe_dir);
	     exe_dir = NULL;
	     free(eina_list_data_get(exe_path));
	     exe_path = eina_list_remove_list(exe_path, exe_path);
	  }
     }
   /* obviously the dir open failed - so remove the first path item */
   else
     {
	free(eina_list_data_get(exe_path));
	exe_path = eina_list_remove_list(exe_path, exe_path);
     }
   /* we have mroe scannign to do */
   return EINA_TRUE;
}

static Eina_Bool
_e_exebuf_update_timer(void *data)
{
   _e_exebuf_matches_update();
   update_timer = NULL;
   return EINA_FALSE;
}
