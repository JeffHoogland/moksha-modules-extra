#ifdef E_MOD_PHOTO_TYPEDEFS

typedef struct _Photo_Config Photo_Config;
typedef struct _Photo_Config_Item Photo_Config_Item;

#else

#ifndef PHOTO_CONFIG_H_INCLUDED
#define PHOTO_CONFIG_H_INCLUDED

#define CONFIG_VERSION 3

struct _Photo_Config
{
   int version;

   int show_label;
   int nice_trans;
   
   int pictures_from;
   int pictures_set_bg_purge;
   const char *pictures_viewer;
   int pictures_thumb_size;
   struct
   {
      Eina_List *dirs;
      int auto_reload;
      int popup;
      int thumb_msg;
   } local;
   struct
   {
      
   } net;

   int action_mouse_over;
   int action_mouse_left;
   int action_mouse_middle;

   Eina_List *items;
};

struct _Photo_Config_Item
{
   const char *id;

   int timer_s;
   int timer_active;

   int alpha;
   int show_label;

   int action_mouse_over;
   int action_mouse_left;
   int action_mouse_middle;
};

int                photo_config_init(void);
int                photo_config_shutdown(void);

int                photo_config_save(void);

Photo_Config_Item *photo_config_item_new(const char *id);
void               photo_config_item_free(Photo_Config_Item *pic);

#endif
#endif
