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

typedef struct _Instance Instance;
typedef unsigned long bytes_t;

struct _Instance 
{
   E_Gadcon_Client *gcc;
   Evas_Object *o_net, *pop_bg;
   Ecore_Timer *timer;
   E_Gadcon_Popup *popup;

   short popup_locked;
   bytes_t in, out;

   Config_Item *ci;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

#endif
