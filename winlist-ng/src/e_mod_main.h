#define D_(str) dgettext(PACKAGE, str)

#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include <e.h>
#include <Efreet.h>


typedef struct _Config      Config;
typedef struct _Config_Item Config_Item;
typedef struct _Ng      Ng;
typedef struct _Ngw_Item Ngw_Item;
typedef struct _Ngw_Win  Ngw_Win;
typedef struct _Ngw_Box  Ngw_Box;

struct _Config
{
  E_Module      *module;
  Eina_List     *instances;
  E_Menu        *menu;
  Eina_List     *handlers;

  char          *theme_path;

  Config_Item   *winlist_cfg;

  E_Config_Binding_Key winlist_toggle_key;
  E_Config_Binding_Key winlist_next_key;
  E_Config_Binding_Key winlist_prev_key;

  E_Config_Dialog *cfd;

  int use_composite;
};


struct _Config_Item
{
  int            orient;
  int            size;
  float          zoomfactor;
  float          zoom_range;

  int            hide_animation;

  float	         hide_timeout;
  float	         zoom_duration;

  int            animation_style;
  
  Ng            *ng;
};


struct _Ngw_Win
{
  int             x, y, w, h;
  E_Popup        *popup;
  Ecore_Evas     *ee;
  Evas           *evas;
  Evas_Object    *bg;

  Ng             *ng;
};


struct _Ng
{
  Ngw_Win         *win;
  Ngw_Box         *box;

  Config_Item    *cfg;

  E_Zone         *zone;
  Eina_List      *items;

  E_Config_Dialog *cfd;
};


struct _Ngw_Item
{
  Ng             *ng;
  Evas_Object    *obj;
  Evas_Object    *o_icon;
  E_Border       *border;
  char           *label;

  int            x,y,w,h;
  double         scale;

  E_Desk   *desk;
  E_Border *bd_above;
  
  int      was_iconified;
  int      was_shaded;
  int      set_state;
  
};


struct _Ngw_Box
{
  Ng              *ng;
  Evas_Coord       x, y, w, h;
  Evas_Coord       win_w, win_h;

  Evas_Object     *o_bg;
  Evas_Object     *o_over;
  Evas_Object     *clip;
  Evas_Object     *item_bg;
  
  unsigned char    changed : 1;

  int              pos;

  double           zoom;

  int              icon_size;
};




EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI int   e_modapi_about    (E_Module *m);
EAPI int   e_modapi_config   (E_Module *m);


Ng          *ngw_new                  (Config_Item *ci);
void         ngw_remove_items         (Ng *ng);
void         ngw_free                 (Ng *ng);
void         ngw_update_config        (Ng *ng, int change_type, int chg_orient, int chg_apps);

E_Config_Dialog *ngw_configure_module (Config_Item *ci);

Ngw_Item    *ngw_item_new             (Ng *ng, E_Border *bd);

void         ngw_item_remove          (Ngw_Item *it);
void         ngw_item_signal_emit     (Ngw_Item *it, char *sig);
void         ngw_item_over_signal_emit(Ngw_Item *it, char *sig);

void         ngw_item_mouse_in        (Ngw_Item *it);
void         ngw_item_mouse_out       (Ngw_Item *it);
void         ngw_item_mouse_down      (Ngw_Item *it, Evas_Event_Mouse_Down *ev);
void         ngw_item_mouse_up        (Ngw_Item *it, Evas_Event_Mouse_Up *ev);
void         ngw_item_drag_start      (Ngw_Item *it);
void         ngw_item_free            (Ngw_Item *it);

Ngw_Box      *ngw_box_new             (Ng *ng);
void         ngw_box_free             (Ngw_Box *box);
void         ngw_box_item_show        (Ngw_Box *box, Ngw_Item *it);
void         ngw_box_item_remove      (Ngw_Box *box, Ngw_Item *it);

void         ngw_box_reposition       (Ngw_Box *box);
void         ngw_box_position_to_item (Ngw_Box *box, Ngw_Item *it);

void         ngw_win_show             (Ngw_Win *win);
void         ngw_win_hide             (Ngw_Win *win);




void         ngw_border_menu_show     (Ng *ng, E_Border *bd, Evas_Coord x, Evas_Coord y, int dir, Ecore_X_Time timestamp);


int ngw_winlist_init(void);
int ngw_winlist_shutdown(void);

int  ngw_winlist_show(void);
void ngw_winlist_hide(void);
int  ngw_winlist_active_get(void);
void ngw_winlist_next(void);
void ngw_winlist_prev(void);
void ngw_winlist_modifiers_set(int mod);

extern Config *ngw_config;
extern E_Config_DD *conf_edd;
extern E_Config_DD *conf_item_edd;

#endif

