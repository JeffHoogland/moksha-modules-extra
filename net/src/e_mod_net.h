#ifndef E_MOD_NET_H
#define E_MOD_NET_H

Eina_Bool _net_cb_poll(void *data);
void _net_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event);
void _net_cb_mouse_in(void *data, Evas_Object *obj, const char *emission, const char *source);
void _net_cb_mouse_out(void *data, Evas_Object *obj, const char *emission, const char *source);

#endif
