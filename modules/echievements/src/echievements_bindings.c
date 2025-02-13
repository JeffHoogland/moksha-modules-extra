#include "e_mod_main.h"

static Eina_Bool
_ech_streq(const char *a, const char *b)
{
   return e_util_both_str_empty(a, b) || (!e_util_strcmp(a, b));
}

EINTERN unsigned int
ech_bindings_check_keys(void)
{
   E_Config_Binding_Key *bi;
   unsigned int ret = 0;
   Eina_List *l;

   EINA_LIST_FOREACH(e_config->key_bindings, l, bi)
     {
#define CHK_KEYBIND_DFLT(_context, _key, _modifiers, _anymod, _action, _params) \
  if (                                                                          \
    (bi->context == _context) &&                                                \
    (bi->modifiers == (_modifiers)) &&                                          \
    (bi->any_mod == _anymod) &&                                                 \
    _ech_streq(bi->key, _key) &&                                                \
    _ech_streq(bi->action, _action) &&                                          \
    _ech_streq(bi->params, _params)                                             \
    )                                                                           \
    continue
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Left",
                         E_BINDING_MODIFIER_SHIFT | E_BINDING_MODIFIER_ALT, 0,
                         "desk_flip_by", "-1 0");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Right",
                         E_BINDING_MODIFIER_SHIFT | E_BINDING_MODIFIER_ALT, 0,
                         "desk_flip_by", "1 0");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Up",
                         E_BINDING_MODIFIER_SHIFT | E_BINDING_MODIFIER_ALT, 0,
                         "desk_flip_by", "0 -1");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Down",
                         E_BINDING_MODIFIER_SHIFT | E_BINDING_MODIFIER_ALT, 0,
                         "desk_flip_by", "0 1");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Up",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_raise", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Down",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_lower", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "x",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_close", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "k",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_kill", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "w",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_menu", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "s",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_sticky_toggle", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "f",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_fullscreen_toggle", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "i",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_iconic_toggle", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "f",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_maximized_toggle", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F10",
                         E_BINDING_MODIFIER_SHIFT, 0,
                         "window_maximized_toggle", "default vertical");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F10",
                         E_BINDING_MODIFIER_CTRL, 0,
                         "window_maximized_toggle", "default horizontal");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Left",
                         E_BINDING_MODIFIER_WIN, 0,
                         "window_maximized_toggle", "default left");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Right",
                         E_BINDING_MODIFIER_WIN, 0,
                         "window_maximized_toggle", "default right");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "r",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "window_shaded_toggle", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Left",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_by", "-1");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Right",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_by", "1");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F1",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "0");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F2",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "1");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F3",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "2");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F4",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "3");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F5",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "4");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F6",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "5");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F7",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "6");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F8",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "7");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F9",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "8");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F10",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "9");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F11",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "10");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F12",
                         E_BINDING_MODIFIER_ALT, 0,
                         "desk_linear_flip_to", "11");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "m",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "menu_show", "main");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "a",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "menu_show", "favorites");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Menu",
                         0, 0,
                         "menu_show", "main");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Menu",
                         E_BINDING_MODIFIER_CTRL, 0,
                         "menu_show", "clients");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Menu",
                         E_BINDING_MODIFIER_ALT, 0,
                         "menu_show", "favorites");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Insert",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "exec", "terminology");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Tab",
                         E_BINDING_MODIFIER_ALT, 0,
                         "winlist", "next");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Tab",
                         E_BINDING_MODIFIER_SHIFT | E_BINDING_MODIFIER_ALT, 0,
                         "winlist", "prev");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "End",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "restart", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Delete",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "syscon", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Escape",
                         E_BINDING_MODIFIER_ALT, 0,
                         "everything", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "l",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "desk_lock", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "d",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT, 0,
                         "desk_deskshow_toggle", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F1",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_SHIFT, 0,
                         "screen_send_to", "0");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F2",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_SHIFT, 0,
                         "screen_send_to", "1");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F3",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_SHIFT, 0,
                         "screen_send_to", "2");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "F4",
                         E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_SHIFT, 0,
                         "screen_send_to", "3");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86AudioLowerVolume",
                         0, 0,
                         "volume_decrease", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86AudioRaiseVolume",
                         0, 0,
                         "volume_increase", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86AudioMute",
                         0, 0,
                         "volume_mute", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Print",
                         0, 0,
                         "shot", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86Standby",
                         0, 0,
                         "suspend", "now");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86Start",
                         0, 0,
                         "menu_show", "all");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86PowerDown",
                         0, 0,
                         "hibernate", "now");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86PowerOff",
                         0, 0,
                         "halt", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86Sleep",
                         0, 0,
                         "suspend", "now");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86Suspend",
                         0, 0,
                         "suspend", "now");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86Hibernate",
                         0, 0,
                         "hibernate", "now");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "Execute",
                         0, 0,
                         "everything", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86MonBrightnessUp",
                         0, 0,
                         "backlight_adjust", "0.1");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86MonBrightnessDown",
                         0, 0,
                         "backlight_adjust", "-0.1");
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86LightBulb",
                         0, 0,
                         "backlight", NULL);
        CHK_KEYBIND_DFLT(E_BINDING_CONTEXT_ANY, "XF86BrightnessAdjust",
                         0, 0,
                         "backlight", NULL);

        ret++;
     }
   return ret;
}

EINTERN unsigned int
ech_bindings_check_mouse(void)
{
   E_Config_Binding_Mouse *bi;
   unsigned int ret = 0;
   Eina_List *l;

   EINA_LIST_FOREACH(e_config->mouse_bindings, l, bi)
     {
#define CHK_MOUSEBIND_DFLT(_context, _button, _modifiers, _anymod, _action, _params) \
  if (                                                                               \
    (bi->context == _context) &&                                                     \
    (bi->button == _button) &&                                                       \
    (bi->modifiers == _modifiers) &&                                                 \
    (bi->any_mod == _anymod) &&                                                      \
    _ech_streq(bi->action, _action) &&                                               \
    _ech_streq(bi->params, _params)                                                  \
    )                                                                                \
    continue

        CHK_MOUSEBIND_DFLT(E_BINDING_CONTEXT_WINDOW, 1, E_BINDING_MODIFIER_ALT, 0, "window_move", NULL);
        CHK_MOUSEBIND_DFLT(E_BINDING_CONTEXT_WINDOW, 2, E_BINDING_MODIFIER_ALT, 0, "window_resize", NULL);
        CHK_MOUSEBIND_DFLT(E_BINDING_CONTEXT_WINDOW, 3, E_BINDING_MODIFIER_ALT, 0, "window_menu", NULL);
        CHK_MOUSEBIND_DFLT(E_BINDING_CONTEXT_ZONE, 1, 0, 0, "menu_show", "main");
        CHK_MOUSEBIND_DFLT(E_BINDING_CONTEXT_ZONE, 2, 0, 0, "menu_show", "clients");
        CHK_MOUSEBIND_DFLT(E_BINDING_CONTEXT_ZONE, 3, 0, 0, "menu_show", "favorites");
        ret++;
     }
   return ret;
}

EINTERN unsigned int
ech_bindings_check_wheel(void)
{
   E_Config_Binding_Wheel *bi;
   unsigned int ret = 0;
   Eina_List *l;

   EINA_LIST_FOREACH(e_config->wheel_bindings, l, bi)
     {
#define CHK_WHEELBIND_DFLT(_context, _direction, _z, _modifiers, _anymod, _action, _params) \
  if (                                                                                      \
    (bi->context == _context) &&                                                            \
    (bi->direction == _direction) &&                                                        \
    (bi->z == _z) &&                                                                        \
    (bi->modifiers == _modifiers) &&                                                        \
    (bi->any_mod == _anymod) &&                                                             \
    _ech_streq(bi->action, _action) &&                                                      \
    _ech_streq(bi->params, _params)                                                         \
    )                                                                                       \
    continue
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_CONTAINER, 0, -1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "-1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_CONTAINER, 1, -1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "-1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_CONTAINER, 0, 1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_CONTAINER, 1, 1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_WINDOW, 0, -1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "-1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_WINDOW, 1, -1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "-1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_WINDOW, 0, 1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_WINDOW, 1, 1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_POPUP, 0, -1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "-1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_POPUP, 1, -1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "-1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_POPUP, 0, 1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "1");
        CHK_WHEELBIND_DFLT(E_BINDING_CONTEXT_POPUP, 1, 1, E_BINDING_MODIFIER_ALT, 0,
                           "desk_linear_flip_by", "1");
        ret++;
     }
   return ret;
}

EINTERN unsigned int
ech_bindings_check_edge(void)
{
   E_Config_Binding_Edge *bi;
   unsigned int ret = 0;
   Eina_List *l;

   EINA_LIST_FOREACH(e_config->edge_bindings, l, bi)
     {
#define CHK_EDGEBIND_DFLT(_context, _edge, _modifiers, _anymod, _action, _params, _delay) \
  if (                                                                                    \
    (bi->context == _context) &&                                                          \
    (bi->edge == _edge) &&                                                                \
    (bi->modifiers == _modifiers) &&                                                      \
    (bi->any_mod == _anymod) &&                                                           \
    (fabs(bi->delay - _delay) <= DBL_EPSILON) &&                                          \
    _ech_streq(bi->action, _action) &&                                                    \
    _ech_streq(bi->params, _params)                                                       \
    )                                                                                     \
    continue

        CHK_EDGEBIND_DFLT(E_BINDING_CONTEXT_ZONE, E_ZONE_EDGE_LEFT,
                          0, 0, "desk_flip_in_direction", NULL, 0.3);
        CHK_EDGEBIND_DFLT(E_BINDING_CONTEXT_ZONE, E_ZONE_EDGE_RIGHT,
                          0, 0, "desk_flip_in_direction", NULL, 0.3);
        CHK_EDGEBIND_DFLT(E_BINDING_CONTEXT_ZONE, E_ZONE_EDGE_TOP,
                          0, 0, "desk_flip_in_direction", NULL, 0.3);
        CHK_EDGEBIND_DFLT(E_BINDING_CONTEXT_ZONE, E_ZONE_EDGE_BOTTOM,
                          0, 0, "desk_flip_in_direction", NULL, 0.3);
        ret++;
     }
   return ret;
}

EINTERN unsigned int
ech_bindings_check_acpi(void)
{
   E_Config_Binding_Acpi *bi;
   unsigned int ret = 0;
   Eina_List *l;

   EINA_LIST_FOREACH(e_config->acpi_bindings, l, bi)
     {
#define CHK_ACPIBIND_DFLT(_type, _status, _action, _params) \
  if (                                                      \
    (bi->type == _type) &&                                  \
    (bi->status == _status) &&                              \
    _ech_streq(bi->action, _action) &&                      \
    _ech_streq(bi->params, _params)                         \
    )                                                       \
    continue

        /* should probably add a restore defaults button to this cfg dialog sometime */
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_BRIGHTNESS_DOWN, -1, "backlight_adjust", "-0.1");
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_BRIGHTNESS_UP, -1, "backlight_adjust", "0.1");
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_BRIGHTNESS, 0, "dim_screen", NULL);
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_BRIGHTNESS, 1, "undim_screen", NULL);
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_BUTTON, -1, "syscon", NULL);
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_POWER, -1, "halt", NULL);
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_SLEEP, -1, "suspend", "now");
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_HIBERNATE, -1, "hibernate", "now");
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_LID, 1, "suspend", "now");
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_MUTE, -1, "volume_mute", NULL);
        CHK_ACPIBIND_DFLT(E_ACPI_TYPE_BRIGHTNESS, -1, "backlight", NULL);
        ret++;
     }
   return ret;
}

EINTERN unsigned int
ech_bindings_check_signal(void)
{
   E_Config_Binding_Signal *bi;
   unsigned int ret = 0;
   Eina_List *l;

   EINA_LIST_FOREACH(e_config->signal_bindings, l, bi)
     {
#define CHK_SIGBIND_DFLT(_signal, _source, _action, _params) \
  if (                                                       \
    _ech_streq(bi->signal, _signal) &&                       \
    _ech_streq(bi->source, _source) &&                       \
    _ech_streq(bi->action, _action) &&                       \
    _ech_streq(bi->params, _params)                          \
    )                                                        \
    continue

        CHK_SIGBIND_DFLT("mouse,down,1,double", "e.event.titlebar", "window_shaded_toggle", "up");
        CHK_SIGBIND_DFLT("mouse,down,2", "e.event.titlebar", "window_shaded_toggle", "up");
        CHK_SIGBIND_DFLT("mouse,wheel,?,1", "e.event.titlebar", "window_shaded", "0 up");
        CHK_SIGBIND_DFLT("mouse,wheel,?,-1", "e.event.titlebar", "window_shaded", "1 up");
        CHK_SIGBIND_DFLT("mouse,clicked,3", "e.event.titlebar", "window_menu", NULL);
        CHK_SIGBIND_DFLT("mouse,clicked,?", "e.event.icon", "window_menu", NULL);
        CHK_SIGBIND_DFLT("mouse,clicked,[12]", "e.event.close", "window_close", NULL);
        CHK_SIGBIND_DFLT("mouse,clicked,3", "e.event.close", "window_kill", NULL);
        CHK_SIGBIND_DFLT("mouse,clicked,1", "e.event.maximize", "window_maximized_toggle", NULL);
        CHK_SIGBIND_DFLT("mouse,clicked,2", "e.event.maximize", "window_maximized_toggle", "smart");
        CHK_SIGBIND_DFLT("mouse,clicked,3", "e.event.maximize", "window_maximized_toggle", "expand");
        CHK_SIGBIND_DFLT("mouse,clicked,?", "e.event.minimize", "window_iconic_toggle", NULL);
        CHK_SIGBIND_DFLT("mouse,clicked,?", "e.event.shade", "window_shaded_toggle", "up");
        CHK_SIGBIND_DFLT("mouse,clicked,?", "e.event.lower", "window_lower", NULL);
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.icon", "window_drag_icon", NULL);
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.titlebar", "window_move", NULL);
        CHK_SIGBIND_DFLT("mouse,up,1", "e.event.titlebar", "window_move", "end");
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.resize.tl", "window_resize", "tl");
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.resize.t", "window_resize", "t");
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.resize.tr", "window_resize", "tr");
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.resize.r", "window_resize", "r");
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.resize.br", "window_resize", "br");
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.resize.b", "window_resize", "b");
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.resize.bl", "window_resize", "bl");
        CHK_SIGBIND_DFLT("mouse,down,1", "e.event.resize.l", "window_resize", "l");
        CHK_SIGBIND_DFLT("mouse,up,1", "e.event.resize.*", "window_resize", "end");
        CHK_SIGBIND_DFLT("mouse,down,3", "e.event.resize.*", "window_move", NULL);
        CHK_SIGBIND_DFLT("mouse,up,3", "e.event.resize.*", "window_move", "end");
        ret++;
     }
   return ret;
}

