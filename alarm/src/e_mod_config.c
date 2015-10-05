#include "e.h"
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   int    time_format;

   Evas_Object *alarms_ilist;
   E_Config_Dialog *alarms_cfd;
   int    alarms_active;
   int    alarms_autoremove_default;
   int    alarms_details;
   int    alarms_open_popup_default;
   int    alarms_run_program_default;
   char  *alarms_program_default;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata);
static void         _common_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata, Evas_Object *o);
static int          _common_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void _cb_alarms_list(void *data);
static void _cb_alarm_add(void *data, void *data2);
static void _cb_alarm_del(void *data, void *data2);
static void _cb_alarm_config(void *data, void *data2);
static void _cb_alarm_duplicate(void *data, void *data2);

void
alarm_config_module(void) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   char buf[4096];
   
   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;

   snprintf(buf, sizeof(buf), "%s/module.edj", e_module_dir_get(alarm_config->module));
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("Alarm Configuration"), "Alarm", "_e_modules_alarm_config_dialog", buf, 0, v, NULL);
   alarm_config->config_dialog = cfd;
}

void
alarm_config_refresh_alarms_ilist(E_Config_Dialog_Data *cfdata)
{
   Eina_List *l;
   int pos;
   int wmw, wmh;

   pos = e_widget_ilist_selected_get(cfdata->alarms_ilist);
   e_widget_ilist_clear(cfdata->alarms_ilist);

   for(l=alarm_config->alarms; l; l=eina_list_next(l))
     {
        Evas_Object *ic;
        Alarm *al;
        char buf[1024], bufdate[15];
        
        al = eina_list_data_get(l);
        
        if (al->state == ALARM_STATE_OFF)
          ic = NULL;
        else
          {
             ic = e_icon_add(evas_object_evas_get(cfdata->alarms_ilist));
             if (!alarm_config->theme) e_util_edje_icon_set(ic, THEME_ICON_ALARM_ON);
             else e_icon_file_edje_set(ic, alarm_config->theme, THEME_ICON_ALARM_ON);
          }
        if (al->sched.type == ALARM_SCHED_DAY)
          {
             struct tm *st;
             time_t t;
             
             t = al->sched.date_epoch - (al->sched.hour*3600) - (al->sched.minute*60);
             st = localtime(&t);
             strftime(bufdate, sizeof(bufdate), "%m/%d", st);
             snprintf(buf, sizeof(buf), "%s (%s %.2d:%.2d)", al->name, bufdate, al->sched.hour, al->sched.minute);
          }
        else
          {
             // TODO: show the days of the week
             snprintf(buf, sizeof(buf), "%s (Weekly %.2d:%.2d)", al->name, al->sched.hour, al->sched.minute);
          }
        e_widget_ilist_append(cfdata->alarms_ilist, ic, buf, _cb_alarms_list, cfdata, NULL);
     }

   e_widget_size_min_get(cfdata->alarms_ilist, &wmw, &wmh);
   if (eina_list_count(alarm_config->alarms) > 0) 
     e_widget_size_min_set(cfdata->alarms_ilist, wmw, 200);
   else 
     e_widget_size_min_set(cfdata->alarms_ilist, 165, 100);
   
   e_widget_ilist_go(cfdata->alarms_ilist);
   e_widget_ilist_selected_set(cfdata->alarms_ilist, pos);
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   free(cfdata->alarms_program_default);
   alarm_config->config_dialog = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   cfdata->time_format = alarm_config->time_format;

   if (alarm_config->alarms_state == ALARM_STATE_OFF)
     cfdata->alarms_active = ALARM_STATE_OFF;
   else
     cfdata->alarms_active = ALARM_STATE_ON;
   cfdata->alarms_autoremove_default = alarm_config->alarms_autoremove_default;
   cfdata->alarms_details = alarm_config->alarms_details;
   cfdata->alarms_open_popup_default = alarm_config->alarms_open_popup_default;
   cfdata->alarms_run_program_default = alarm_config->alarms_run_program_default;
   if (alarm_config->alarms_program_default)
     cfdata->alarms_program_default = strdup(alarm_config->alarms_program_default);
   else
     cfdata->alarms_program_default = strdup("");
}

static void
_common_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata, Evas_Object *o)
{
   Evas_Object *of, *ob;

   of = e_widget_frametable_add(evas, D_("Alarm"), 0);

   ob = e_widget_check_add(evas, D_("Active"), &(cfdata->alarms_active));
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 1, 1, 1, 1, 10);

   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_ilist_selector_set(ob, 1);
   cfdata->alarms_ilist = ob;
   alarm_config_refresh_alarms_ilist(cfdata);
   e_widget_frametable_object_append(of, ob, 0, 1, 6, 1, 1, 1, 1, 1);

   ob = e_widget_button_add(evas, D_("Add"), NULL, _cb_alarm_add, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 0, 2, 2, 1, 1, 1, 1, 1);
   ob = e_widget_button_add(evas, D_("Delete"), NULL, _cb_alarm_del, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 2, 2, 2, 1, 1, 1, 1, 1);
   ob = e_widget_button_add(evas, D_("Configure"), NULL, _cb_alarm_config, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 4, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_button_add(evas, D_("Duplicate"), NULL, _cb_alarm_duplicate, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 5, 2, 1, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 0, 0, 1, 2, 1, 1, 1, 1);
}

static int
_common_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   if (alarm_config->alarms_state != cfdata->alarms_active)
     {
        alarm_config->alarms_state = cfdata->alarms_active;
        if (cfdata->alarms_active)
          alarm_edje_signal_emit(EDJE_SIG_SEND_ALARM_STATE_ON);
        else
          alarm_edje_signal_emit(EDJE_SIG_SEND_ALARM_STATE_OFF);
     }

   return 1;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o;
   
   o = e_widget_table_add(evas, 0);

   _common_create_widgets(cfd, evas, cfdata, o);

   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   int ret;

   ret = _common_apply_data(cfd, cfdata);

   e_config_save_queue();
   return ret;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ob;

   o = e_widget_table_add(evas, 0);

   _common_create_widgets(cfd, evas, cfdata, o);


   of = e_widget_frametable_add(evas, D_("Alarm Options"), 0);

   /*
     ob = e_widget_label_add(evas, D_("Time format"));
     e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);
     rg = e_widget_radio_group_new(&(cfdata->time_format));
     ob = e_widget_radio_add(evas, D_("12h"), TIME_FORMAT_12, rg);
     e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);
     ob = e_widget_radio_add(evas, D_("24h"), TIME_FORMAT_24, rg);
     e_widget_frametable_object_append(of, ob, 2, 0, 1, 1, 1, 1, 1, 1);
   */

   ob = e_widget_check_add(evas, D_("Show next alarm"), &(cfdata->alarms_details));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 1, 0, 1, 1, 1, 0, 1, 0);


   of = e_widget_frametable_add(evas, D_("Alarms Defaults"), 0);

   ob = e_widget_check_add(evas, D_("Open popup"), &(cfdata->alarms_open_popup_default));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);

   ob = e_widget_check_add(evas, D_("Run program"), &(cfdata->alarms_run_program_default));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);

   ob = e_widget_entry_add(evas, &(cfdata->alarms_program_default), NULL, NULL, NULL);
   e_widget_size_min_set(ob, 80, 25);
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);

   ob = e_widget_check_add(evas, D_("Auto-Remove alarms"), &(cfdata->alarms_autoremove_default));
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 1, 1, 1, 1, 1, 0, 1, 0);

   return o;
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   int ret;
   
   ret = _common_apply_data(cfd, cfdata);

   alarm_config->time_format = cfdata->time_format;
   if (alarm_config->alarms_details != cfdata->alarms_details)
     {
        alarm_config->alarms_details = cfdata->alarms_details;
        alarm_details_change();
     }

   alarm_config->alarms_autoremove_default = cfdata->alarms_autoremove_default;
   alarm_config->alarms_open_popup_default = cfdata->alarms_open_popup_default;
   alarm_config->alarms_run_program_default = cfdata->alarms_run_program_default;
   if (alarm_config->alarms_program_default)
     eina_stringshare_del(alarm_config->alarms_program_default);
   alarm_config->alarms_program_default = eina_stringshare_add(cfdata->alarms_program_default);

   e_config_save_queue();
   return ret;
}



static void
_cb_alarms_list(void *data)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
}

static void
_cb_alarm_add(void *data, void *data2)
{
   if (alarm_config->config_dialog_alarm_new) return;

   alarm_config_alarm(NULL);
}

static void
_cb_alarm_del(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   Alarm *al;

   cfdata = data;
   al = eina_list_nth(alarm_config->alarms,
                      e_widget_ilist_selected_get(cfdata->alarms_ilist));
   if (!al)
     return;

   alarm_alarm_del(al);
   e_config_save_queue();
}

static void
_cb_alarm_config(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   Alarm *al;

   cfdata = data;
   al = eina_list_nth(alarm_config->alarms,
                      e_widget_ilist_selected_get(cfdata->alarms_ilist));
   if (!al) return;
   if (al->config_dialog) return;

   alarm_config_alarm(al);
}

static void
_cb_alarm_duplicate(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   Alarm *al, *al_new;

   cfdata = data;
   al = eina_list_nth(alarm_config->alarms,
                      e_widget_ilist_selected_get(cfdata->alarms_ilist));
   if (!al) return;

   al_new = alarm_alarm_duplicate(al);
   alarm_config->alarms = eina_list_append(alarm_config->alarms, al_new);

   /* refresh things */
   if (alarm_config->config_dialog)
     {
        E_Config_Dialog_Data *mcda;
        
        mcda = alarm_config->config_dialog->cfdata;
        alarm_config_refresh_alarms_ilist(mcda);
     }
}
