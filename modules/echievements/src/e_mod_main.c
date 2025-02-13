#include "e_mod_main.h"

EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Echievements"};
static E_Config_DD *conf_edd = NULL;
static Eina_List *handlers = NULL;

EINTERN Mod *mod = NULL;
EINTERN Config *ech_config = NULL;

static void
_e_mod_ech_config_free(void)
{
   etrophy_gamescore_free(ech_config->gs);
   E_FREE(ech_config);
}

static void
_e_mod_ech_config_load(void)
{
#undef T
#undef D
   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, config_version, UINT);
   E_CONFIG_SUB(D, T, gs, etrophy_gamescore_edd_get());

   ech_config = e_config_domain_load("module.echievements", conf_edd);
   if (ech_config)
     {
        if (!e_util_module_config_check("Echievements", ech_config->config_version, MOD_CONFIG_FILE_VERSION))
          _e_mod_ech_config_free();
     }

   if (!ech_config)
     {
        ech_config = E_NEW(Config, 1);
        ech_config->config_version = (MOD_CONFIG_FILE_EPOCH << 16);
     }
   if (!ech_config->gs)
     ech_config->gs = etrophy_gamescore_new("echievements");
}

static Eina_Bool
_e_mod_ech_mod_init_end_cb(void *d EINA_UNUSED, int type EINA_UNUSED, void *ev EINA_UNUSED)
{
   ecore_event_handler_del(eina_list_data_get(handlers));
   handlers = eina_list_remove_list(handlers, handlers);
   mod->module_init_end = 1;
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_mod_ech_mouse_move_cb(void *d EINA_UNUSED, int type EINA_UNUSED, Ecore_Event_Mouse_Move *ev)
{
   if ((mod->mouse.x != mod->mouse.y) || (mod->mouse.x != -1))
     {
        Echievement *ec;
        Eina_List *l;

        mod->mouse.dx = ev->root.x - mod->mouse.x, mod->mouse.dy = ev->root.y - mod->mouse.y;
        EINA_LIST_FOREACH(mod->mouse.hooks, l, ec)
          if (ec->mouse_hook) ec->mouse_hook(ec);
     }
   mod->mouse.x = ev->root.x, mod->mouse.y = ev->root.y;
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_mod_ech_desklock_timer_cb(void *d EINA_UNUSED)
{
   mod->desklock.timers = eina_list_remove_list(mod->desklock.timers, mod->desklock.timers);
   return EINA_FALSE;
}

static Eina_Bool
_e_mod_ech_desklock_cb(void *d EINA_UNUSED, int type EINA_UNUSED, E_Event_Desklock *ev)
{
   if (!ev->on)
     mod->desklock.timers = eina_list_append(mod->desklock.timers,
       ecore_timer_add(60 * 60, _e_mod_ech_desklock_timer_cb, NULL));

   return ECORE_CALLBACK_RENEW;
}

EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[4096];

   bindtextdomain(PACKAGE, LOCALEDIR);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   snprintf(buf, sizeof(buf), "%s/e-module-echievements.edj", m->dir);

   e_configure_registry_category_add("extensions", 80, D_("Extensions"),
                                     NULL, "preferences-extensions");
   e_configure_registry_item_add("extensions/echievements", 110, D_("Echievements"),
                                 NULL, buf, e_int_config_echievements);

   etrophy_init();
   e_notification_init();

   mod = E_NEW(Mod, 1);
   mod->module = m;
   mod->mouse.x = mod->mouse.y = -1;

   mod->module_init_end = !e_module_loading_get();
   if (!mod->module_init_end)
     E_LIST_HANDLER_APPEND(handlers, E_EVENT_MODULE_INIT_END, _e_mod_ech_mod_init_end_cb, NULL);
   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_MOUSE_MOVE, _e_mod_ech_mouse_move_cb, NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_DESKLOCK, _e_mod_ech_desklock_cb, NULL);

   _e_mod_ech_config_load();
   ech_init();

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_configure_registry_item_del("extensions/echievements");

   e_configure_registry_category_del("extensions");

   E_FN_DEL(e_object_del, mod->cfd);
   ech_shutdown();
   e_config_domain_save("module.echievements", conf_edd, ech_config);
   _e_mod_ech_config_free();
   E_CONFIG_DD_FREE(conf_edd);
   eina_list_free(mod->mouse.hooks);
   E_FREE_LIST(mod->desklock.timers, ecore_timer_del);
   E_FREE(mod);
   E_FREE_LIST(handlers, ecore_event_handler_del);
   e_notification_shutdown();
   etrophy_shutdown();
   return 1;
}

EAPI int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.echievements", conf_edd, ech_config);
   return 1;
}

