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

typedef enum _Flame_Palette_Type Flame_Palette_Type;

typedef struct _Config Config;
typedef struct _Flame Flame;
typedef struct _Flame_Face Flame_Face;

enum _Flame_Palette_Type
{
   DEFAULT_NONE,
   GOLD_PALETTE,
   FIRE_PALETTE,
   PLASMA_PALETTE,
   MATRIX_PALETTE,
   ICE_PALETTE,
   WHITE_PALETTE,
   CUSTOM_PALETTE
};

struct _Config
{
   Evas_Coord height;
   int hspread, vspread;
   int variance;
   int vartrend;
   int residual;
   int r, g, b;
   Flame_Palette_Type palette_type;
};

struct _Flame
{
   E_Menu *config_menu;
   Flame_Face *face;
   E_Module *module;
   
   E_Config_DD *conf_edd;
   Config *conf;
   E_Config_Dialog *config_dialog;
};

struct _Flame_Face
{
   Flame *flame;
   E_Container *con;
   Evas *evas;

   Evas_Object *flame_object;
   Ecore_Animator *anim;

   Evas_Coord xx, yy, ww;

   /* palette */
   unsigned int *palette;
   unsigned int *im;
   int ims;

   /* the flame arrays */
   int ws;
   unsigned int *f_array1, *f_array2;

   Ecore_Event_Handler *ev_handler_container_resize;
};

extern E_Module *flame_module;

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

void _flame_cb_config_updated(void *data);

#endif /* E_MOD_MAIN_H */
