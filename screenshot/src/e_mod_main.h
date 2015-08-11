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

#define MOD_CONFIG_FILE_EPOCH 0x0001
#define MOD_CONFIG_FILE_GENERATION 0x008d
#define MOD_CONFIG_FILE_VERSION \
   ((MOD_CONFIG_FILE_EPOCH << 16) | MOD_CONFIG_FILE_GENERATION)

typedef struct _Config Config;

struct _Config 
{
   int version;
   int quality, thumb_size, mode;
   double delay;
   unsigned char prompt, use_app, use_bell;
   unsigned char use_thumb;
   const char *location, *filename, *app, *mod_dir;
   E_Config_Dialog *cfd;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

E_Config_Dialog *e_int_config_screenshot_module(E_Container *con, const char *params);

extern Config *ss_cfg;
extern E_Module *ss_mod;

#endif
