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

E_Config_Dialog *wp_conf_show(E_Container *con, const char *params);
void wp_conf_hide(void);

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_Wallpaper2 Wallpaper2 (Alternative Selector)
 *
 * More graphically appealing wallpaper selector. It is targeted at
 * mobile devices.
 *
 * @note under development and not recommended for use yet.
 *
 * @see Module_Conf_Theme
 * @}
 */
#endif
