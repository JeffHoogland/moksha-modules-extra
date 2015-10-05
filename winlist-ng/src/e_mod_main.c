#include "e_mod_main.h"
#include "e_mod_keybindings.h"
#include <X11/extensions/shape.h>



static int      _ngw_win_free(Ngw_Win *win);
static Ngw_Win *_ngw_win_new(Ng *ng);
static void     _ngw_win_position_calc(Ngw_Win *es);
static Eina_Bool      _ngw_win_container_resize_cb(void *data, int type, void *event);


E_Config_DD *conf_edd = NULL;
E_Config_DD *conf_item_edd = NULL;
Config      *ngw_config = NULL;



Ng *
ngw_new(Config_Item *cfg)
{
   Ng *ng;
   cfg->ng = NULL;

   ng = E_NEW(Ng, 1);
   cfg->ng = ng;
   ng->cfg = cfg;

   ng->items = NULL;
  
   ng->zone = e_util_zone_current_get(e_manager_current_get());

   ng->win = _ngw_win_new(ng);
   _ngw_win_position_calc(ng->win);

   ng->box = ngw_box_new(ng);

   return ng;
}

void
ngw_remove_items(Ng *ng)
{
   while (ng->items)
     ngw_item_remove((Ngw_Item*)ng->items->data);
}

void
ngw_free(Ng *ng)
{
   ngw_config->instances = eina_list_remove(ngw_config->instances, ng);

   ngw_remove_items(ng);

   ngw_box_free(ng->box);

   _ngw_win_free(ng->win);
   free(ng);
}

/* TODO */
void
ngw_update_config(Ng *ng, int change_type, int change_orient, int chg_apps)
{
}

EAPI E_Config_Dialog *
_ngw_config_dialog(E_Container *con, const char *params)
{
   return ngw_configure_module(ngw_config->winlist_cfg);
}


/***************************************************************************/

EAPI E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Ngw_Winlist" };

EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[4096];

   /* Location of message catalogs for localization */
   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");
  
   conf_item_edd = E_CONFIG_DD_NEW("Ngw_Config_Item", Config_Item);
#undef T
#undef D
#define T Config_Item
#define D conf_item_edd
   E_CONFIG_VAL(D, T, orient, INT);
   E_CONFIG_VAL(D, T, size, INT);
   E_CONFIG_VAL(D, T, hide_timeout, FLOAT);
   E_CONFIG_VAL(D, T, zoom_duration, FLOAT);
   E_CONFIG_VAL(D, T, zoomfactor, FLOAT);
   E_CONFIG_VAL(D, T, zoom_range, FLOAT);
   E_CONFIG_VAL(D, T, hide_animation, INT);
   conf_edd = E_CONFIG_DD_NEW("Ngw_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_VAL(D, T, winlist_next_key.context, INT);
   E_CONFIG_VAL(D, T, winlist_next_key.modifiers, INT);
   E_CONFIG_VAL(D, T, winlist_next_key.key, STR);
   E_CONFIG_VAL(D, T, winlist_next_key.action, STR);
   E_CONFIG_VAL(D, T, winlist_next_key.params, STR);
   E_CONFIG_VAL(D, T, winlist_next_key.any_mod, UCHAR);
   E_CONFIG_VAL(D, T, winlist_prev_key.context, INT);
   E_CONFIG_VAL(D, T, winlist_prev_key.modifiers, INT);
   E_CONFIG_VAL(D, T, winlist_prev_key.key, STR);
   E_CONFIG_VAL(D, T, winlist_prev_key.action, STR);
   E_CONFIG_VAL(D, T, winlist_prev_key.params, STR);
   E_CONFIG_VAL(D, T, winlist_prev_key.any_mod, UCHAR);

   E_CONFIG_SUB(D, T, winlist_cfg, conf_item_edd);
#undef T
#undef D


   ngw_config = (Config*) e_config_domain_load("module.winlist_ng", conf_edd);
   if(!ngw_config)
     {
	ngw_config = E_NEW(Config, 1);
	ngw_config->winlist_next_key.context	 = E_BINDING_CONTEXT_ANY;
	ngw_config->winlist_next_key.key	         = eina_stringshare_add("comma");
	ngw_config->winlist_next_key.modifiers	 = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	ngw_config->winlist_next_key.any_mod	 = 0;
	ngw_config->winlist_next_key.action	 = eina_stringshare_add(NGW_WINLIST_NEXT_ACTION);
	ngw_config->winlist_next_key.params	 = NULL;

	ngw_config->winlist_prev_key.context	 = E_BINDING_CONTEXT_ANY;
	ngw_config->winlist_prev_key.key	         = eina_stringshare_add("period");
	ngw_config->winlist_prev_key.modifiers	 = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	ngw_config->winlist_prev_key.any_mod	 = 0;
	ngw_config->winlist_prev_key.action	 = eina_stringshare_add(NGW_WINLIST_PREV_ACTION);
	ngw_config->winlist_prev_key.params	 = NULL;

	Config_Item *cfg = E_NEW(Config_Item, 1);
	cfg->orient = E_GADCON_ORIENT_FLOAT;
	cfg->size = 34;
	cfg->zoom_duration = 0.3;
	cfg->zoom_range = 0.6;
	cfg->hide_timeout  = 0.2;
	cfg->zoomfactor  = 2.0;
	cfg->hide_animation = 1;
      
	ngw_config->winlist_cfg = cfg;
     }

   ngw_config->module = m;
   ngw_config->cfd = NULL;

   snprintf(buf, sizeof(buf), "%s/ng.edj", e_module_dir_get(ngw_config->module));
   ngw_config->theme_path = strdup(buf);
  
   ngw_config->handlers = eina_list_append
     (ngw_config->handlers, ecore_event_handler_add
      (E_EVENT_CONTAINER_RESIZE, _ngw_win_container_resize_cb, NULL));

   if(ecore_x_screen_is_composited(0) || e_config->use_composite)
     {
	ngw_config->use_composite = 1;
	printf("found composite manager\n");
     }
   else
     {
	ngw_config->use_composite = 0;    
	printf("no composite manager found\n");
     }

   ngw_winlist_init();

   ngw_register_module_actions();
   ngw_register_module_keybindings();

   e_configure_registry_item_add("extensions/winlist_ng", 50, D_("Winlist NG"), NULL, "enlightenment/winlist", _ngw_config_dialog);

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   while (ngw_config->handlers)
     {
	ecore_event_handler_del(ngw_config->handlers->data);
	ngw_config->handlers = eina_list_remove_list(ngw_config->handlers, ngw_config->handlers);
     }

   ngw_winlist_shutdown();
   free(ngw_config->theme_path);
   free(ngw_config);
   ngw_config = NULL;

   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);

   e_configure_registry_item_del("extensions/winlist_ng");

   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   e_config_domain_save("module.winlist_ng", conf_edd, ngw_config);
   return 1;
}

EAPI int
e_modapi_about(E_Module *m)
{
   e_module_dialog_show(m,D_("Winlist NG Module"),
			D_("a replacement for the standard winlist"));
   return 1;
}

EAPI int
e_modapi_config(E_Module *m)
{
   ngw_configure_module(ngw_config->winlist_cfg);
   return 1;
}


/**************************************************************************/

static Ngw_Win *
_ngw_win_new(Ng *ng)
{
   Ngw_Win *win;

   win = E_NEW(Ngw_Win, 1);
   if (!win) return NULL;

   win->ng = ng;

   win->popup = e_popup_new(ng->zone, 0, 0, 0, 0);

   win->ee = win->popup->ecore_evas;
   win->evas = win->popup->evas;

   if(e_config->use_composite)
     {
	ecore_evas_alpha_set(win->ee, 1);
     }
   else
     {
	win->popup->shaped = 1;
	ecore_evas_shaped_set(win->popup->ecore_evas, 1);
     }
   
   win->popup->evas_win = ecore_evas_software_x11_window_get(win->ee);

   e_popup_layer_set(win->popup, 250);

   return win;
}

static int
_ngw_win_free(Ngw_Win *win)
{
   e_object_del(E_OBJECT(win->popup));

   free(win);
   return 1;
}

static Eina_Bool
_ngw_win_container_resize_cb(void *data, int ev_type, void *event_info)
{
   _ngw_win_position_calc(ngw_config->winlist_cfg->ng->win);
  
   return EINA_TRUE;
}

void
ngw_win_show(Ngw_Win *win)
{
   e_popup_show(win->popup);
}

void
ngw_win_hide(Ngw_Win *win)
{
   e_popup_hide(win->popup);
}

static void
_ngw_win_position_calc(Ngw_Win *win)
{
   Ng *ng = win->ng;

   E_Gadcon_Orient orient = (E_Gadcon_Orient) ng->cfg->orient;
   int size = 240;

   switch (orient)
     {
      case E_GADCON_ORIENT_FLOAT:
	 win->w = ng->zone->w;
	 win->h = size;
	 win->x = 0;
	 win->y = (ng->zone->h - size) / 2;
	 break;

      default:
	 break;
     }
   e_popup_move_resize(win->popup, win->x, win->y, win->w, win->h);
}


