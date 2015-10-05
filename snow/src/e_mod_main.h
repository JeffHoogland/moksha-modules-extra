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
typedef struct _Snow Snow;
typedef struct _Snow_Flake Snow_Flake;

struct _Config
{
   int tree_count;
   int flake_count;
   int show_trees;
};

struct _Snow
{
   E_Module *module;
   Eina_List *cons;
   Evas *canvas;
   Ecore_Animator *animator;
   Eina_List *trees;
   Eina_List *flakes;

   E_Config_DD *conf_edd;
   Config *conf;
   Evas_Coord width, height;
   E_Config_Dialog *config_dialog;
};

struct _Snow_Flake
{
   Evas_Object *flake;
   double start_time;
   int speed;
};

extern E_Module *snow_module;

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

void _snow_cb_config_updated(void *data);
#endif
