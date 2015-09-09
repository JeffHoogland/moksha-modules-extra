#include <e.h>
#include "e_mod_config.h"
#include "e_mod_main.h"
#include "e_mod_gadcon.h"

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *item_edd = NULL;
Config *net_cfg = NULL;

EAPI E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Net" };

EAPI void *
e_modapi_init(E_Module *m) 
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   item_edd = E_CONFIG_DD_NEW("Config_Item", Config_Item);
   E_CONFIG_VAL(item_edd, Config_Item, id, STR);
   E_CONFIG_VAL(item_edd, Config_Item, device, STR);
   E_CONFIG_VAL(item_edd, Config_Item, app, STR);
   E_CONFIG_VAL(item_edd, Config_Item, limit, INT);
   E_CONFIG_VAL(item_edd, Config_Item, show_popup, INT);
   E_CONFIG_VAL(item_edd, Config_Item, show_text, INT);
   
   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   E_CONFIG_LIST(conf_edd, Config, items, item_edd);

   net_cfg = e_config_domain_load("module.net", conf_edd);
   if (!net_cfg) 
     {
	Config_Item *ci;

	net_cfg = E_NEW(Config, 1);
	ci = _net_config_item_get("0");
     }
   net_cfg->mod = m;
   _net_gc_register();
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m) 
{
   _net_gc_unregister();
   net_cfg->mod = NULL;
   
   if (net_cfg->cfd) e_object_del(E_OBJECT(net_cfg->cfd));
   if (net_cfg->menu) 
     {
	e_menu_post_deactivate_callback_set(net_cfg->menu, NULL, NULL);
	e_object_del(E_OBJECT(net_cfg->menu));
	net_cfg->menu = NULL;
     }
   
   while (net_cfg->items) 
     {
	Config_Item *ci;

	ci = net_cfg->items->data;
	if (ci->id) eina_stringshare_del(ci->id);
	if (ci->device) eina_stringshare_del(ci->device);
	if (ci->app) eina_stringshare_del(ci->app);
	net_cfg->items = eina_list_remove_list(net_cfg->items, net_cfg->items);
	E_FREE(ci);
     }
   
   E_FREE(net_cfg);
   E_CONFIG_DD_FREE(item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m) 
{
   e_config_domain_save("module.net", conf_edd, net_cfg);
   return 1;
}
