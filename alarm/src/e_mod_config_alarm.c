#include "e.h"
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   int new;
   Alarm *al;

   char *name;
   int state;

   char *description;
   int autoremove;
   int open_popup;
   int run_program;
   char *program;

   struct
   {
      Alarm_Sched type;
      int hour;
      int minute;
      char *date;
      int day_monday;
      int day_tuesday;
      int day_wenesday;
      int day_thursday;
      int day_friday;
      int day_saturday;
      int day_sunday;
   } sched;
   struct
   {
      Evas_Object *date;
   } sched_gui;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata, Alarm *al);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void _cb_alarm_test(void *data, void *data2);
static void _cb_alarm_today(void *data, void *data2);
static void _cb_alarm_tomorrow(void *data, void *data2);

void
alarm_config_alarm(Alarm *al) 
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
			     D_("Alarm Configuration : Setup an alarm"), "Alarm", "_e_modules_alarm_alarm_config_dialog", buf, 0, v, al);
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata, cfd->data);

   if (cfdata->al)
     cfdata->al->config_dialog = cfd;
   else
     alarm_config->config_dialog_alarm_new = cfd;

   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   free(cfdata->name);
   free(cfdata->description);
   free(cfdata->program);

   if (cfdata->al)
     cfdata->al->config_dialog = NULL;
   else
     alarm_config->config_dialog_alarm_new = NULL;

   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata, Alarm *al)
{
   if (al)
     {
        cfdata->new = 0;
        cfdata->al = al;
        if (al->state != ALARM_STATE_OFF)
          cfdata->state = 1;
        else
          cfdata->state = 0;
        if (al->name)
          cfdata->name = strdup(al->name);
        else
          cfdata->name = strdup("");
        cfdata->sched.type = al->sched.type;
        cfdata->sched.hour = al->sched.hour;
        cfdata->sched.minute = al->sched.minute;
        if (al->sched.type == ALARM_SCHED_DAY)
          {
             struct tm *st;
             time_t t;
             char buf[20];

             t = al->sched.date_epoch - (al->sched.hour*3600) - (al->sched.minute*60);
             st = localtime(&t);
             strftime(buf, sizeof(buf), "%Y/%m/%d", st);
             cfdata->sched.date = strdup(buf);
          }
        else
          cfdata->sched.date = strdup("");
        cfdata->sched.day_monday = al->sched.day_monday;
        cfdata->sched.day_tuesday = al->sched.day_tuesday;
        cfdata->sched.day_wenesday = al->sched.day_wenesday;
        cfdata->sched.day_thursday = al->sched.day_thursday;
        cfdata->sched.day_friday = al->sched.day_friday;
        cfdata->sched.day_saturday = al->sched.day_saturday;
        cfdata->sched.day_sunday = al->sched.day_sunday;
	cfdata->autoremove = al->autoremove;
	if (al->description)
          cfdata->description = strdup(al->description);
	else
          cfdata->description = strdup("");
        cfdata->open_popup = al->open_popup;
        cfdata->run_program = al->run_program;
        if (al->program)
          cfdata->program = strdup(al->program);
        else
          cfdata->program = strdup("");
     }
   else
     {
	struct tm *st;
	time_t t;
	char buf[20];

        cfdata->new = 1;
        cfdata->state = 1;
        cfdata->name = strdup("");
        cfdata->sched.type = ALARM_SCHED_DEFAULT;

	t = time(NULL);
	st = localtime(&t);
	strftime(buf, sizeof(buf), "%Y/", st);
	cfdata->sched.date = strdup(buf);

	cfdata->autoremove = alarm_config->alarms_autoremove_default;
        cfdata->description = strdup("");
        cfdata->open_popup = alarm_config->alarms_open_popup_default;
        if (alarm_config->alarms_run_program_default)
          cfdata->run_program = ALARM_RUN_PROGRAM_PARENT;
        else
          cfdata->run_program = ALARM_RUN_PROGRAM_NO;
        cfdata->program = strdup("");
     }
}

static void
_common_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata, Evas_Object *o)
{
   Evas_Object *of, *of2, *ob;
   E_Radio_Group *rg;

   of = e_widget_frametable_add(evas, D_("Informations"), 0);

   ob = e_widget_check_add(evas, D_("Active"), &(cfdata->state));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, D_("Name"));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_entry_add(evas, &(cfdata->name), NULL, NULL, NULL);
   e_widget_size_min_set(ob, 200, 25);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, D_("Description"));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_entry_add(evas, &(cfdata->description), NULL, NULL, NULL);
   e_widget_size_min_set(ob, 250, 25);
   e_widget_frametable_object_append(of, ob, 0, 3, 2, 1, 1, 1, 1, 1);
   

   e_widget_table_object_append(o, of, 0, 0, 1, 1, 1, 1, 1, 1);


   of = e_widget_frametable_add(evas, D_("Schedule"), 0);

   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f hour"), 0.0, 23.0, 1.0, 0,
                            NULL, &(cfdata->sched.hour), 130);
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 1, 1, 0, 1, 0);
   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f minute"), 0.0, 59.0, 1.0, 0,
                            NULL, &(cfdata->sched.minute), 130);
   e_widget_frametable_object_append(of, ob, 0, 1, 2, 1, 1, 0, 1, 0);

   rg = e_widget_radio_group_new((int*)&(cfdata->sched.type));

   of2 = e_widget_frametable_add(evas, D_("Days of the week"), 0);

   ob = e_widget_radio_add(evas, "", ALARM_SCHED_WEEK, rg);
   e_widget_frametable_object_append(of2, ob, 0, 1, 2, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, D_("Monday"), &(cfdata->sched.day_monday));
   e_widget_frametable_object_append(of2, ob, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, D_("Tuesday"), &(cfdata->sched.day_tuesday));
   e_widget_frametable_object_append(of2, ob, 0, 3, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, D_("Wenesday"), &(cfdata->sched.day_wenesday));
   e_widget_frametable_object_append(of2, ob, 0, 4, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, D_("Thursday"), &(cfdata->sched.day_thursday));
   e_widget_frametable_object_append(of2, ob, 0, 5, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, D_("Friday"), &(cfdata->sched.day_friday));
   e_widget_frametable_object_append(of2, ob, 0, 6, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, D_("Saturday"), &(cfdata->sched.day_saturday));
   e_widget_frametable_object_append(of2, ob, 1, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, D_("Sunday"), &(cfdata->sched.day_sunday));
   e_widget_frametable_object_append(of2, ob, 1, 3, 1, 1, 1, 1, 1, 1);

   e_widget_frametable_object_append(of, of2, 0, 3, 1, 1, 1, 1, 1, 1);

   of2 = e_widget_frametable_add(evas, D_("One day"), 0);

   ob = e_widget_radio_add(evas, "", ALARM_SCHED_DAY, rg);
   e_widget_frametable_object_append(of2, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, D_("Date (YYYY/MM/DD)"));
   e_widget_frametable_object_append(of2, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   cfdata->sched_gui.date = e_widget_entry_add(evas, &(cfdata->sched.date), NULL, NULL, NULL);
   e_widget_size_min_set(cfdata->sched_gui.date, 100, 25);
   e_widget_frametable_object_append(of2, cfdata->sched_gui.date, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_button_add(evas, D_("Today"), NULL, _cb_alarm_today, cfdata, NULL);
   e_widget_frametable_object_append(of2, ob, 0, 3, 1, 1, 1, 1, 1, 1);
   ob = e_widget_button_add(evas, D_("Tomorrow"), NULL, _cb_alarm_tomorrow, cfdata, NULL);
   e_widget_frametable_object_append(of2, ob, 0, 4, 1, 1, 1, 1, 1, 1);

   e_widget_frametable_object_append(of, of2, 1, 3, 1, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 0, 1, 1, 1, 1, 1, 1, 1);
}

static int
_common_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Alarm *al;
   int error;

   al = alarm_alarm_add(cfdata->state, cfdata->name,
                        cfdata->sched.type,
                        cfdata->sched.date,
                        cfdata->sched.day_monday,
                        cfdata->sched.day_tuesday,
                        cfdata->sched.day_wenesday,
                        cfdata->sched.day_thursday,
                        cfdata->sched.day_friday,
                        cfdata->sched.day_saturday,
                        cfdata->sched.day_sunday,
                        cfdata->sched.hour, cfdata->sched.minute,
			cfdata->autoremove,
			cfdata->description,
                        cfdata->open_popup,
                        cfdata->run_program,
                        cfdata->program, &error);

   if (!al)
     {
        char buf[4096];
        switch (error)
          {
          case ALARM_ADD_ERROR_UNKNOWN:
             snprintf(buf, sizeof(buf),
                      D_("<hilight>Error, The alarm was not added !</hilight><br><br>"
                         "There is an error in the informations / schedule of your alarm"));
             break;
          case ALARM_ADD_ERROR_NAME:
             snprintf(buf, sizeof(buf),
                      D_("<hilight>Error, The alarm was not added !</hilight><br><br>"
                         "You have to enter a name for the alarm"));
             break;
          case ALARM_ADD_ERROR_SCHED_WEEK:
             snprintf(buf, sizeof(buf),
                      D_("<hilight>Error, The alarm was not added !</hilight><br><br>"
                         "You have to select at least one day in the week"));
             break;
          case ALARM_ADD_ERROR_SCHED_DAY:
             snprintf(buf, sizeof(buf),
                      D_("<hilight>Error, The alarm was not added !</hilight><br><br>"
                         "The date you entered for the alarm has incorrect syntax<br><br>"
                         "You have to respect this format :<br>"
                         "   YYYY/MM/DD<br>"
                         "YYYY is the year on 4 numbers<br>"
                         "MM is the month on 2 numbers<br>"
                         "DD is the day on 2 numbers<br>"));
             break;
          case ALARM_ADD_ERROR_SCHED_BEFORE:
             snprintf(buf, sizeof(buf),
                      D_("<hilight>Error, The alarm was not added !</hilight><br><br>"
                         "The date you entered is before now"));
             break;
          }
        e_module_dialog_show(alarm_config->module, D_("Alarm Module Error"), buf);
        return 0;
     }

   if (!cfdata->new)
     alarm_config->alarms = eina_list_remove(alarm_config->alarms, cfdata->al);

   alarm_config->alarms = eina_list_append(alarm_config->alarms, al);
   cfdata->new = 0;
   cfdata->al = al;

   al->config_dialog = cfd;
   alarm_config->config_dialog_alarm_new = NULL;

   /* refresh things */
   if (alarm_config->config_dialog)
     {
        E_Config_Dialog_Data *mcda;
        
        mcda = alarm_config->config_dialog->cfdata;
        alarm_config_refresh_alarms_ilist(mcda);
     }
   if (alarm_config->alarms_details)
     alarm_edje_refresh_details();

   return 1;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *ob;

   o = e_widget_table_add(evas, 0);

   _common_create_widgets(cfd, evas, cfdata, o);

   ob = e_widget_button_add(evas, D_("Test this alarm"), NULL, _cb_alarm_test, cfd, cfdata);

   e_widget_table_object_append(o, ob, 0, 2, 1, 1, 1, 1, 1, 1);

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
   E_Radio_Group *rg;

   o = e_widget_table_add(evas, 0);

   _common_create_widgets(cfd, evas, cfdata, o);

   of = e_widget_frametable_add(evas, D_("Ring Options"), 0);

   ob = e_widget_label_add(evas, D_("Remove alarm"));
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);

   rg = e_widget_radio_group_new(&(cfdata->autoremove));

   if (!alarm_config->alarms_autoremove_default)
     {
        ob = e_widget_radio_add(evas, D_("No [default]"), 0, rg);
        e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);
        ob = e_widget_radio_add(evas, D_("Yes"), 1, rg);
        e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 1, 1, 1);
     }
   else
     {
        ob = e_widget_radio_add(evas, D_("No"), 0, rg);
        e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);
        ob = e_widget_radio_add(evas, D_("Yes [default]"), 1, rg);
        e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 1, 1, 1);
     }

   ob = e_widget_label_add(evas, D_("Open popup"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);

   rg = e_widget_radio_group_new(&(cfdata->open_popup));

   if (!alarm_config->alarms_open_popup_default)
     {
        ob = e_widget_radio_add(evas, D_("No [default]"), 0, rg);
        e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
        ob = e_widget_radio_add(evas, D_("Yes"), 1, rg);
        e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);
     }
   else
     {
        ob = e_widget_radio_add(evas, D_("No"), 0, rg);
        e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
        ob = e_widget_radio_add(evas, D_("Yes [default]"), 1, rg);
        e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);
     }
        
   ob = e_widget_label_add(evas, D_("Run a program"));
   e_widget_frametable_object_append(of, ob, 0, 4, 1, 1, 1, 1, 1, 1);

   rg = e_widget_radio_group_new(&(cfdata->run_program));

   ob = e_widget_radio_add(evas, D_("No"), ALARM_RUN_PROGRAM_NO, rg);
   e_widget_frametable_object_append(of, ob, 0, 5, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, D_("Run default one"), ALARM_RUN_PROGRAM_PARENT, rg);
   e_widget_frametable_object_append(of, ob, 0, 6, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, D_("Run this program"), ALARM_RUN_PROGRAM_OWN, rg);
   e_widget_frametable_object_append(of, ob, 0, 7, 1, 1, 1, 1, 1, 1);

   ob = e_widget_entry_add(evas, &(cfdata->program), NULL, NULL, NULL);
   e_widget_size_min_set(ob, 100, 25);
   e_widget_frametable_object_append(of, ob, 0, 8, 1, 1, 1, 1, 1, 1);

   e_widget_table_object_append(o, of, 1, 1, 1, 1, 1, 1, 1, 1);

   ob = e_widget_button_add(evas, D_("Test this alarm"), NULL, _cb_alarm_test, cfd, cfdata);

   e_widget_table_object_append(o, ob, 0, 2, 2, 1, 1, 1, 1, 1);

   return o;
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   int ret;

   ret = _common_apply_data(cfd, cfdata);

   e_config_save_queue();
   return ret;
}


static void
_cb_alarm_test(void *data, void *data2)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_Data *cfdata;
   char buf[4096];

   cfd = data;
   cfdata = data2;

   if(!_basic_apply_data(cfd, cfdata))
     return;

   if (alarm_alarm_ring(cfdata->al, 1))
     {
        if (alarm_config->alarms_state == ALARM_STATE_OFF)
          snprintf(buf, sizeof(buf),
                   D_("<hilight>Alarm test SUCCEED !</hilight><br><br>"
                      "But DONT FORGET to ACTIVATE the alarms<br>"
                      "via the 'Active' check button on the<br>"
                      "main Alarm configuration panel."));
        else
          snprintf(buf, sizeof(buf),
                   D_("<hilight>Alarm test SUCCEED !</hilight>"));
     }
   else
     {
        snprintf(buf, sizeof(buf),
                 D_("<hilight>Alarm test FAILED !</hilight>"));
     }
   e_module_dialog_show(alarm_config->module, D_("Alarm Module Test Report"), buf);
}

static void
_cb_alarm_today(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   struct tm *st;
   time_t t;
   char buf[20];

   cfdata = data;
   t = time(NULL);
   st = localtime(&t);
   strftime(buf, sizeof(buf), "%Y/%m/%d", st);

   e_widget_entry_text_set(cfdata->sched_gui.date, buf);
}

static void
_cb_alarm_tomorrow(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   struct tm *st;
   time_t t;
   char buf[20];

   cfdata = data;
   t = time(NULL) + 3600*24;
   st = localtime(&t);
   strftime(buf, sizeof(buf), "%Y/%m/%d", st);

   e_widget_entry_text_set(cfdata->sched_gui.date, buf);
}
