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

#define _RET_NONE_VALUE 0
#define _RET_BOTTOM_VALUE 1
#define _RET_TOP_VALUE 2
#define _RET_LEFT_VALUE 3
#define _RET_RIGHT_VALUE 4

#define ID_WALKER 1
#define ID_FALLER 2
#define ID_CLIMBER 3
#define ID_FLOATER 4
#define ID_SPLATTER 5
#define ID_FLYER 6
#define ID_BOMBER 7
#define ID_ANGEL 8

typedef struct _Config Config;
typedef struct _Population Population;
typedef struct _Penguin Penguin;
typedef struct _Action Action;
typedef struct _Custom_Action Custom_Action;

struct _Config
{
   double zoom;
   int penguins_count;
   const char *theme;
   int alpha;
};

struct _Population
{
   E_Module *module;
   Eina_List *cons;
   Evas *canvas;
   Ecore_Animator *animator;
   Eina_List *penguins;
   Eina_Hash *actions;
   Eina_List *customs;
   int custom_num;
   Eina_List *themes;

   E_Config_DD *conf_edd;
   Config *conf;
   Evas_Coord width, height;
   E_Config_Dialog *config_dialog;
};

struct _Action
{
   char *name;
   int id;
   Evas_Coord w,h;
   int speed;
};

struct _Custom_Action
{
   char *name;
   Evas_Coord w,h;
   int h_speed;
   int v_speed;
   int r_min;
   int r_max;
   char *left_program_name;
   char *right_program_name;
};

struct _Penguin
{
   Evas_Object *obj;
   int reverse;
   double x, y;
   int faller_h;
   int r_count;
   Action *action;
   Custom_Action *custom;
   Population *pop;
};

extern E_Module *penguins_mod;

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

void _penguins_cb_config_updated(void *data);

#endif
