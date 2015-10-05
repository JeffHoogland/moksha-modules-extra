#ifdef E_MOD_PHOTO_TYPEDEFS

typedef struct _Photo_Item Photo_Item;

#else

#ifndef PHOTO_ITEM_H_INCLUDED
#define PHOTO_ITEM_H_INCLUDED

#define ITEM_EDJE_MSG_SEND_LABEL_ALWAYS 0

#define ITEM_NICE_TRANS_DEFAULT 1

#define ITEM_TIMER_S_DEFAULT 30
#define ITEM_TIMER_S_MIN 5
#define ITEM_TIMER_S_MAX (60*5)
#define ITEM_TIMER_ACTIVE_DEFAULT 1

#define ITEM_ALPHA_DEFAULT 255

#define ITEM_SHOW_LABEL_DEFAULT 0
#define ITEM_SHOW_LABEL_NO 0
#define ITEM_SHOW_LABEL_YES 1
#define ITEM_SHOW_LABEL_PARENT 2

#define ITEM_MOUSE_OVER_ACTION_DEFAULT 0
#define ITEM_MOUSE_LEFT_ACTION_DEFAULT 2
#define ITEM_MOUSE_MIDDLE_ACTION_DEFAULT 7

#define ITEM_ACTION_NO 0
#define ITEM_ACTION_LABEL 1
#define ITEM_ACTION_INFOS 2
#define ITEM_ACTION_PREV 3
#define ITEM_ACTION_NEXT 4
#define ITEM_ACTION_PAUSE 5
#define ITEM_ACTION_SETBG 6
#define ITEM_ACTION_VIEWER 7
#define ITEM_ACTION_MENU 8
#define ITEM_ACTION_PARENT 10

#define ITEM_INFOS_TIMER 5

struct _Photo_Item
{
   E_Gadcon_Client *gcc;
   Evas_Object *obj;

   Photo_Config_Item *config;
   E_Config_Dialog *config_dialog;
   E_Menu *menu;
  E_Menu *menu_histo;

   Ecore_Timer *timer;

   Picture *picture0;
   Picture *picture1;
   int edje_part;
   int in_transition;

   Popup_Warn *popw;
   Popup_Info *popi;
   
   struct
   {
      Eina_List *list;
      int pos;
      Popup_Info *popi;
   } histo;

   Ecore_Event_Handler *local_ev_fill_handler;
   Ecore_Event_Handler *net_ev_fill_handler;
};


Photo_Item  *photo_item_add(E_Gadcon_Client *gcc, Evas_Object *obj, const char *id);
void         photo_item_del(Photo_Item *pi);

void         photo_item_timer_set(Photo_Item *pi, int active, int time);
void         photo_item_label_mode_set(Photo_Item *pi);

Picture     *photo_item_picture_current_get(Photo_Item *pi);
Evas_Object *photo_item_picture_object_current_get(Photo_Item *pi);

int          photo_item_action_label(Photo_Item *pi);
int          photo_item_action_infos(Photo_Item *pi);
int          photo_item_action_change(Photo_Item *pi, int position);
int          photo_item_action_pause_toggle(Photo_Item *pi);
int          photo_item_action_setbg(Photo_Item *pi);
int          photo_item_action_viewer(Photo_Item *pi);
int          photo_item_action_menu(Photo_Item *pi, Evas_Event_Mouse_Down *ev);

#endif
#endif
