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

#define ENABLE_DEBUG 0
#define DEBUG(f, ...) if (ENABLE_DEBUG) \
    printf("\x1b[31m[slideshow]\x1b[0m "f "\n", __VA_ARGS__)

/* Increment for Major Changes */
#define MOD_CONFIG_FILE_EPOCH      1
/* Increment for Minor Changes (ie: user doesn't need a new config) */
#define MOD_CONFIG_FILE_GENERATION 0
#define MOD_CONFIG_FILE_VERSION    ((MOD_CONFIG_FILE_EPOCH * 1000000) + MOD_CONFIG_FILE_GENERATION)

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;

struct _Config
{
   E_Module *module;
   E_Config_Dialog *config_dialog;
   Eina_List *instances;
   Eina_List *items;
   int version;
};

struct _Config_Item
{
   const char *id;

   int disable_timer;
   int disable_sched;
   int random_order;
   int all_desks;
   double poll_time, hours, minutes;
   const char *dir, *file_day, *file_night;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int   e_modapi_shutdown(E_Module *m);
EAPI int   e_modapi_save(E_Module *m);

void _config_slideshow_module(Config_Item *ci);
void _slide_config_updated(Config_Item *ci);
extern Config *slide_config;

#endif
