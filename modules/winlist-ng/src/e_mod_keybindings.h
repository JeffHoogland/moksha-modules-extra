#ifdef E_TYPEDEFS
#else
#ifndef E_MOD_KEYBIND_H
#define E_MOD_KEYBIND_H

//#include "e_mod_main.h"

#define NGW_WINLIST_NEXT_ACTION   "ngw_winlist_next"
#define NGW_WINLIST_PREV_ACTION   "ngw_winlist_prev"

int ngw_register_module_actions();
int ngw_unregister_module_actions();

int ngw_register_module_keybindings();
int ngw_unregister_module_keybindings();

#endif
#endif
