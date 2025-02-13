#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <e.h>
#include <Etrophy.h>
#include <E_Notify.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define D_(str) dgettext(PACKAGE, str)
# define DP_(str, str_p, n) dngettext(PACKAGE, str, str_p, n)
#else
# define bindtextdomain(domain,dir)
# define bind_textdomain_codeset(domain,codeset)
# define D_(str) (str)
# define DP_(str, str_p, n) (str_p)
#endif

#define N_(str) (str)

#define MOD_CONFIG_FILE_EPOCH 0
#define MOD_CONFIG_FILE_GENERATION 1
#define MOD_CONFIG_FILE_VERSION ((MOD_CONFIG_FILE_EPOCH * 1000000) + MOD_CONFIG_FILE_GENERATION)

#define XSTR(X) #X
#define STRINGIFY(X) XSTR(X)

// need to update these whenever default profile updates I guess
#define DEFAULT_BINDING_COUNT_ACPI 11
#define DEFAULT_BINDING_COUNT_KEY 65
#define DEFAULT_BINDING_COUNT_MOUSE 6
#define DEFAULT_BINDING_COUNT_SIGNAL 28

#define ECH(X) ECHIEVEMENT_ID_##X
/* See instructions in echievements.x for adding new ones */
typedef enum
{
   ECH(SHELF_POSITIONS),
   ECH(NOTHING_ELSE_MATTERS),
   ECH(FEAR_OF_THE_DARK),
   ECH(CAVE_DWELLER),
   ECH(WINDOW_ENTHUSIAST),
   ECH(WINDOW_LOVER),
   ECH(WINDOW_STALKER),
   ECH(WINDOW_MOVER),
   ECH(WINDOW_OCD),
   ECH(MOUSE_RUNNER),
   ECH(MOUSE_MARATHONER),
   ECH(MOUSE_HERO),
   ECH(MIGHTY_MOUSE),
   ECH(WINDOW_HAULER),
   ECH(WINDOW_SLINGER),
   ECH(WINDOW_SHERPA),
   ECH(PHYSICIST),
   ECH(QUICKDRAW),
   ECH(OPAQUE),
   ECH(KEYBOARD_USER),
   ECH(KEYBOARD_TAPPER),
   ECH(KEYBOARD_ABUSER),
   ECH(KEYBOARD_NINJA),
   ECH(CLICKER),
   ECH(SUPER_CLICKER),
   ECH(SUPER_DUPER_CLICKER),
   ECH(CLICK_MANIAC),
   ECH(SLEEPER),
   ECH(HALTER),
   ECH(EDGY),
   ECH(EDGIER),
   ECH(EDGIEST),
   ECH(EDGAR),
   ECH(WHEELY),
   ECH(ROLLIN),
   ECH(ROLY_POLY),
   ECH(SIGNALLER),
   ECH(SIGNAL_CONTROLLER),
   ECH(SIGNAL_MASTER),
   ECH(SIGURD),
   ECH(NOT_SO_INCOGNITO),
   ECH(DUALIST),
   ECH(THRICE),
   ECH(GOING_HD),
   ECH(REAL_ESTATE_MOGUL),
   ECH(MAXIMUM_DEFINITION),
   ECH(TERMINOLOGIST),
   ECH(LIFE_ON_THE_EDGE), //FIXME: this will be granted immediately until module goes into core :/
   ECH(BILINGUAL),
   ECH(POLYGLOT),
   ECH(GADGETEER),
   ECH(INSPECTOR_GADGET),
   ECH(GADGET_HACKWRENCH),
   ECH(SECURITY_CONSCIOUS),
   ECH(SECURITY_NUT),
   ECH(CHIEF_OF_SECURITY),
   ECH(TILED),
   ECH(PERSISTENT), //FIXME: timer does not persist across restarts :/
   ECH(NEVER_GONNA_GIVE_YOU_UP),
   ECH(LAST)
} Echievement_Id;

typedef struct Echievement
{
   EINA_INLIST;
   Echievement_Id id;
   Etrophy_Trophy *trophy;
   Eina_List *handlers;
   Ecore_Cb mouse_hook;
   struct
   {
      Evas_Object *icon;
      Evas_Object *label;
   } dialog;
   void *data; // echievement data
   Eina_Bool bh_handlers : 1; //handlers are border hooks
} Echievement;

typedef void (*Echievement_Init_Cb)(Echievement*);

typedef struct Mod
{
   E_Config_Dialog *cfd;
   Evas_Object *cfd_list[2]; //list of earned trophies, list of all trophies
   Evas_Object *label; //label of total points
   Evas_Object_Event_Cb obj_del_cb; //for deleting list object callbacks

   E_Module *module;
   Eina_Hash *trophies; // for fast lookups
   Echievement *trophies_list; // head for iterating
   Eina_Inlist *itr; // current iterator position if needed
   struct
   {
      int x, y;
      int dx, dy;
      Eina_List *hooks;
   } mouse;
   struct
   {
      Eina_List *timers;
   } desklock;
   Eina_Bool module_init_end : 1;
} Mod;

typedef struct Config
{
   unsigned int config_version;
   Etrophy_Gamescore *gs;
} Config;

extern Mod *mod;
extern Config *ech_config;

E_Config_Dialog *e_int_config_echievements(E_Container *con, const char *params);
EINTERN void ech_cfg_ech_update(Echievement *ec);
EINTERN void ech_cfg_ech_add(Echievement *ec);

void ech_init(void);
void ech_shutdown(void);

unsigned int ech_bindings_check_keys(void);
unsigned int ech_bindings_check_mouse(void);
unsigned int ech_bindings_check_wheel(void);
unsigned int ech_bindings_check_edge(void);
unsigned int ech_bindings_check_acpi(void);
unsigned int ech_bindings_check_signal(void);

EINTERN Eina_List *ech_language_enumerate(void);
#endif
