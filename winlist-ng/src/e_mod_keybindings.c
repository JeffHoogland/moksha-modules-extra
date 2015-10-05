#include "e_mod_main.h"
#include "e_mod_keybindings.h"

#define NGW_WINLIST_ACTION_GROUP	 D_("NG Winlist")
#define NGW_WINLIST_ACTION_NEXT   D_("Switch To Next Window")
#define NGW_WINLIST_ACTION_PREV   D_("Switch To Previous Window")
#define NGW_WINLIST_ACTION_DESK_TOGGLE   D_("Show Windows From All Desks")
#define NGW_WINLIST_ACTION_ZONE_TOGGLE   D_("Show Windows From Other Zones")

#define ACT_FN_GO_KEY(act) \
   static void _e_actions_act_##act##_go_key(E_Object *obj, const char *params, Ecore_Event_Key *ev)

#define ACT_FN_GO_WHEEL(act) \
   static void _e_actions_act_##act##_go_wheel(E_Object *obj, const char *params, Ecore_Event_Mouse_Wheel *ev)

#define ACT_GO_KEY(name) \
   { \
      act = e_action_add(#name); \
      if (act) act->func.go_key = _e_actions_act_##name##_go_key; \
      if (act) act->func.go_wheel = _e_actions_act_##name##_go_wheel; \
   }

#define ACT_FN_GO(act) \
   static void _e_actions_act_##act##_go(E_Object *obj, const char *params)

#define ACT_GO(name) \
  { \
     act = e_action_add(#name); \
     if (act) act->func.go = _e_actions_act_##name##_go; \
  }

#define CFG_KEYBIND(_context, _key, _modifiers, _anymod, _action, _params) \
   eb = E_NEW(E_Config_Binding_Key, 1); \
   eb->context = _context; \
   eb->key = eina_stringshare_add(_key); \
   eb->modifiers = _modifiers; \
   eb->any_mod = _anymod; \
   eb->action = _action == NULL ? NULL : eina_stringshare_add(_action); \
   eb->params = _params == NULL ? NULL : eina_stringshare_add(_params)


/********* Private Declarations ***************/

ACT_FN_GO_KEY(ngw_winlist_next);
ACT_FN_GO_KEY(ngw_winlist_prev);
ACT_FN_GO_WHEEL(ngw_winlist_next);
ACT_FN_GO_WHEEL(ngw_winlist_prev);

static void _ngw_register_module_keybinding(E_Config_Binding_Key *key, const char *action);
void _ngw_unregister_module_keybinding(E_Config_Binding_Key *key, int save);



/*************** public **********************/
int
ngw_register_module_actions()
{
  E_Action *act;

  e_action_predef_name_set(NGW_WINLIST_ACTION_GROUP, NGW_WINLIST_ACTION_NEXT, NGW_WINLIST_NEXT_ACTION,
			   NULL, NULL, 0);
  e_action_predef_name_set(NGW_WINLIST_ACTION_GROUP, NGW_WINLIST_ACTION_PREV, NGW_WINLIST_PREV_ACTION,
			   NULL, NULL, 0);

  ACT_GO_KEY(ngw_winlist_next);
  ACT_GO_KEY(ngw_winlist_prev);

  return 1;
}
int
ngw_unregister_module_actions()
{
  e_action_del(NGW_WINLIST_NEXT_ACTION);
  e_action_del(NGW_WINLIST_PREV_ACTION);

  e_action_predef_name_del(NGW_WINLIST_ACTION_GROUP, NGW_WINLIST_ACTION_NEXT); 
  e_action_predef_name_del(NGW_WINLIST_ACTION_GROUP, NGW_WINLIST_ACTION_PREV);
  e_config_save();

  return 1;
}

int
ngw_register_module_keybindings()
{
  e_managers_keys_ungrab();

  _ngw_register_module_keybinding(&(ngw_config->winlist_next_key), NGW_WINLIST_NEXT_ACTION);

  _ngw_register_module_keybinding(&(ngw_config->winlist_prev_key), NGW_WINLIST_PREV_ACTION);

  e_managers_keys_grab();

  return 1;
}

int
ngw_unregister_module_keybindings()
{
  e_managers_keys_ungrab();

  _ngw_unregister_module_keybinding(&(ngw_config->winlist_next_key), 1);

  _ngw_unregister_module_keybinding(&(ngw_config->winlist_prev_key), 1);

  e_managers_keys_grab();

  return 1;
}

void
_ngw_unregister_module_keybinding(E_Config_Binding_Key *key, int save)
{
  Eina_List   *l;
  int done = 0, found = 0;
  E_Config_Binding_Key *eb;

  if (!key) return;

  while (!done)
    {
      done = 1;
      for (l = e_config->key_bindings; l; l = l->next)
	{
	  eb = (E_Config_Binding_Key*) l->data;

	  if (eb && eb->action && eb->action && 
	      !strcmp(!eb->action ? "" : eb->action, !key->action ? "" : key->action))
	    {
	      if (save)
		{ 
		  if (key->key) eina_stringshare_del(key->key);
		  if (key->params) eina_stringshare_del(key->params);

		  key->context    = eb->context;
		  key->key	       = eina_stringshare_add(eb->key);
		  key->modifiers  = eb->modifiers;
		  key->any_mod    = eb->any_mod;
		  key->params     = !eb->params ? NULL : eina_stringshare_add(eb->params);
		}

	      e_bindings_key_del(eb->context, eb->key, eb->modifiers, 
				 eb->any_mod, eb->action, eb->params);

	      if (eb->key) eina_stringshare_del(eb->key);
	      if (eb->action) eina_stringshare_del(eb->action);
	      if (eb->params) eina_stringshare_del(eb->params);
	      E_FREE(eb);

	      e_config->key_bindings = eina_list_remove_list(e_config->key_bindings, l);
	      found = 1;
	      done = 0;
	      break;
	    }
	}
    }

  if (!found)
    {
      //here we actually have to reset the key
      if (key->key) eina_stringshare_del(key->key);
      if (key->params) eina_stringshare_del(key->params);
      key->key = NULL;
      key->context = E_BINDING_CONTEXT_ANY;
      key->modifiers = E_BINDING_MODIFIER_NONE;
      key->any_mod = 0;
    }
}

static void
_ngw_register_module_keybinding(E_Config_Binding_Key *key, const char *action)
{
  int found;
  Eina_List   *l;
  E_Config_Binding_Key	*eb;

  if (!key || !key->key || !key->key[0] || !action) return;

  CFG_KEYBIND(key->context, key->key, key->modifiers, key->any_mod, 
	      action, key->params);

  for (l = e_config->key_bindings, found = 0; l && !found; l = l->next)
    {
      E_Config_Binding_Key *t = (E_Config_Binding_Key*) l->data; 
	
      if (!strcmp(!t->action ? "" : t->action, eb->action) &&
	  !strcmp(!t->params ? "" : t->params, !eb->params ? "" : eb->params)) 
	found = 1;
    }

  if (!found)
    {
      e_config->key_bindings = eina_list_append(e_config->key_bindings, eb); 
	
      e_bindings_key_add((E_Binding_Context) key->context, key->key,
                         (E_Binding_Modifier)key->modifiers, key->any_mod,
			 action, key->params);
    }
  else
    {
      if (eb->key) eina_stringshare_del(eb->key);
      if (eb->action) eina_stringshare_del(eb->action);
      if (eb->params) eina_stringshare_del(eb->params);
      E_FREE(eb);
    }
}


/*************** Private ********************/

ACT_FN_GO_KEY(ngw_winlist_next)
{
  ngw_winlist_modifiers_set(ev->modifiers);

  if(!ngw_winlist_show())
    {
      ngw_winlist_next();
    }
  
  return;
}

ACT_FN_GO_KEY(ngw_winlist_prev)
{
  ngw_winlist_modifiers_set(ev->modifiers);
  
  if(!ngw_winlist_show())
    ngw_winlist_prev();
 
  return;
}

ACT_FN_GO_WHEEL(ngw_winlist_next)
{
  ngw_winlist_modifiers_set(ev->modifiers);
   
  if(!ngw_winlist_show())
    {
      ngw_winlist_next();
    }
  
  return;
}

ACT_FN_GO_WHEEL(ngw_winlist_prev)
{
  ngw_winlist_modifiers_set(ev->modifiers);
   
  if(!ngw_winlist_show())
    ngw_winlist_prev();
 
  return;
}
