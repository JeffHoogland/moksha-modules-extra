#ifdef E_MOD_PHOTO_TYPEDEFS

typedef struct _Photo Photo;

#else

#ifndef E_MOD_MAIN_H_INCLUDED
#define E_MOD_MAIN_H_INCLUDED

#ifdef ENABLE_NLS
# include <libintl.h>
# define D_(string) dgettext(PACKAGE, string)
#else
# define bindtextdomain(domain,dir)
# define bind_textdomain_codeset(domain,codeset)
# define D_(string) (string)
#endif

#define PHOTO_THEME_VERSION "1"
#define PHOTO_THEME_IN_E "base/theme/modules/photo"
#define PHOTO_THEME_ITEM "modules/photo/item"
#define PHOTO_THEME_POPI "modules/photo/popi"
#define PHOTO_THEME_POPW "modules/photo/popw"
#define PHOTO_THEME_ICON_INFOS "modules/photo/icon/infos"
#define PHOTO_THEME_ICON_NEXT "modules/photo/icon/next"
#define PHOTO_THEME_ICON_PREVIOUS "modules/photo/icon/previous"
#define PHOTO_THEME_ICON_PAUSE "modules/photo/icon/pause"
#define PHOTO_THEME_ICON_RESUME "modules/photo/icon/resume"
#define PHOTO_THEME_ICON_SETBG "modules/photo/icon/setbg"
#define PHOTO_THEME_ICON_VIEWER "modules/photo/icon/viewer"
#define PHOTO_THEME_ICON_DIR_NOT_LOADED "modules/photo/icon/dir/not_loaded"
#define PHOTO_THEME_ICON_DIR_LOADING "modules/photo/icon/dir/loading"
#define PHOTO_THEME_ICON_DIR_LOADED "modules/photo/icon/dir/loaded"

struct _Photo
{
   E_Module *module;

   Photo_Config *config;
   E_Config_Dialog *config_dialog;
   E_Config_Dialog *config_dialog_adddir;

   Eina_List *items;
   Eina_List *setbg_topurge;

   char *theme;

  Evas *e_evas;
  char *display;
  int canvas_w, canvas_h;
};

Photo *photo;

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int   e_modapi_shutdown(E_Module *m);
EAPI int   e_modapi_save(E_Module *m);

#endif
#endif

