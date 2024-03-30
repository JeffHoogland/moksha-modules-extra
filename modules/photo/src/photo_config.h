#ifdef E_MOD_PHOTO_TYPEDEFS

typedef struct _Photo_Config Photo_Config;
typedef struct _Photo_Config_Item Photo_Config_Item;

#else

#ifndef PHOTO_CONFIG_H_INCLUDED
#define PHOTO_CONFIG_H_INCLUDED

#define CONFIG_VERSION 4

#define IMPORT_STRETCH          0
#define IMPORT_TILE             1
#define IMPORT_CENTER           2
#define IMPORT_SCALE_ASPECT_IN  3
#define IMPORT_SCALE_ASPECT_OUT 4
#define IMPORT_PAN              5

#define PHOTO_BG_DIALOG_DEFAULT EINA_FALSE
#define PHOTO_BG_METHOD_DEFAULT IMPORT_CENTER
#define PHOTO_BG_EXTERNAL_DEFAULT EINA_FALSE
#define PHOTO_BG_QUALITY_DEFAULT 100
#define PHOTO_BG_COLOR_R_DEFAULT 0
#define PHOTO_BG_COLOR_G_DEFAULT 0
#define PHOTO_BG_COLOR_B_DEFAULT 0
#define PHOTO_BG_COLOR_A_DEFAULT 255

struct _Photo_Config
{
   int version;

   int show_label;
   int nice_trans;
   
   int pictures_from;
   int pictures_set_bg_purge;
   const char *pictures_viewer;
   int pictures_thumb_size;
   int bg_dialog;
   int bg_method;
   int bg_external;
   int bg_quality;
   int bg_color_r;
   int bg_color_g;
   int bg_color_b;
   int bg_color_a;
   struct
   {
      Eina_List *dirs;
      int auto_reload;
      int popup;
      int thumb_msg;
   } local;
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
