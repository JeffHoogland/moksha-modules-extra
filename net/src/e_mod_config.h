#ifndef E_MOD_CONFIG_H
#define E_MOD_CONFIG_H

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;

struct _Config 
{
   E_Module *mod;
   E_Config_Dialog *cfd;
   E_Menu *menu;
   Eina_List *instances;
   Eina_List *items;
};

struct _Config_Item 
{
   const char *id;
   const char *device;
   const char *app;
   int limit, show_text;
   int show_popup;
};

Config_Item *_net_config_item_get(const char *id);
Eina_List *_net_config_devices_get(void);
void _net_config_updated(Config_Item *ci);

extern Config *net_cfg;

#endif
