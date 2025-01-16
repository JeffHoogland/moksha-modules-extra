#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include "config.h"
#ifdef HAVE_GETTEXT
# include <libintl.h>
# define D_(string) dgettext(LOCALEDOMAIN, string)
#else
# define bindtextdomain(domain,dir)
# define bind_textdomain_codeset(domain,codeset)
# define D_(string) (string)
#endif
#define N_(string) (string)
#define RESOLUTION_MINUTE 0
#define RESOLUTION_SECOND 1

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;

struct _Config
{
   const char *mod_dir;
   E_Config_Dialog *config_dialog;
   Eina_List *instances, *items;
};

struct _Config_Item
{
   const char *id;
   double font_size_up, font_size_down, color_r, color_g, color_b, color_alpha;
   int show_time, show_date, show_tip;
   const char *time_format;
   const char *date_format;
   const char *tip_format;
   const char *time_offset;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

void _config_tclock_module(Config_Item *ci);
void _tclock_config_updated(Config_Item *ci);

extern Config *tclock_config;

#endif
