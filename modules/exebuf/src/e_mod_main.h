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
/* You can increment the EPOCH value if the old configuration is not
 * compatible anymore, it creates an entire new one.
 * You need to increment GENERATION when you add new values to the
 * configuration file but is not needed to delete the existing conf  */
#define MOD_CONFIG_FILE_EPOCH 0x0001
#define MOD_CONFIG_FILE_GENERATION 0x008d
#define MOD_CONFIG_FILE_VERSION \
   ((MOD_CONFIG_FILE_EPOCH << 16) | MOD_CONFIG_FILE_GENERATION)

/* More mac/def; Define your own. What do you need ? */
#define CONN_DEVICE_ETHERNET 0

/* We create a structure config for our module, and also a config structure
 * for every item element (you can have multiple gadgets for the same module) */

typedef struct _Config Config;

struct _Config 
{
  /* config file version */
  int version;

  int         max_exe_list; // GUI
  int         max_eap_list; // GUI
  int         max_hist_list; // GUI
  int         scroll_animate; // GUI
  double      scroll_speed; // GUI
  double      pos_align_x; // GUI
  double      pos_align_y; // GUI
  double      pos_size_w; // GUI
  double      pos_size_h; // GUI
  int         pos_min_w; // GUI
  int         pos_min_h; // GUI
  int         pos_max_w; // GUI
  int         pos_max_h; // GUI
  const char *term_cmd; // GUI
  
  /* if you open a config dialog, store a reference to it in a pointer like
   * this one, so if the user opens a second time the dialog, you know it's
   * already open. Also needed for destroy the dialog when we are exiting */
  E_Config_Dialog *cfd;

  /* Store a reference to the module instance provided by enlightenment in
   * e_modapi_init, in case you need to access it. (not written to disk) */
  E_Module *module;
};

/* Setup the E Module Version, Needed to check if module can run. */
/* The version is stored at compilation time in the module, and is checked
 * by E in order to know if the module is compatible with the actual version */
EAPI extern E_Module_Api e_modapi;

/* E API Module Interface Declarations
 *
 * e_modapi_init:     it is called when e17 initialize the module, note that
 *                    a module can be loaded but not initialized (running)
 *                    Note that this is not the same as _gc_init, that is called
 *                    when the module appears on his container
 * e_modapi_shutdown: it is called when e17 is closing, so calling the modules
 *                    to finish
 * e_modapi_save:     this is called when e17 or by another reason is requeested
 *                    to save the configuration file                      */
EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

int e_exebuf_init(void);
int e_exebuf_shutdown(void);
int  e_exebuf_show(E_Zone *zone);
void e_exebuf_hide(void);

/* Function for calling the module's Configuration Dialog */
E_Config_Dialog *exebuf_config_dialog(E_Container *con, const char *params);

extern Config *exebuf_conf;

#endif
