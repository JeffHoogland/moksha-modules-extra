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

#define RESOLUTION_MINUTE 0
#define RESOLUTION_SECOND 1

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;

struct _Config
{
   const char *mod_dir;
   E_Config_Dialog *config_dialog;
   E_Menu *menu;
   Eina_List *instances, *items;
};

struct _Config_Item
{
   const char *id;
   int resolution;
   double poll_time;
   int show_time, show_date, show_tip;
   const char *time_format;
   const char *date_format;
   const char *tip_format;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

void _config_tclock_module(Config_Item *ci);
void _tclock_config_updated(Config_Item *ci);

extern Config *tclock_config;

#endif
