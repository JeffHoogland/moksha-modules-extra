#include "e.h"
#include "e_mod_main.h"
#include <config.h>

#define ALARM_ADD_FAIL(errcode) if (al) alarm_alarm_del(al); if (error) *error = errcode; return NULL;

/* module requirements */
EAPI E_Module_Api e_modapi = 
   {
      E_MODULE_API_VERSION,
      "Alarm"
   };

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static const E_Gadcon_Client_Class _gadcon_class =
   {
      GADCON_CLIENT_CLASS_VERSION,
      "alarm",
      {
         _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
         e_gadcon_site_is_not_toolbar
      },
      E_GADCON_CLIENT_STYLE_PLAIN
   };

/* alarm module specifics */
typedef struct _Instance Instance;
struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object     *obj;
   E_Gadcon_Orient orient;
};

static int    _alarm_check_date(Alarm *al, int strict);
static void   _alarm_snooze(Alarm *al);
static void   _alarm_cb_dialog_snooze_ok(void *data, E_Dialog *dia);
static void   _alarm_cb_dialog_snooze_cancel(void *data, E_Dialog *dia);
static void   _alarm_dialog_snooze_delete(E_Dialog *dia, Alarm *al);
static double _epoch_find_date(char *date, int hour, int minute);
static double _epoch_find_next(int day_monday, int day_tuesday, int day_wenesday, int day_thursday, int day_friday, int day_saturday, int day_sunday, int hour, int minute);
static void   _button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void   _menu_cb_deactivate_post(void *data, E_Menu *m);
static void   _menu_cb_alarm_snooze(void *data, E_Menu *m, E_Menu_Item *mi);
static void   _menu_cb_alarm_add(void *data, E_Menu *m, E_Menu_Item *mi);
static void   _menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi);
static void   _cb_edje_alarm_state_on(void *data, Evas_Object *obj, const char *emission, const char *source);
static void   _cb_edje_alarm_state_off(void *data, Evas_Object *obj, const char *emission, const char *source);
static void   _cb_edje_alarm_ring_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static Eina_Bool    _cb_alarms_ring_etimer(void *data);
static Eina_Bool    _cb_alarm_snooze_time(void *data);

static E_Config_DD *_conf_edd = NULL;
static E_Config_DD *_alarms_edd = NULL;
Config *alarm_config = NULL;


/*
 * Gadcon functions
 */

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;
   char buf[PATH_MAX];
   
   inst = E_NEW(Instance, 1);
   
   o = edje_object_add(gc->evas);
   snprintf(buf, sizeof(buf), "%s/alarm.edj", e_module_dir_get(alarm_config->module));
   if (!e_theme_edje_object_set
       (o, "base/theme/modules/alarm", "modules/alarm/main"))
   edje_object_file_set(o, buf, THEME_MAIN);
     
   edje_object_signal_callback_add(o, EDJE_SIG_RECV_ALARM_STATE_ON,
           _cb_edje_alarm_state_on, NULL);
   edje_object_signal_callback_add(o, EDJE_SIG_RECV_ALARM_STATE_OFF,
           _cb_edje_alarm_state_off, NULL);
   edje_object_signal_callback_add(o, EDJE_SIG_RECV_ALARM_RING_STOP,
           _cb_edje_alarm_ring_stop, NULL);

   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   
   inst->gcc = gcc;
   inst->obj = o;
   inst->orient = E_GADCON_ORIENT_HORIZ;

   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
          _button_cb_mouse_down, inst);

   alarm_config->instances = eina_list_append(alarm_config->instances, inst);

   alarm_details_change();

   switch (alarm_config->alarms_state)
     {
     case ALARM_STATE_ON:
        edje_object_signal_emit(o, EDJE_SIG_SEND_ALARM_STATE_ON);
        break;
     case ALARM_STATE_OFF:
        edje_object_signal_emit(o, EDJE_SIG_SEND_ALARM_STATE_OFF);
        break;
     case ALARM_STATE_RINGING:
        edje_object_signal_emit(o, EDJE_SIG_SEND_ALARM_RING_START);
        break;
     }

   e_config_save_queue();

   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;
   
   inst = gcc->data;
   evas_object_del(inst->obj);
   alarm_config->instances = eina_list_remove(alarm_config->instances, inst);
   free(inst);

   e_config_save_queue();
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   Instance *inst;
   int w, h;

   inst = gcc->data;
   if (orient != -1)
      inst->orient = orient;

   /* details or not */
   if (alarm_config->alarms_details)
     {
        w = 40;
        h = 16;
     }
   else
     {
        w = 16;
        h = 16;
     }

   /* vertical */
   switch (inst->orient)
     {
     case E_GADCON_ORIENT_VERT:
     case E_GADCON_ORIENT_LEFT:
     case E_GADCON_ORIENT_RIGHT:
     case E_GADCON_ORIENT_CORNER_LT:
     case E_GADCON_ORIENT_CORNER_RT:
     case E_GADCON_ORIENT_CORNER_LB:
     case E_GADCON_ORIENT_CORNER_RB:
        w = 16;
        h = 16;
     default:
	break;
     }

   e_gadcon_client_aspect_set(gcc, w, h);
   e_gadcon_client_min_size_set(gcc, w, h);
}
   
static const char *
_gc_label(const E_Gadcon_Client_Class *client_class)
{
   return D_("Alarm");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas)
{
   Evas_Object *o;
   char buf[4096];
   
   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/alarm.edj",
      e_module_dir_get(alarm_config->module));
   edje_object_file_set(o, buf, "modules/alarm/main");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   return _gadcon_class.name;
}


/*
 * Alarm functions
 */

Alarm *
alarm_alarm_add(int state, char *name, int type,
                char *date, int day_monday, int day_tuesday, int day_wenesday, int day_thursday, int day_friday, int day_saturday, int day_sunday,
                int hour, int minute,
                int autoremove, char *description,
                int open_popup, int run_program, char *program,
                int *error)
{
   Alarm *al = NULL;

   if (!name)
     {
        ALARM_ADD_FAIL(ALARM_ADD_ERROR_NAME);
     }
   if (!strlen(name))
     {
        ALARM_ADD_FAIL(ALARM_ADD_ERROR_NAME);
     }

   al = E_NEW(Alarm, 1);

   if (state)
     al->state = ALARM_STATE_ON;
   else
     al->state = ALARM_STATE_OFF;
   al->name = eina_stringshare_add(name);
   al->sched.type = type;
   switch(type)
     {
     case ALARM_SCHED_DAY:
        if ( !(al->sched.date_epoch = _epoch_find_date(date, hour, minute)) )
          {
             ALARM_ADD_FAIL(ALARM_ADD_ERROR_SCHED_DAY);
          }
        if (al->sched.date_epoch <= ecore_time_unix_get())
          {
             ALARM_ADD_FAIL(ALARM_ADD_ERROR_SCHED_BEFORE);
          }
        break;

     case ALARM_SCHED_WEEK:
        al->sched.day_monday = day_monday;
        al->sched.day_tuesday = day_tuesday;
        al->sched.day_wenesday = day_wenesday;
        al->sched.day_thursday = day_thursday;
        al->sched.day_friday = day_friday;
        al->sched.day_saturday = day_saturday;
        al->sched.day_sunday = day_sunday;
        if ( !(al->sched.date_epoch = _epoch_find_next(day_monday,
                                                       day_tuesday,
                                                       day_wenesday,
                                                       day_thursday,
                                                       day_friday,
                                                       day_saturday,
                                                       day_sunday,
                                                       hour, minute)) )
          {
             ALARM_ADD_FAIL(ALARM_ADD_ERROR_SCHED_WEEK);
          }
        break;

     default:
        ALARM_ADD_FAIL(ALARM_ADD_ERROR_UNKNOWN);
     }

   al->sched.hour = hour;
   al->sched.minute = minute;
   al->snooze.minute = ALARM_SNOOZE_MINUTE_DEFAULT;
   al->snooze.hour = ALARM_SNOOZE_HOUR_DEFAULT;
   al->autoremove = autoremove;
   if (description)
     al->description = eina_stringshare_add(description);
   al->open_popup = open_popup;
   al->run_program = run_program;
   if (program)
     if (strlen(program))
       al->program = eina_stringshare_add(program);

   if (!_alarm_check_date(al, 1))
     {
        ALARM_ADD_FAIL(ALARM_ADD_ERROR_UNKNOWN);
     }
   
   if (!alarm_config->alarms_ring_etimer)
     alarm_config->alarms_ring_etimer = ecore_timer_add(ALARMS_CHECK_TIMER,
                                                        _cb_alarms_ring_etimer,
                                                        NULL);

   return al;
}

void
alarm_alarm_del(Alarm *al)
{
   if (al->name)
     eina_stringshare_del(al->name);
   if (al->state == ALARM_STATE_RINGING)
     alarm_alarm_ring_stop(al, 0);
   if (al->config_dialog)
     e_object_del(E_OBJECT(al->config_dialog));

   if (al->description)
     eina_stringshare_del(al->description);
   if (al->program)
     eina_stringshare_del(al->program);

   if (al->snooze.dia)
     e_object_del(E_OBJECT(al->snooze.dia));
   if (al->snooze.etimer)
     ecore_timer_del(al->snooze.etimer);

   alarm_config->alarms = eina_list_remove(alarm_config->alarms, al);
   free(al);

   /* refresh things */
   if (alarm_config->config_dialog)
     {
        E_Config_Dialog_Data *mcda;

        mcda = alarm_config->config_dialog->cfdata;
        alarm_config_refresh_alarms_ilist(mcda);
     }
   if (alarm_config->alarms_details)
     alarm_edje_refresh_details();

   if ( !eina_list_count(alarm_config->alarms) &&
        alarm_config->alarms_ring_etimer )
     {
        ecore_timer_del(alarm_config->alarms_ring_etimer);
        alarm_config->alarms_ring_etimer = NULL;
     }
}

Alarm *
alarm_alarm_duplicate(Alarm *al)
{
   Alarm *al_new;

   al_new = E_NEW(Alarm, 1);

   if (al->name)
     al_new->name = eina_stringshare_add(al->name);
   al_new->state = al->state;
   if (al->description)
     al_new->description = eina_stringshare_add(al->description);
   al_new->autoremove = al->autoremove;
   al_new->open_popup = al->open_popup;
   al_new->run_program = al->run_program;
   if (al->program)
     al_new->program = eina_stringshare_add(al->program);
   al_new->sched.type = al->sched.type;
   al_new->sched.date_epoch = al->sched.date_epoch;
   al_new->sched.hour = al->sched.hour;
   al_new->sched.minute = al->sched.minute;
   al_new->sched.day_monday = al->sched.day_monday;
   al_new->sched.day_tuesday = al->sched.day_tuesday;
   al_new->sched.day_wenesday = al->sched.day_wenesday;
   al_new->sched.day_thursday = al->sched.day_thursday;
   al_new->sched.day_friday = al->sched.day_friday;
   al_new->sched.day_saturday = al->sched.day_saturday;
   al_new->sched.day_sunday = al->sched.day_sunday;
   al_new->snooze.hour = al->snooze.hour;
   al_new->snooze.minute = al->snooze.minute;
   al_new->snooze.remember = al->snooze.remember;

   return al_new;
}

int
alarm_alarm_ring(Alarm *al, int test)
{
   char buf[4096];
   int ret = 1;

   alarm_config->alarms_state = ALARM_STATE_RINGING;
   if (!test)
     al->state = ALARM_STATE_RINGING;
   alarm_config->alarms_ringing_nb++;
   alarm_edje_signal_emit(EDJE_SIG_SEND_ALARM_RING_START);

   // TODO: real popups
   if (al->open_popup)
     {
        snprintf(buf, sizeof(buf),
                 D_("<hilight>Alarm : %s</hilight><br><br>"
                    "%s"),
                 al->name, (al->description) ? al->description : "" );
        e_module_dialog_show(alarm_config->module, D_("Alarm Module Popup"), buf);
     }

   if (al->run_program != ALARM_RUN_PROGRAM_NO)
     {
        Ecore_Exe *exe;
        
        if (al->run_program == ALARM_RUN_PROGRAM_PARENT)
          {
             exe = ecore_exe_pipe_run(alarm_config->alarms_program_default,
                                      ECORE_EXE_USE_SH, NULL);
          }
        else
          {
             exe = ecore_exe_pipe_run(al->program,
                                      ECORE_EXE_USE_SH, NULL);
          }

        if (exe > 0)
          ecore_exe_free(exe);
        else
          {
             snprintf(buf, sizeof(buf),
                      D_("<hilight>Failed !</hilight><br><br>"
                         "Alarm couln't launch the program you specified"));
             e_module_dialog_show(alarm_config->module, D_("Alarm Module Error"), buf);
             ret = 0;
          }
     }

   _alarm_check_date(al, 0);

   if (alarm_config->alarms_details)
     alarm_edje_refresh_details();

   e_config_save_queue();

   return ret;
}

void
alarm_alarm_ring_stop(Alarm *al, int check)
{
   Eina_List *l;

   if (al)
     {
        if (al->state != ALARM_STATE_RINGING) return;

        al->state = ALARM_STATE_ON;
        alarm_config->alarms_ringing_nb--;
        if (check) _alarm_check_date(al, 0);
     }
   else
     {
        for (l=alarm_config->alarms; l; l=eina_list_next(l))
          {
             al = eina_list_data_get(l);
             if (al->state == ALARM_STATE_RINGING)
               {
                  printf("Stop alarm %s\n", al->name);
                  al->state = ALARM_STATE_ON;
                  alarm_config->alarms_ringing_nb--;
                  if (check) _alarm_check_date(al, 0);
               }
          }
     }

   if (alarm_config->alarms_ringing_nb == 0)
     {
        alarm_edje_signal_emit(EDJE_SIG_SEND_ALARM_STATE_ON);
        alarm_config->alarms_state = ALARM_STATE_ON;
     }

   e_config_save_queue();
}

void
alarm_edje_signal_emit(const char *source, const char *message)
{
   Eina_List *l;

   for (l=alarm_config->instances; l; l=eina_list_next(l))
     {
        Instance *inst;

        inst = eina_list_data_get(l);
        edje_object_signal_emit(inst->obj, source, message);
     }
}

void
alarm_edje_text_set(char *part, char *text)
{
   Eina_List *l;

   for (l=alarm_config->instances; l; l=eina_list_next(l))
     {
        Instance *inst;

        inst = eina_list_data_get(l);
        edje_object_part_text_set(inst->obj, part, text);
     }
}

void
alarm_edje_refresh_details(void)
{
   Eina_List *l;
   Alarm *al;
   Alarm *first;
   double first_epoch;
   char buf[32];

   first = NULL;
   first_epoch = LONG_MAX;

   for (l=alarm_config->alarms; l; l=eina_list_next(l))
     {
        al = eina_list_data_get(l);
        if (al->state == ALARM_STATE_OFF)
          continue;
        if (al->sched.date_epoch < first_epoch)
          {
             first = al;
             first_epoch = al->sched.date_epoch;
          }
     }

   if (!first)
     {
        alarm_edje_text_set("alarm_txt_line", "-- --:--");
        return;
     }
   else
     {
        struct tm *st;
        time_t t;
        char bufdate[32];
        
        t = first_epoch - (first->sched.hour*3600) - (first->sched.minute*60);
        st = localtime(&t);
        strftime(bufdate, sizeof(bufdate), "%d", st);
        snprintf(buf, sizeof(buf), "%.2s %.2d:%.2d",
                 bufdate, first->sched.hour, first->sched.minute);
        alarm_edje_text_set("alarm_txt_line", buf);
     }
}

void
alarm_details_change(void)
{
   Eina_List *l;

   for(l=alarm_config->instances; l; l=eina_list_next(l))
     {
        Instance *i;

        i = eina_list_data_get(l);
        _gc_orient(i->gcc, -1);
     }

   if (alarm_config->alarms_details)
     {
        alarm_edje_refresh_details();
        alarm_edje_signal_emit(EDJE_SIG_SEND_ALARM_DETAILS_ON);
     }
   else
     alarm_edje_signal_emit(EDJE_SIG_SEND_ALARM_DETAILS_OFF);
}

static int
_alarm_check_date(Alarm *al, int strict)
{
   switch(al->sched.type)
     {
     case ALARM_SCHED_DAY:
        if (al->sched.date_epoch <= ecore_time_unix_get())
          {
             printf("check_date : before ! state %d\n", al->state);
             if (strict)
               return 0;
             else
               {
                  if (al->autoremove)
                    alarm_alarm_del(al);
                  else
                    {
                       printf("state : %d\n", al->state);
                       /*
                         if (al->state == ALARM_STATE_RINGING)
                         alarm_alarm_ring_stop(al, 0);
                       */
                       if (al->state == ALARM_STATE_ON)
                         al->state = ALARM_STATE_OFF;
                    }
               }
          }
        else
          {
             printf("check_date : after (%f < %f) ! state %d\n", al->sched.date_epoch, ecore_time_unix_get(), al->state);
             /*
               if (al->state == ALARM_STATE_RINGING)
               alarm_alarm_ring_stop(al, 0);
             */
          }
        break;

     case ALARM_SCHED_WEEK:
        if ((al->sched.day_monday || 
             al->sched.day_tuesday || 
             al->sched.day_wenesday || 
             al->sched.day_thursday || 
             al->sched.day_friday || 
             al->sched.day_saturday || 
             al->sched.day_sunday) == 0)
          return 0;

        if (al->sched.date_epoch <= ecore_time_unix_get())
          {
             al->sched.date_epoch =
                _epoch_find_next(al->sched.day_monday,
                                 al->sched.day_tuesday,
                                 al->sched.day_wenesday,
                                 al->sched.day_thursday,
                                 al->sched.day_friday,
                                 al->sched.day_saturday,
                                 al->sched.day_sunday,
                                 al->sched.hour,
                                 al->sched.minute);
          }
        break;
     }

   if (alarm_config->config_dialog)
     alarm_config_refresh_alarms_ilist(alarm_config->config_dialog->cfdata);

   return 1;
}

static void
_alarm_snooze(Alarm *al)
{
   E_Dialog *dia;
   Evas_Object *o, *ob;
   Evas *evas;
   char buf[4096];
   int w, h;

   if (al->snooze.dia)
     return;

   dia = e_dialog_new(e_container_current_get(e_manager_current_get()), "Alarm", "_e_modules_alarm_snooze_dialog");
   if (!dia)
     return;
   evas = e_win_evas_get(dia->win);

   snprintf(buf, sizeof(buf), D_("Snooze %s"), al->name);
   e_dialog_title_set(dia, buf);
   
   o = e_widget_list_add(evas, 0, 0);
   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f hours"), 0.0, 24.0, 1.0, 0,
                            NULL, &(al->snooze.hour), 100);
   e_widget_list_object_append(o, ob, 0, 0, 1.0);
   ob = e_widget_slider_add(evas, 1, 0, D_("%1.0f minutes"), 0.0, 60.0, 1.0, 0,
                            NULL, &(al->snooze.minute), 100);
   e_widget_list_object_append(o, ob, 0, 0, 1.0);

   e_widget_size_min_get(o, &w, &h);
   e_dialog_content_set(dia, o, w, h);

   e_dialog_button_add(dia, D_("Ok"), NULL, _alarm_cb_dialog_snooze_ok, al);
   e_dialog_button_add(dia, D_("Close"), NULL, _alarm_cb_dialog_snooze_cancel, al);

   al->snooze.dia = dia;
   e_win_centered_set(dia->win, 1);
   e_dialog_show(dia);
}

static void
_alarm_cb_dialog_snooze_ok(void *data, E_Dialog *dia)
{
   Alarm *al;
   int time;

   al = data;
   time = al->snooze.hour*3600 + al->snooze.minute*60;
   if (!time) return;
   if (al->snooze.etimer) ecore_timer_del(al->snooze.etimer);
   al->state = ALARM_STATE_SNOOZED;
   al->snooze.remember = 1;
   al->snooze.etimer = ecore_timer_add(time, _cb_alarm_snooze_time, al);
   _alarm_dialog_snooze_delete(dia, al);

   e_config_save_queue();
}

static void
_alarm_cb_dialog_snooze_cancel(void *data, E_Dialog *dia)
{
   Alarm *al;

   al = data;
   _alarm_dialog_snooze_delete(dia, al);

   e_config_save_queue();
}

static void
_alarm_dialog_snooze_delete(E_Dialog *dia, Alarm *al)
{
   e_object_del(E_OBJECT(dia));
   al->snooze.dia = NULL;
}

static double
_epoch_find_date(char *date, int hour, int minute)
{
   struct tm ts;
   time_t t;
   int i;

   if (!date)
     return 0;
   if (strlen(date) < 10)
     return 0;

   /* init date struct */
   t = time(NULL);
   localtime_r(&t, &ts);

   /* date format : "YYYY/MM/DD" */
   ts.tm_sec = 0;
   ts.tm_min = minute;
   ts.tm_hour = hour;
   sscanf(date+8, "%2d", &ts.tm_mday);
   sscanf(date+5, "%2d", &ts.tm_mon);
   ts.tm_mon--;   /* mktime works with 0-11 */
   sscanf(date, "%4d", &i);
   ts.tm_year = i - 1900;

   t = mktime(&ts);

   printf("epoch_find_date : %lu (now : %lu)", t, time(NULL));

   return (double)t;
}

static double
_epoch_find_next(int day_monday, int day_tuesday, int day_wenesday, int day_thursday, int day_friday, int day_saturday, int day_sunday, int hour, int minute)
{
   int day_week[7];
   struct tm ts;
   struct tm *ts_today;
   time_t t;
   int i, days_offset;
   double epoch;

   day_week[0] = day_sunday;
   day_week[1] = day_monday;
   day_week[2] = day_tuesday;
   day_week[3] = day_wenesday;
   day_week[4] = day_thursday;
   day_week[5] = day_friday;
   day_week[6] = day_saturday;
   if (!(day_week[0] || day_week[1] || day_week[2] || day_week[3] ||
         day_week[4] || day_week[5] || day_week[6]))
     return 0;
   t = time(NULL);
   ts_today = localtime(&t);

   localtime_r(&t, &ts); /* init the struct */
   ts.tm_sec = 0;
   ts.tm_min = minute;
   ts.tm_hour = hour;
   ts.tm_mday = ts_today->tm_mday;
   ts.tm_mon = ts_today->tm_mon;
   ts.tm_year = ts_today->tm_year;
   epoch = (double)mktime(&ts);

   days_offset = 0;
   for(i=ts_today->tm_wday; i<ts_today->tm_wday+7; i++)
     {
        if(day_week[i%7])
          {
             if ((i%7) != ts_today->tm_wday)
               break;
             else
               {
                  if (ts_today->tm_hour < hour)
                    break;
                  else
                    if ((ts_today->tm_hour == hour) && (ts_today->tm_min < minute))
                      break;
               
               }
          }
        days_offset++;
     }

   epoch += days_offset*3600*24;

   return epoch;
}

static void
_button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;
   
   inst = data;
   ev = event_info;
   if (ev->button == 3)
     {
  E_Menu *m;
  E_Menu_Item *mi;
  int cx, cy, cw, ch;
        int nb_snoozed = 0;

  m = e_menu_new();
  mi = e_menu_item_new(m);
  e_menu_item_label_set(mi, D_("Add an alarm"));
  e_menu_item_callback_set(mi, _menu_cb_alarm_add, NULL);
  if (!alarm_config->theme)
    e_util_menu_item_theme_icon_set(mi, THEME_ICON_ALARM_ON);
  else
    e_menu_item_icon_edje_set(mi, alarm_config->theme, THEME_ICON_ALARM_ON);

  mi = e_menu_item_new(m);
  e_menu_item_separator_set(mi, 1);
  mi = e_menu_item_new(m);
  e_menu_item_label_set(mi, D_("Settings"));
  e_util_menu_item_theme_icon_set(mi, "preferences-system");
  e_menu_item_callback_set(mi, _menu_cb_configure, NULL);

  m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

        /* snooze menu */
        if (alarm_config->alarms_state == ALARM_STATE_RINGING)
          {
             Eina_List *l;

             for (l=alarm_config->alarms; l; l=eina_list_next(l))
               {
                  Alarm *al;
                  al = eina_list_data_get(l);
                  if (al->state == ALARM_STATE_RINGING)
                    {
                       char buf[30];
                       snprintf(buf, sizeof(buf), D_("Snooze %s"), al->name);
                       mi = e_menu_item_new_relative(m, NULL);
                       e_menu_item_label_set(mi, buf);
                       e_menu_item_callback_set(mi, _menu_cb_alarm_snooze, al);
                       if (!alarm_config->theme) e_util_menu_item_theme_icon_set(mi, THEME_ICON_SNOOZE);
                       else e_menu_item_icon_edje_set(mi, alarm_config->theme, THEME_ICON_SNOOZE);
                       if (al->snooze.remember)
                         {
                            snprintf(buf, sizeof(buf), D_("Snooze %.14s of %.2d:%.2d"),
                                     al->name, al->snooze.hour, al->snooze.minute);
                            mi = e_menu_item_new_relative(m, NULL);
                            e_menu_item_label_set(mi, buf);
                            e_menu_item_callback_set(mi, _menu_cb_alarm_snooze, al);
                         }
                       nb_snoozed = 1;
                    }
               }
          }

        if (!nb_snoozed)
          {
             mi = e_menu_item_new_relative(m, NULL);
             e_menu_item_label_set(mi, D_("Snooze (No alarm to delay)"));
             if (!alarm_config->theme) e_util_menu_item_theme_icon_set(mi, THEME_ICON_SNOOZE);
             else e_menu_item_icon_edje_set(mi, alarm_config->theme, THEME_ICON_SNOOZE);
          }

  e_menu_post_deactivate_callback_set(m, _menu_cb_deactivate_post, inst);
  alarm_config->menu = m;
  e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon,
            &cx, &cy, &cw, &ch);
  e_menu_activate_mouse(m,
            e_util_zone_current_get(e_manager_current_get()),
            cx + ev->output.x, cy + ev->output.y, 1, 1,
            E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
  evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
         EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void
_menu_cb_deactivate_post(void *data, E_Menu *m)
{
   if (!alarm_config->menu) return;
   e_object_del(E_OBJECT(alarm_config->menu));
   alarm_config->menu = NULL;
}

static void
_menu_cb_alarm_snooze(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Alarm *al;

   al = data;
   _alarm_snooze(al);
}

static void
_menu_cb_alarm_add(void *data, E_Menu *m, E_Menu_Item *mi)
{
   alarm_config_alarm(NULL);
}

static void
_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi)
{
   if (!alarm_config) return;
   if (alarm_config->config_dialog) return;
   alarm_config_module();
}

static void
_cb_edje_alarm_state_on(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   if (alarm_config->alarms_state == ALARM_STATE_ON) return;

   alarm_config->alarms_state = ALARM_STATE_ON;
   alarm_config->alarms_ring_etimer = ecore_timer_add(ALARMS_CHECK_TIMER,
                                                      _cb_alarms_ring_etimer,
                                                      NULL);
   e_config_save_queue();   
}

static void
_cb_edje_alarm_state_off(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   if (alarm_config->alarms_state == ALARM_STATE_OFF) return;

   alarm_config->alarms_state = ALARM_STATE_OFF;
   ecore_timer_del(alarm_config->alarms_ring_etimer);
   e_config_save_queue();
}

static void
_cb_edje_alarm_ring_stop(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   if (alarm_config->alarms_state != ALARM_STATE_RINGING) return;

   alarm_alarm_ring_stop(NULL, 1);
}

static Eina_Bool
_cb_alarms_ring_etimer(void *data)
{
   Eina_List *l;
   double now;

   if (alarm_config->alarms_state == ALARM_STATE_OFF)
     {
        alarm_config->alarms_ring_etimer = NULL;
        return EINA_FALSE;
     }

   now = ecore_time_unix_get();

   for(l=alarm_config->alarms; l; l=eina_list_next(l))
     {
        Alarm *al;

        al = eina_list_data_get(l);
        if (al->state != ALARM_STATE_ON)
          continue;

        if (al->sched.date_epoch <= now)
          alarm_alarm_ring(al, 0);
     }

   return EINA_TRUE;
}

static Eina_Bool
_cb_alarm_snooze_time(void *data)
{
   Alarm *al;

   al = data;
   al->snooze.etimer = NULL;

   alarm_alarm_ring(al, 0);

   return EINA_FALSE;
}


/*
 * Module functions
 */

EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[4096];
   Eina_List *l;

   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   _alarms_edd = E_CONFIG_DD_NEW("Alarm_Alarm", Alarm);
#undef T
#undef D
#define T Alarm
#define D _alarms_edd
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, state, SHORT);
   E_CONFIG_VAL(D, T, description, STR);
   E_CONFIG_VAL(D, T, autoremove, SHORT);
   E_CONFIG_VAL(D, T, open_popup, SHORT);
   E_CONFIG_VAL(D, T, run_program, SHORT);
   E_CONFIG_VAL(D, T, program, STR);
   E_CONFIG_VAL(D, T, sched.type, SHORT);
   E_CONFIG_VAL(D, T, sched.date_epoch, DOUBLE);
   E_CONFIG_VAL(D, T, sched.day_monday, SHORT);
   E_CONFIG_VAL(D, T, sched.day_tuesday, SHORT);
   E_CONFIG_VAL(D, T, sched.day_wenesday, SHORT);
   E_CONFIG_VAL(D, T, sched.day_thursday, SHORT);
   E_CONFIG_VAL(D, T, sched.day_friday, SHORT);
   E_CONFIG_VAL(D, T, sched.day_saturday, SHORT);
   E_CONFIG_VAL(D, T, sched.day_sunday, SHORT);
   E_CONFIG_VAL(D, T, sched.hour, SHORT);
   E_CONFIG_VAL(D, T, sched.minute, SHORT);
   E_CONFIG_VAL(D, T, snooze.hour, SHORT);
   E_CONFIG_VAL(D, T, snooze.minute, SHORT);
   E_CONFIG_VAL(D, T, snooze.remember, SHORT);
   _conf_edd = E_CONFIG_DD_NEW("Alarm_Config", Config);
#undef T
#undef D
#define T Config
#define D _conf_edd
   E_CONFIG_VAL(D, T, time_format, SHORT);
   E_CONFIG_VAL(D, T, alarms_state, SHORT);
   E_CONFIG_LIST(D, T, alarms, _alarms_edd);
   E_CONFIG_VAL(D, T, alarms_details, SHORT);
   E_CONFIG_VAL(D, T, alarms_autoremove_default, SHORT);
   E_CONFIG_VAL(D, T, alarms_open_popup_default, SHORT);
   E_CONFIG_VAL(D, T, alarms_run_program_default, SHORT);
   E_CONFIG_VAL(D, T, alarms_program_default, STR);
   E_CONFIG_VAL(D, T, config_version, SHORT);
   alarm_config = e_config_domain_load("module.alarm", _conf_edd);
   if (alarm_config)
     {
        if (alarm_config->config_version < CONFIG_VERSION)
          {
             snprintf(buf, sizeof(buf),
                      D_("<hilight>Alarm module : Configuration Upgraded</hilight><br><br>"
                         "Your configuration of alarm module<br>"
                         "has been upgraded<br>"
                         "Your settings and alarms were removed<br>"
                         "Sorry for the inconvenience<br><br>"
                         "(%d -> %d)"), alarm_config->config_version, CONFIG_VERSION);
             e_module_dialog_show(alarm_config->module, D_("Alarm Module version " MODULE_VERSION), buf);
             alarm_config = NULL;
          }
        else
          {
             if (alarm_config->config_version > CONFIG_VERSION)
               {
                  snprintf(buf, sizeof(buf),
                           D_("<hilight>Alarm module : Configuration Downgraded</hilight><br><br>"
                              "Your configuration of Alarm module<br>"
                              "has been downgraded<br>"
                              "Your settings and alarms were removed<br>"
                              "Sorry for the inconvenience<br><br>"
                              "(%d ->%d)"), alarm_config->config_version, CONFIG_VERSION);
                  e_module_dialog_show(alarm_config->module, D_("Alarm Module version " MODULE_VERSION), buf);
                  alarm_config = NULL;
               }
          }
     }

   if (!alarm_config)
     {
  alarm_config = E_NEW(Config, 1);
        alarm_config->time_format = TIME_FORMAT_DEFAULT;
        alarm_config->alarms_state = ALARM_STATE_OFF;
  alarm_config->alarms_details = ALARMS_DETAILS_DEFAULT;
        alarm_config->alarms_autoremove_default = ALARMS_AUTOREMOVE_DEFAULT;
        alarm_config->alarms_open_popup_default = ALARMS_OPEN_POPUP_DEFAULT;
        alarm_config->alarms_run_program_default = ALARMS_RUN_PROGRAM_DEFAULT;
        alarm_config->config_version = CONFIG_VERSION;
     }
   E_CONFIG_LIMIT(alarm_config->time_format, TIME_FORMAT_12, TIME_FORMAT_24);
   E_CONFIG_LIMIT(alarm_config->alarms_details, 0, 1);
   E_CONFIG_LIMIT(alarm_config->alarms_autoremove_default, 0, 1);
   E_CONFIG_LIMIT(alarm_config->alarms_open_popup_default, 0, 1);

   /* set the number of ringing alarms */
   for (l=alarm_config->alarms; l; l=eina_list_next(l))
     {
        Alarm *al;
        al = eina_list_data_get(l);
        if (al->state == ALARM_STATE_RINGING)
          alarm_config->alarms_ringing_nb++;
     }

   /* set alarms state and check dates */
   if (alarm_config->alarms)
     {
        for (l=alarm_config->alarms; l; l=eina_list_next(l))
          {
             Alarm *al;
             al = eina_list_data_get(l);
             _alarm_check_date(al, 0);
          }
        alarm_config->alarms_ring_etimer = ecore_timer_add(ALARMS_CHECK_TIMER,
                                                           _cb_alarms_ring_etimer,
                                                           NULL);
     }

   //FIXME not sure about that, maybe must use edje directly to find the part
   if (!e_theme_category_find(THEME_IN_E))
     {
        char buf[512];

        snprintf(buf, sizeof(buf), "%s/alarm.edj", e_module_dir_get(m));
        alarm_config->theme = strdup(buf);
     }
   
   alarm_config->module = m;

   e_gadcon_provider_register((const E_Gadcon_Client_Class *)&_gadcon_class);

   return alarm_config;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   e_gadcon_provider_unregister((const E_Gadcon_Client_Class *)&_gadcon_class);
   
   if (alarm_config->alarms)
     {
        while (eina_list_count(alarm_config->alarms))
          {
             Alarm *al;
             al = eina_list_data_get(alarm_config->alarms);
             alarm_alarm_del(al);
          }
        eina_list_free(alarm_config->alarms);
     }
   if (alarm_config->alarms_ring_etimer)
     ecore_timer_del(alarm_config->alarms_ring_etimer);
   if (alarm_config->alarms_program_default)
     eina_stringshare_del(alarm_config->alarms_program_default);

   if (alarm_config->theme)
     free(alarm_config->theme);
   if (alarm_config->config_dialog) 
     e_object_del(E_OBJECT(alarm_config->config_dialog));
   if (alarm_config->config_dialog_alarm_new) 
     e_object_del(E_OBJECT(alarm_config->config_dialog_alarm_new));
   if (alarm_config->menu)
     {
  e_menu_post_deactivate_callback_set(alarm_config->menu , NULL, NULL);
  e_object_del(E_OBJECT(alarm_config->menu));
     }

   E_FREE(alarm_config);
   E_CONFIG_DD_FREE(_alarms_edd);
   E_CONFIG_DD_FREE(_conf_edd);

   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   e_config_domain_save("module.alarm", _conf_edd, alarm_config);
   return 1;
}
