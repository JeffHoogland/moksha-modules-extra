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

#define DEGREES_F 0
#define DEGREES_C 1

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;
typedef struct _Popup Popup;

struct _Config
{
   E_Module *module;
   E_Config_Dialog *config_dialog;
   Eina_List *instances;
   Eina_List *items;
   E_Menu *menu;
};

struct _Config_Item
{
   const char *id;

   double poll_time;
   int degrees;
   const char *host, *code;
   int show_text;
   int popup_on_hover;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int   e_modapi_shutdown(E_Module *m);
EAPI int   e_modapi_save(E_Module *m);

void _config_forecasts_module(Config_Item *ci);
void _forecasts_config_updated(Config_Item *ci);

extern Config *forecasts_config;

#endif
