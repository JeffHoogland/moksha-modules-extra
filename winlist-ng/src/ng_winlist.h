#ifndef NGW_WINLIST_H
#define NGW_WINLIST_H

int ngw_winlist_init(void);
int ngw_winlist_shutdown(void);

int  ngw_winlist_show(E_Zone *zone);
void ngw_winlist_hide(void);
int  ngw_winlist_active_get(void);
void .ingw_winlist_next(void);
void ngw_winlist_prev(void);
void ngw_winlist_modifiers_set(int mod);

#endif
