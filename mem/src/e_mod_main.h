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
   double poll_time;
   int show_percent;
   int always_text;
   int real_ignore_buffers;
   int real_ignore_cached;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int   e_modapi_shutdown(E_Module *m);
EAPI int   e_modapi_save(E_Module *m);

void _mem_config_updated(Config_Item *ci);
void _config_mem_module(Config_Item *ci);
void _mem_get_values(Config_Item *ci, int *real, int *swap, int *total_real, int *total_swap);
extern Config *mem_config;

#endif
