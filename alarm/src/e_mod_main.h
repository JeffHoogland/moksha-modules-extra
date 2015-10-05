#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#ifdef ENABLE_NLS
# include <libintl.h>
# define D_(string) dgettext(PACKAGE, string)
#else
# define bindtextdomain(domain,dir)
# define bind_textdomain_codeset(domain,codeset)
# define D_(string) (string)
#endif


typedef enum _Alarm_State
   {
      ALARM_STATE_OFF,
      ALARM_STATE_ON,
      ALARM_STATE_RINGING,
      ALARM_STATE_SNOOZED
   } Alarm_State;

typedef enum _Alarm_Sched
   {
      ALARM_SCHED_DAY,
      ALARM_SCHED_WEEK
   } Alarm_Sched;

typedef enum _Alarm_Add_Error
   {
      ALARM_ADD_ERROR_UNKNOWN,
      ALARM_ADD_ERROR_NAME,
      ALARM_ADD_ERROR_SCHED_DAY,
      ALARM_ADD_ERROR_SCHED_WEEK,
      ALARM_ADD_ERROR_SCHED_BEFORE
   } Alarm_Add_Error;

typedef enum _Alarm_Run_Program
   {
      ALARM_RUN_PROGRAM_OWN,
      ALARM_RUN_PROGRAM_PARENT,
      ALARM_RUN_PROGRAM_NO
   } Alarm_Run_Program;

typedef enum _Time_Format
   {
      TIME_FORMAT_12,
      TIME_FORMAT_24
   } Time_Format;

#define CONFIG_VERSION 7

#define VALUE_PARENT 9999

#define TIME_FORMAT_DEFAULT TIME_FORMAT_24

#define ALARMS_DETAILS_DEFAULT 1
#define ALARMS_AUTOREMOVE_DEFAULT 0
#define ALARMS_OPEN_POPUP_DEFAULT 1
#define ALARMS_RUN_PROGRAM_DEFAULT 0
#define ALARMS_CHECK_TIMER (60)

#define ALARM_SCHED_DEFAULT ALARM_SCHED_DAY
#define ALARM_SNOOZE_HOUR_DEFAULT 0
#define ALARM_SNOOZE_MINUTE_DEFAULT 15

#define THEME_IN_E "base/theme/modules/alarm"
#define THEME_MAIN "modules/alarm/main"
#define THEME_ICON_SNOOZE "modules/alarm/icon/alarm/snooze"
#define THEME_ICON_ALARM_ON "modules/alarm/icon/alarm/on"
#define THEME_ICON_ALARM_OFF "modules/alarm/icon/alarm/off"

#define EDJE_SIG_RECV_ALARM_STATE_ON "state,on", ""
#define EDJE_SIG_RECV_ALARM_STATE_OFF "state,off", ""
#define EDJE_SIG_RECV_ALARM_RING_STOP "ring,stop", ""

#define EDJE_SIG_SEND_ALARM_STATE_ON "state,on", "alarm"
#define EDJE_SIG_SEND_ALARM_STATE_OFF "state,off", "alarm"
#define EDJE_SIG_SEND_ALARM_RING_START "ring,start", "alarm"
#define EDJE_SIG_SEND_ALARM_DETAILS_ON "details,on", "alarm"
#define EDJE_SIG_SEND_ALARM_DETAILS_OFF "details,off", "alarm"

typedef struct _Config Config;
typedef struct _Alarm Alarm;

struct _Alarm
{
   const char  *name;
   Alarm_State state;
   E_Config_Dialog *config_dialog;

   const char *description;
   int autoremove;
   int open_popup;
   int run_program;
   const char *program;
  
   struct
   {
      Alarm_Sched type;
      double date_epoch;
      int hour;
      int minute;
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
      int hour;
      int minute;
      int remember;
      E_Dialog *dia;
      Ecore_Timer *etimer;
   } snooze;
};

struct _Config
{
   Time_Format      time_format;

   Alarm_State      alarms_state;
   Eina_List       *alarms;
   Ecore_Timer     *alarms_ring_etimer;
   int              alarms_details;
   int              alarms_autoremove_default;
   int              alarms_open_popup_default;
   int              alarms_run_program_default;
   const char      *alarms_program_default;

   int   config_version;
   char *theme;
   int alarms_ringing_nb;
   E_Module        *module;
   E_Config_Dialog *config_dialog;
   E_Config_Dialog *config_dialog_alarm_new;
   Eina_List       *instances;
   E_Menu          *menu;
};

EAPI extern E_Module_Api e_modapi;
extern Config *alarm_config;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);

Alarm *alarm_alarm_add(int state, char *name, int type, char *date, int day_monday, int day_tuesday, int day_wenesday, int day_thursday, int day_friday, int day_saturday, int day_sunday, int hour, int minute, int autoremove, char *description, int open_popup, int run_program, char *program, int *error);
void   alarm_alarm_del(Alarm *al);
Alarm *alarm_alarm_duplicate(Alarm *al);
int    alarm_alarm_ring(Alarm *al, int test);
void   alarm_alarm_ring_stop(Alarm *al, int check);
void   alarm_edje_signal_emit(const char *source, const char *message);
void   alarm_edje_text_set(char *part, char *text);
void   alarm_edje_refresh_details(void);
void   alarm_details_change(void);

void   alarm_config_module(void);
void   alarm_config_refresh_alarms_ilist(E_Config_Dialog_Data *cfdata);

void   alarm_config_alarm(Alarm *al);

#endif
