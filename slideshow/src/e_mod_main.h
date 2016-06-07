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

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;

struct _Config
{
   E_Module *module;
   E_Config_Dialog *config_dialog;
   E_Menu *menu;
   Eina_List *instances;
   Eina_List *items;
};

struct _Config_Item
{
   const char *id;

   int disable_timer;
   int random_order;
   int all_desks;
   double poll_time;
   const char *dir;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int   e_modapi_shutdown(E_Module *m);
EAPI int   e_modapi_save(E_Module *m);

void _config_slideshow_module(Config_Item *ci);
void _slide_config_updated(Config_Item *ci);
extern Config *slide_config;

#endif
