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

/* Macros used for config file versioning */
#define MOD_CONFIG_FILE_EPOCH 0x0001
#define MOD_CONFIG_FILE_GENERATION 0x008d
#define MOD_CONFIG_FILE_VERSION \
   ((MOD_CONFIG_FILE_EPOCH << 16) | MOD_CONFIG_FILE_GENERATION)

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;

/* Base config struct. Store Item Count, etc
 * 
 * *module (not written to disk) (E Interaction)
 * *cfd (not written to disk) (config dialog)
 * 
 * Store list of your items that you want to keep. (sorting)
 * Can define per-module config properties here.
 * 
 * Version used to know when user config too old */
struct _Config 
{
   E_Module *module;
   E_Config_Dialog *cfd;

   /* The list; their location on screen ? */
   Eina_List *conf_items;

   /* config file version */
   int version;
};

/* This struct used to hold config for individual items from above list */
struct _Config_Item 
{
   /* unique id */
   const char *id;

   /* selected disk */
   const char *disk;
};

/* Setup the E Module Version, Needed to check if module can run. */
EAPI extern E_Module_Api e_modapi;

/* E API Module Interface Declarations
 * 
 * Need to initialize, shutdown, save the module */
EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

/* Function for calling the modules config dialog */
E_Config_Dialog *e_int_config_diskio_module(E_Container *con, Config_Item *ci);

extern Config *diskio_conf;

#endif
