#include <e.h>
#include "e_mod_main.h"

#define GOLDEN_RATIO 1.618033989
#define STATUS_EXE_TAG "e/modules/execwatch/status_exe"
#define DBLCLK_EXE_TAG "e/modules/execwatch/dblclk_exe"

typedef struct _Instance Instance;
typedef struct _Execwatch Execwatch;

struct _Instance 
{
   E_Gadcon_Client *gcc;
   Execwatch       *execwatch;
   Ecore_Timer     *poll_timer;

   Ecore_Exe       *status_exe;
   Ecore_Exe       *dblclk_exe;
   Ecore_Exe_Event_Data *read;

   Config_Item     *ci;
   E_Gadcon_Popup  *popup;

   char            *status_exe_result;
};

struct _Execwatch 
{
   Instance *inst;
   Evas_Object *execwatch_obj;
   Evas_Object *icon_obj;
   Evas_Object *icon_custom_obj;
};

static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static Config_Item *_config_item_get(const char *id);
static void _execwatch_display(Instance *inst, char *icon);
static Eina_Bool  _execwatch_status_cmd_exec(void *data);
static int  _execwatch_status_cmd_exit(void *data, int type, void *event);
static void _execwatch_popup_content_create(Instance *inst);
static void _execwatch_popup_destroy(Instance *inst);
static void _cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _menu_cb_post(void *data, E_Menu *m);
static void _menu_cb_configure(void *data, E_Menu * m, E_Menu_Item * mi);
static void _menu_cb_exec(void *data, E_Menu * m, E_Menu_Item * mi);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *execwatch_config = NULL;

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "execwatch", 
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
      e_gadcon_site_is_not_toolbar
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   Execwatch       *execwatch;
   Instance        *inst;
   E_Gadcon_Client *gcc;
   char             buf[4096];

   inst = E_NEW(Instance, 1);   
   inst->ci = _config_item_get(id);

   execwatch = E_NEW(Execwatch, 1);
   execwatch->inst = inst;

   snprintf(buf, sizeof(buf), "%s/execwatch.edj", 
	    e_module_dir_get(execwatch_config->module));
   
   execwatch->execwatch_obj = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(execwatch->execwatch_obj, 
				"base/theme/modules/execwatch", "modules/execwatch/main"))
     edje_object_file_set(execwatch->execwatch_obj, buf, "modules/execwatch/main");
   evas_object_show(execwatch->execwatch_obj);
   execwatch->icon_obj = edje_object_add(gc->evas);
   execwatch->icon_custom_obj = e_icon_add(gc->evas);

   gcc = e_gadcon_client_new(gc, name, id, style, execwatch->execwatch_obj);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->execwatch = execwatch;

   execwatch_config->instances = eina_list_append(execwatch_config->instances, inst);
   evas_object_event_callback_add(execwatch->execwatch_obj, EVAS_CALLBACK_MOUSE_IN,
				  _cb_mouse_in, inst);
   evas_object_event_callback_add(execwatch->execwatch_obj, EVAS_CALLBACK_MOUSE_OUT,
				  _cb_mouse_out, inst);
   evas_object_event_callback_add(execwatch->execwatch_obj, EVAS_CALLBACK_MOUSE_DOWN,
				  _cb_mouse_down, inst);

   if (inst->ci->display_name)
     edje_object_part_text_set(execwatch->execwatch_obj, "display_name",
			       inst->ci->display_name);
   if (inst->ci->status_cmd && strlen(inst->ci->status_cmd) &&
       (inst->ci->poll_time_hours || inst->ci->poll_time_mins || inst->ci->poll_time_secs))
     {
	inst->poll_timer = ecore_timer_add((inst->ci->poll_time_hours + inst->ci->poll_time_mins + inst->ci->poll_time_secs),
					   _execwatch_status_cmd_exec, inst);
	_execwatch_status_cmd_exec(inst);
     }	
   else
     _execwatch_display(inst, "cmd_edit");

   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst;
   Execwatch *execwatch;
   
   inst = gcc->data;
   execwatch = inst->execwatch;

   if (inst->poll_timer) ecore_timer_del(inst->poll_timer);
   if (inst->status_exe) ecore_exe_terminate (inst->status_exe);
   if (inst->popup) _execwatch_popup_destroy(inst);
   if (execwatch->execwatch_obj) evas_object_del(execwatch->execwatch_obj);
   if (execwatch->icon_obj) evas_object_del(execwatch->icon_obj);
   if (execwatch->icon_custom_obj) evas_object_del(execwatch->icon_custom_obj);
   
   execwatch_config->instances = eina_list_remove(execwatch_config->instances, inst);
   E_FREE(execwatch);
   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient) 
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class) 
{
   return "Execwatch";
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas) 
{
   Evas_Object *o;
   char buf[4096];

   if (!execwatch_config->module) return NULL;
   
   snprintf(buf, sizeof(buf), "%s/e-module-execwatch.edj", 
	    e_module_dir_get(execwatch_config->module));
   
   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   Config_Item *ci;

   ci = _config_item_get(NULL);
   return ci->id;
}

static Config_Item *
_config_item_get(const char *id) 
{
   Eina_List   *l;
   Config_Item *ci;
   char buf[128];

   if (!id)
     {
	int  num = 0;

	/* Create id */
	if (execwatch_config->items)
	  {
	     const char *p;
	     ci = eina_list_last(execwatch_config->items)->data;
	     p = strrchr(ci->id, '.');
	     if (p) num = atoi(p + 1) + 1;
	  }
	snprintf(buf, sizeof(buf), "%s.%d", _gc_class.name, num);
	id = buf;
     }
   else
     {
	for (l = execwatch_config->items; l; l = l->next) 
	  {
	     ci = l->data;
	     if (!ci->id) continue;
	     if (!strcmp(ci->id, id))
	       return ci;
	  }
     }

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->display_mode = 0; // default: show icon
   ci->display_name = eina_stringshare_add("Edit!");
   ci->icon_path = eina_stringshare_add("");
   ci->status_cmd = eina_stringshare_add("");
   ci->dblclk_cmd = eina_stringshare_add("");
   ci->okstate_mode = 0;
   ci->okstate_exitcode = 0;
   ci->okstate_string = eina_stringshare_add("");
   ci->okstate_lines = 0;
   ci->refresh_after_dblclk_cmd = 0;
   ci->poll_time_hours = 0.0;
   ci->poll_time_mins = 1.0;
   ci->poll_time_secs = 0.0;

   execwatch_config->items = eina_list_append(execwatch_config->items, ci);
   return ci;
}

static void  
_execwatch_cmd_output_prepare(Instance *inst)
{
   char *d;
   const char *p;
   int textlen=0;

   if (!inst) return;
   if (inst->status_exe_result)
     {
        E_FREE(inst->status_exe_result);
		inst->status_exe_result = NULL;
     }
   if (inst->read && inst->read->lines)
     {
        Ecore_Exe_Event_Data_Line *lines;
        int i=0;

	lines = inst->read->lines;
	for (i=0; lines[i].line; i++)
           {	
              if (inst->ci->display_mode)
	        {
                   if (i > 0)
                     textlen += 4; // <br>
                   /* replace special stringparts
                    * '\t' -> "        "
                    * '<'  -> "&lt;"
                    * '>'  -> "&gt;"
                    * '&'  -> "&amp;"
                    */
                   for (p = lines[i].line; *p != 0; p++)
                     {
                        if (*p == '\t')     textlen += 4; // '    '
                        else if (*p == '<') textlen += 4; // '&lt;'
                        else if (*p == '>') textlen += 4; // '&gt;'
                        else if (*p == '&') textlen += 5; // '&amp;'
                        else textlen++;
                     }
                }
              else
                {
                   if (i > 0)
                     textlen += lines[i].size + 1; // \n
                   else
                     textlen += lines[i].size;
                }
           }
        textlen++; // \0

        inst->status_exe_result = calloc(1, textlen);
        if (!inst->status_exe_result) return;

        inst->status_exe_result[0] = '\0';
	d = inst->status_exe_result;
	for (i = 0; lines[i].line; i++)
	  {
             if (inst->ci->display_mode)
               {
                  if (i > 0)
                    {
                       strcpy(d, "<br>");
                       d += 4;
                    }
                  for (p = lines[i].line; *p != 0; p++)
                    {
                       if (*p == '\t')
                         {
                            strcpy(d, "    ");
                            d += 4;
                         }
                       else if (*p == '<')
                         {
                            strcpy(d, "&lt;");
                            d += 4;
                         }
                       else if (*p == '>')
                         {
                            strcpy(d, "&gt;");
                            d += 4;
                         }
                       else if (*p == '&')
                         {
                            strcpy(d, "&amp;");
                            d += 5;
                         }
                       else
                         {
                            *d = *p;
                            d++;
                         }
                    }
               }
             else
	       {
                  if (i > 0)
                    strcat(inst->status_exe_result, "\n");
                  strcat(inst->status_exe_result, lines[i].line);
               }
	  }   
     }
   else
     {
        char *nodata_text = D_("--- no data received ---");

        textlen = strlen (nodata_text);
        textlen++; // \0

        inst->status_exe_result = calloc(1, textlen);
        if (!inst->status_exe_result) return;
        inst->status_exe_result[0] = '\0';
        strcat(inst->status_exe_result, nodata_text);
     }

   inst->status_exe_result[textlen] = '\0';
   edje_object_part_text_set(inst->execwatch->execwatch_obj, "command_output", inst->status_exe_result);
}

static void
_execwatch_display(Instance *inst, char *icon)
{
   Execwatch *execwatch;
   char m[4096], buf[4096];
   char *text;

   if (!inst) return;
   execwatch = inst->execwatch;
   if (!execwatch) return;

   snprintf(m, sizeof(m), "%s/execwatch.edj", e_module_dir_get(execwatch_config->module));
   snprintf (buf, sizeof(buf), "modules/execwatch/icons/%s", icon);
   if (!e_theme_edje_object_set(execwatch->icon_obj,
				"base/theme/modules/execwatch/icons", buf))
   edje_object_file_set(execwatch->icon_obj, m, buf);
   edje_object_part_swallow(execwatch->execwatch_obj, "icon_status", execwatch->icon_obj);
   
   if (inst->ci->display_mode)
     {
        edje_object_signal_emit(execwatch->execwatch_obj,"e,visibility,display_cmd_output","e");
     }
   else if (inst->ci->icon_path && ecore_file_exists(inst->ci->icon_path))
     {
        e_icon_file_set(execwatch->icon_custom_obj, inst->ci->icon_path);
        edje_object_part_swallow(execwatch->execwatch_obj, "icon_custom", execwatch->icon_custom_obj);
        edje_object_signal_emit(execwatch->execwatch_obj,"e,visibility,display_icon_custom","e");
     }
   else
     {
        edje_object_signal_emit(execwatch->execwatch_obj,"e,visibility,display_icon_status","e");
     }
}

static Eina_Bool
_execwatch_status_cmd_exec(void *data)
{
   Instance *inst;
   Execwatch *execwatch;

   inst = data;
   if (!inst) return EINA_TRUE;
   execwatch = inst->execwatch;
   if (!execwatch) return EINA_TRUE;
   if (inst->status_exe) return EINA_TRUE;
   if (!inst->ci->status_cmd || !strlen(inst->ci->status_cmd)) return EINA_TRUE;

   _execwatch_display(inst, "cmd_exec");
   inst->status_exe = ecore_exe_pipe_run(inst->ci->status_cmd,
				  ECORE_EXE_PIPE_AUTO | ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_ERROR |
				  ECORE_EXE_PIPE_READ_LINE_BUFFERED | ECORE_EXE_PIPE_ERROR_LINE_BUFFERED,
				  inst);
   ecore_exe_tag_set(inst->status_exe, STATUS_EXE_TAG);

   return EINA_TRUE;
}

static Eina_Bool
_execwatch_cmd_exit(void *data, int type, void *event)
{
   Ecore_Exe_Event_Data_Line *lines;
   Ecore_Exe_Event_Del *ev;
   Instance *inst;
   int i=0, old_popup_state=0, old_popup_pinned_state=0;

   ev = event;
   if (!ev) return EINA_TRUE;
   if (!ev->exe) return EINA_TRUE;
   if (!ecore_exe_tag_get(ev->exe)) return EINA_TRUE;
   inst = ecore_exe_data_get(ev->exe);
   if (!inst) return EINA_TRUE;
   if (!inst->ci) return EINA_TRUE;

   if (!strcmp(ecore_exe_tag_get(ev->exe), STATUS_EXE_TAG))
     {
        _execwatch_cmd_output_prepare(inst);

	inst->read = ecore_exe_event_data_get(ev->exe, ECORE_EXE_PIPE_READ);
	inst->status_exe = NULL;

	if (!inst->ci->okstate_mode)
	  {
             // check exitcode
	     if (ev->exit_code == inst->ci->okstate_exitcode)
	       _execwatch_display(inst, "cmd_ok");
	     else
	       _execwatch_display(inst, "cmd_error");
	  }
	else if (inst->read && inst->read->lines)
          {
             int icon_set = 0;

             lines = inst->read->lines;
             for (i = 0; lines[i].line; i++)
                {
                   if (inst->ci->okstate_mode == 1)
                     {
                        // check lines and compare with string
                        if (strstr(lines[i].line, inst->ci->okstate_string))
                          {
                             _execwatch_display(inst, "cmd_ok");
                             icon_set = 1;
                             break;
                          }
                     }
                   else if (inst->ci->okstate_mode == 2)
                     {
                        // check number of returned lines
                        if (i == inst->ci->okstate_lines)
                          {
                             _execwatch_display(inst, "cmd_ok");
                             icon_set = 1;
                          }
                     }
                   if (!icon_set) _execwatch_display(inst, "cmd_error");
                }
          }
        else
          _execwatch_display(inst, "cmd_error");

	if (inst->popup)
	  {
	     old_popup_state = inst->popup->win->visible;
	     old_popup_pinned_state = inst->popup->pinned;
	  }
	_execwatch_popup_content_create(inst);
	if (inst->popup && old_popup_state)
	  {
	     e_gadcon_popup_show(inst->popup);
	     if (old_popup_pinned_state) e_gadcon_popup_toggle_pinned(inst->popup);
	  }
     }
   else if (!strcmp(ecore_exe_tag_get(ev->exe), DBLCLK_EXE_TAG))
     {
	inst->dblclk_exe = NULL;
	if (inst->ci->refresh_after_dblclk_cmd) _execwatch_status_cmd_exec(inst);
     }

   return EINA_TRUE;
}

static void
_execwatch_popup_content_create(Instance *inst)
{
   Evas_Object *o, *of, *ob;
   Evas *evas;
   Ecore_Exe_Event_Data_Line *lines;
   Evas_Coord mw, mh;
   time_t current_time;
   struct tm *local_time;
   char buf[64];
   char *text; 
   int i=0, textlen=0;

   if (!inst->ci->status_cmd || !strlen(inst->ci->status_cmd)) return;

   if (inst->popup) _execwatch_popup_destroy(inst);	// FIXME: reopen popup after content recreation
   inst->popup = e_gadcon_popup_new(inst->gcc);

   current_time = time (NULL);
   local_time = localtime (&current_time);

   evas = inst->popup->win->evas;
   o = e_widget_list_add(evas, 0, 0);
   snprintf(buf, sizeof(buf), "Output (%02d:%02d): %s", local_time->tm_hour,
							  local_time->tm_min,
							  inst->ci->display_name);
   of = e_widget_framelist_add (evas, buf, 0);
   ob = e_widget_textblock_add(evas);
   if (inst->status_exe_result)
     e_widget_textblock_plain_set(ob, inst->status_exe_result);
   e_widget_size_min_set(ob, 240, 120);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   e_widget_size_min_get(o, &mw, &mh);
   if ((double) mw / mh > GOLDEN_RATIO)
     mh = mw / GOLDEN_RATIO;
   else if ((double) mw / mh < GOLDEN_RATIO - (double) 1)
     mw = mh * (GOLDEN_RATIO - (double) 1);
   e_widget_size_min_set(o, mw, mh);

   e_gadcon_popup_content_set(inst->popup, o);
}

static void
_execwatch_popup_destroy(Instance *inst)
{
   if (!inst->popup) return;
   e_object_del(E_OBJECT(inst->popup));
}

static void
_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;

   if (!(inst = data)) return;

   if (!inst->ci->display_mode)
     e_gadcon_popup_show(inst->popup);
}

static void
_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;

   if (!(inst = data)) return;
   e_gadcon_popup_hide(inst->popup);
}

static void
_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;

   if (ev->button == 1)
     {
	e_gadcon_popup_toggle_pinned(inst->popup);

	if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
	  {
	     if (!inst->dblclk_exe &&
		 inst->ci->dblclk_cmd &&
		 strlen(inst->ci->dblclk_cmd))
	       {
		  inst->dblclk_exe = ecore_exe_run(inst->ci->dblclk_cmd, inst);
		  ecore_exe_tag_set(inst->dblclk_exe, DBLCLK_EXE_TAG);
	       }
	  }
     }
   if (ev->button == 3)
     {
	E_Menu *m;
	E_Menu_Item *mi;
	int cx, cy, cw, ch;

	m = e_menu_new();

	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, D_("Settings"));
	e_util_menu_item_theme_icon_set(mi, "preferences-system");
	e_menu_item_callback_set(mi, _menu_cb_configure, inst);

	m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

	mi = e_menu_item_new_relative(m, NULL);
	e_menu_item_label_set(mi, D_("Run Command"));
	e_menu_item_callback_set(mi, _menu_cb_exec, inst);

	e_menu_post_deactivate_callback_set(m, _menu_cb_post, inst);
	execwatch_config->menu = m;
	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy, &cw, &ch);
	e_menu_activate_mouse(m,
			      e_util_zone_current_get(e_manager_current_get()),
			      cx + ev->output.x, cy + ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
				 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }	
}

static void
_menu_cb_post(void *data, E_Menu *m)
{
   if (!execwatch_config->menu) return;
   e_object_del(E_OBJECT(execwatch_config->menu));
   execwatch_config->menu = NULL;
}

static void
_menu_cb_configure(void *data, E_Menu * m, E_Menu_Item * mi)
{
   Instance *inst;

   inst = data;
   _config_execwatch_module(inst->ci);
}

static void
_menu_cb_exec(void *data, E_Menu * m, E_Menu_Item * mi)
{
   Instance *inst;

   inst = data;
   _execwatch_status_cmd_exec(inst);
}

void
_execwatch_config_updated(Config_Item *ci)
{
   Eina_List *l;

   if (!execwatch_config) return;
   for (l = execwatch_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (!inst) continue;
	if (inst->ci != ci) continue;

	if (inst->poll_timer) ecore_timer_del(inst->poll_timer);
	if (inst->status_exe) ecore_exe_terminate (inst->status_exe);
	if (inst->ci->display_name)
	  edje_object_part_text_set(inst->execwatch->execwatch_obj, "display_name",
				    inst->ci->display_name);
	inst->poll_timer = ecore_timer_add((inst->ci->poll_time_hours + inst->ci->poll_time_mins + inst->ci->poll_time_secs),
					    _execwatch_status_cmd_exec, inst);
	_execwatch_status_cmd_exec(inst);
     }
}

EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION, "Execwatch"
};

EAPI void *
e_modapi_init(E_Module *m) 
{
   conf_item_edd = E_CONFIG_DD_NEW("execwatch_config_Item", Config_Item);
   conf_edd = E_CONFIG_DD_NEW("execwatch_config", Config);
   
   #undef T
   #define T Config_Item
   #undef D
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, display_mode, INT);
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, display_name, STR);
   E_CONFIG_VAL(D, T, icon_path, STR);
   E_CONFIG_VAL(D, T, status_cmd, STR);
   E_CONFIG_VAL(D, T, dblclk_cmd, STR);
   E_CONFIG_VAL(D, T, okstate_mode, INT);
   E_CONFIG_VAL(D, T, okstate_exitcode, INT);
   E_CONFIG_VAL(D, T, okstate_string, STR);
   E_CONFIG_VAL(D, T, okstate_lines, INT);
   E_CONFIG_VAL(D, T, refresh_after_dblclk_cmd, INT);
   E_CONFIG_VAL(D, T, poll_time_hours, DOUBLE);
   E_CONFIG_VAL(D, T, poll_time_mins, DOUBLE);
   E_CONFIG_VAL(D, T, poll_time_secs, DOUBLE);
   
   #undef T
   #define T Config
   #undef D
   #define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);
   
   execwatch_config = e_config_domain_load("module.execwatch", conf_edd);
   if (!execwatch_config) 
     {
	Config_Item *ci;
	
	execwatch_config = E_NEW(Config, 1);
	ci = E_NEW(Config_Item, 1);
	ci->id = eina_stringshare_add("0");
	ci->display_mode = 0; // default: show icon
	ci->display_name = eina_stringshare_add("Edit!");
	ci->icon_path = eina_stringshare_add("");
	ci->status_cmd = eina_stringshare_add("");
	ci->dblclk_cmd = eina_stringshare_add("");
	ci->okstate_mode = 0;
	ci->okstate_exitcode = 0;
	ci->okstate_string = eina_stringshare_add("");
	ci->okstate_lines = 0;
	ci->refresh_after_dblclk_cmd = 0;
	ci->poll_time_hours = 0.0;
	ci->poll_time_mins = 1.0;
	ci->poll_time_secs = 0.0;
	
	execwatch_config->items = eina_list_append(execwatch_config->items, ci);
     }
   
   execwatch_config->module = m;
   e_gadcon_provider_register(&_gc_class);
   execwatch_config->exe_del = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _execwatch_cmd_exit, NULL);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m) 
{
   execwatch_config->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);

   if (execwatch_config->exe_del)
     ecore_event_handler_del(execwatch_config->exe_del);
   if (execwatch_config->config_dialog)
     e_object_del(E_OBJECT(execwatch_config->config_dialog));
   if (execwatch_config->menu)
     {
        e_menu_post_deactivate_callback_set(execwatch_config->menu, NULL, NULL);
        e_object_del(E_OBJECT(execwatch_config->menu));
        execwatch_config->menu = NULL;
     }

   while (execwatch_config->items) 
     {
	Config_Item *ci;
	
	ci = execwatch_config->items->data;
	if (ci->id) eina_stringshare_del(ci->id);
	execwatch_config->items = eina_list_remove_list(execwatch_config->items, execwatch_config->items);
	E_FREE(ci);
     }

   E_FREE(execwatch_config);
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m) 
{
   e_config_domain_save("module.execwatch", conf_edd, execwatch_config);
   return 1;
}
