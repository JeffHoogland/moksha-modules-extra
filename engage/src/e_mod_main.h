#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include <e.h>
#include <X11/Xlib.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define D_(string) dgettext(PACKAGE, string)
#else
# define bindtextdomain(domain,dir)
# define bind_textdomain_codeset(domain,codeset)
# define D_(string) (string)
#endif

#define MOD_CONFIG_FILE_EPOCH 0x0001
#define MOD_CONFIG_FILE_GENERATION 0x0001
#define MOD_CONFIG_FILE_VERSION					\
  ((MOD_CONFIG_FILE_EPOCH << 16) | MOD_CONFIG_FILE_GENERATION)

#define AUTOHIDE_NONE		0
#define AUTOHIDE_NORMAL		1
#define AUTOHIDE_FULLSCREEN	2
#define AUTOHIDE_OVERLAP	3

typedef struct _Config             Config;
typedef struct _Config_Item        Config_Item;
typedef struct _Config_Box         Config_Box;
typedef struct _Config_Gadcon      Config_Gadcon;
typedef struct _Ng                 Ng;
typedef struct _Ngi_Item           Ngi_Item;
typedef struct _Ngi_Item_Launcher  Ngi_Item_Launcher;
typedef struct _Ngi_Item_Taskbar   Ngi_Item_Taskbar;
typedef struct _Ngi_Item_Gadcon    Ngi_Item_Gadcon;
typedef struct _Ngi_Win            Ngi_Win;
typedef struct _Ngi_Box            Ngi_Box;

struct _Config
{
  int            version;

  E_Module      *module;
  Eina_List     *instances;
  E_Menu        *menu;

  Eina_List     *items;
  char          *theme_path;

  int            use_composite;
  E_Config_Dialog *cfd;
  Eina_List       *handlers;
};

struct _Config_Item
{
  Ng            *ng;

  int            show_label;
  int            show_background;
  int            container;
  int            zone;
  int            orient;
  int            size;
  int            window_size;
  int	         autohide;
  int	         autohide_show_urgent;
  int            hide_below_windows;
  float          zoomfactor;
  float          zoom_range;
  float	         hide_timeout;
  float	         zoom_duration;
  int            alpha;
  int            mouse_over_anim;

  enum { above_all, below_fullscreen } stacking;

  int            lock_deskswitch;
  int            ecomorph_features;

  Eina_List     *boxes;

  E_Config_Dialog *config_dialog;
};

struct _Config_Box
{
  enum { launcher, taskbar, gadcon } type;

  int            taskbar_skip_dialogs;
  int            taskbar_adv_bordermenu;
  int            taskbar_show_iconified;
  int            taskbar_show_desktop;
  int            taskbar_append_right;
  int            taskbar_group_apps;

  const char    *launcher_app_dir;
  int            launcher_lock_dnd;

  Eina_List     *gadcon_items;

  Ngi_Box       *box;
};

struct _Config_Gadcon
{
  const char    *name;
};



struct _Ngi_Win
{
  Ng             *ng;
  E_Popup        *popup;
  Ecore_X_Window  input;
  Ecore_X_Rectangle rect;
  E_Win          *fake_iwin;
  E_Object       *drop_win;
};



struct _Ng
{
  Ngi_Win         *win;
  Evas            *evas;
  Eina_List       *boxes;

  Config_Item     *cfg;

  E_Zone          *zone;
  E_Shelf        *es;

  Evas_Object     *o_bg;
  Evas_Object     *o_frame;
  Evas_Object     *o_label;

  Evas_Object     *clip;
  Evas_Object     *bg_clip;

  unsigned char    changed : 1;

  int              pos;  /* mouse position    */
  double           size; /* current icon size */

  enum { unzoomed, zooming, zoomed, unzooming } state;
  enum { hidden, hiding, showing, show } hide_state;
  int              hide;

  double           zoom;
  double           start_zoom;
  double           start_hide;

  Eina_List       *items_show;
  Eina_List       *items_remove;

  Ngi_Item        *item_active;

  int              hide_step;

  int              w, h, start;
  int              horizontal;

  int              mouse_in;
  int              show_bar;

  int              dnd;
  Ngi_Item        *item_drag;

  Eina_List       *handlers;

  Ecore_Animator  *animator;
  Ecore_Timer     *mouse_in_timer;
  Ecore_Timer     *menu_wait_timer;
  Ecore_Timer     *effect_timer;

  struct
  {
    int clip_separator;
    int separator_width;
    int item_spacing;
    int edge_offset;
    int bg_offset;
    int reflection_offset;
    double keep_overlay_pos;
    double fade_duration;
  } opt;
};

struct _Ngi_Item
{
  Ngi_Box        *box;

  Evas_Object    *obj;
  Evas_Object    *over;

  const char     *label;

  unsigned int    mouse_down;
  int             pos;
  double          displace;
  double          scale;

  struct
  {
    unsigned char  start : 1;
    unsigned char  dnd : 1;
    int            x, y;
    int            dx, dy;
  } drag;

  void (*cb_free)       (Ngi_Item *it);
  void (*cb_mouse_down) (Ngi_Item *it, Ecore_Event_Mouse_Button *ev);
  void (*cb_mouse_up)   (Ngi_Item *it, Ecore_Event_Mouse_Button *ev);
  void (*cb_mouse_in)   (Ngi_Item *it);
  void (*cb_mouse_out)  (Ngi_Item *it);
  void (*cb_drag_start) (Ngi_Item *it);

  double start_time;

  int delete_me;
};

struct _Ngi_Item_Launcher
{
  Ngi_Item base;

  Evas_Object    *o_icon;
  Evas_Object    *o_proxy;

  Efreet_Desktop *app;
};


struct _Ngi_Item_Taskbar
{
  Ngi_Item base;

  Evas_Object    *o_icon;
  Evas_Object    *o_proxy;

  E_Border       *border;

  unsigned int border_was_fullscreen;
  unsigned int urgent;
  const char  *class; /* store icccm.class...*/
};

struct _Ngi_Item_Gadcon
{
  Ngi_Item base;

  Evas_Object    *o_icon;
  Evas_Object    *o_proxy;

  E_Gadcon       *gadcon;
  Config_Gadcon  *cfg_gadcon;

};


struct _Ngi_Box
{
  Ng              *ng;
  Config_Box      *cfg;
  Eina_List       *handlers;
  Eina_List       *items;
  Ecore_Timer     *dnd_timer;
  E_Drop_Handler  *drop_handler;
  Ngi_Item        *item_drop;
  Evas_Object     *separator;
  Evas_Coord       pos, w;

  E_Order         *apps;

  E_Config_Dialog *cfd;
};

EAPI extern E_Module_Api e_modapi;
EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI int   e_modapi_about    (E_Module *m);
EAPI int   e_modapi_config   (E_Module *m);

EAPI E_Config_Dialog *ngi_instances_config(E_Container *con, const char *params);



Ng          *ngi_new                          (Config_Item *ci);
void         ngi_free                         (Ng *ng);
void         ngi_reposition                   (Ng *ng);
void         ngi_input_extents_calc           (Ng *ng);
void         ngi_freeze                       (Ng *ng);
void         ngi_thaw                         (Ng *ng);
void         ngi_animate                      (Ng *ng);
void         ngi_mouse_in                     (Ng *ng);
void         ngi_mouse_out                    (Ng *ng);
Ngi_Item *   ngi_item_at_position_get         (Ng *ng);
void         ngi_bar_lock                     (Ng *ng, int lock);
Config_Item *ngi_bar_config_new               (int container_num, int zone_num);

void         ngi_win_show                     (Ngi_Win *win);
void         ngi_win_hide                     (Ngi_Win *win);
void         ngi_win_position_calc            (Ngi_Win *win);

Ngi_Box     *ngi_box_new                      (Ng *ng);
void         ngi_box_free                     (Ngi_Box *box);

void         ngi_configure_module             (Config_Item *ci);
void         ngi_configure_box                (Ngi_Box *box);

void         ngi_item_init_defaults           (Ngi_Item *it);
void         ngi_item_label_set(Ngi_Item *it, const char *label);


void         ngi_item_free                    (Ngi_Item *it);
void         ngi_item_show                    (Ngi_Item *it, int instant);
void         ngi_item_remove                  (Ngi_Item *it);
void         ngi_item_del_icon                (Ngi_Item *it);
void         ngi_item_mouse_down              (Ngi_Item *it, Ecore_Event_Mouse_Button *ev);
void         ngi_item_mouse_up                (Ngi_Item *it, Ecore_Event_Mouse_Button *ev);
void         ngi_item_mouse_in                (Ngi_Item *it);
void         ngi_item_mouse_out               (Ngi_Item *it);
void         ngi_item_signal_emit             (Ngi_Item *it, char *sig);
void         ngi_item_activate                (Ng *ng);

void         ngi_launcher_new                 (Ng *ng, Config_Box *cfg_box);
void         ngi_launcher_remove              (Ngi_Box *box);

void         ngi_taskbar_init                 (void);
void         ngi_taskbar_new                  (Ng *ng, Config_Box *cfg_box);
void         ngi_taskbar_remove               (Ngi_Box *box);

void         ngi_gadcon_init                  (void);
void         ngi_gadcon_shutdown              (void);
void         ngi_gadcon_new                   (Ng *ng, Config_Box *cfg_box);
void         ngi_gadcon_remove                (Ngi_Box *box);
void         ngi_gadcon_config                (Ngi_Box *box);

void         ngi_border_menu_show             (Ngi_Box *box, E_Border *bd,
                                               Evas_Coord x, Evas_Coord y,
                                               int dir, Ecore_X_Time timestamp);

Eina_Bool    ngi_object_theme_set(Evas_Object *obj, const char *part);

extern Config *ngi_config;
extern E_Config_DD *ngi_conf_edd;
extern E_Config_DD *ngi_conf_item_edd;
extern E_Config_DD *ngi_conf_box_edd;
extern E_Config_DD *ngi_conf_gadcon_edd;
#endif
