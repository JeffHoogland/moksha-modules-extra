#include "e_mod_main.h"
#include "echievements.x"

#define ECH_INIT(NAME)     EINTERN void echievement_init_cb_##NAME(Echievement *ec)
#define ECH_EH_NAME(NAME, TYPE)  echievement_##NAME##_##TYPE##_handler_cb
#define ECH_EH(NAME, TYPE) static Eina_Bool ECH_EH_NAME(NAME, TYPE)(Echievement *ec, int type EINA_UNUSED, void *event)
#define ECH_EH_UNUSED(NAME, TYPE) static Eina_Bool ECH_EH_NAME(NAME, TYPE)(Echievement *ec, int type EINA_UNUSED, void *event EINA_UNUSED)
#define ECH_EH_ADD(NAME, TYPE) \
   do { \
      if (etrophy_trophy_earned_get(ec->trophy)) break; \
      E_LIST_HANDLER_APPEND(ec->handlers, TYPE, ECH_EH_NAME(NAME, TYPE), ec); \
   } while (0)

#define ECH_BH_NAME(NAME, TYPE)  echievement_##NAME##_border_hook_##TYPE
#define ECH_BH(NAME, TYPE) static void ECH_BH_NAME(NAME, TYPE)(Echievement *ec, E_Border *bd)
#define ECH_BH_ADD(NAME, TYPE) \
   do { \
      if (etrophy_trophy_earned_get(ec->trophy)) break; \
      ec->bh_handlers = 1; \
      ec->handlers = eina_list_append(ec->handlers, \
        e_border_hook_add(E_BORDER_HOOK_##TYPE, (Ecore_End_Cb)ECH_BH_NAME(NAME, TYPE), ec)); \
   } while (0)

#define ECH_MH_NAME(NAME) _ech_##NAME##_mouse_hook
#define ECH_MH(NAME) static void ECH_MH_NAME(NAME)(Echievement *ec)
#define ECH_MH_ADD(NAME) \
   do { \
      ec->mouse_hook = (Ecore_Cb)ECH_MH_NAME(NAME); \
      if (!etrophy_trophy_earned_get(ec->trophy)) \
        mod->mouse.hooks = eina_list_append(mod->mouse.hooks, ec); \
   } while (0)
#define ECH_MH_DEL \
   do { \
     ec->mouse_hook = NULL; \
     mod->mouse.hooks = eina_list_remove(mod->mouse.hooks, ec); \
   } while (0)

static Ecore_Idler *_ech_idler = NULL;

/* helper for init */
static void
_ech_list_add(Etrophy_Trophy *et)
{
   Echievement *ec;

   ec = E_NEW(Echievement, 1);
   ec->trophy = et;
   ec->id = eina_hash_population(mod->trophies);
   eina_hash_add(mod->trophies, &ec->id, ec);
   mod->trophies_list = (Echievement *)eina_inlist_append(EINA_INLIST_GET(mod->trophies_list), EINA_INLIST_GET(ec));
}

static void
_ech_add(Echievement_Id id)
{
   Etrophy_Trophy *et;

   et = etrophy_trophy_new(Echievement_Strings[id], Echievement_Descs[id],
                           Echievement_Hide_States[id], Echievement_Goals[id], Echievement_Points[id]);
   etrophy_gamescore_trophy_add(ech_config->gs, et);
   _ech_list_add(et);
}

static void
_ech_free(Echievement *ec)
{
   /* trophy does NOT get freed here!!! */
   if (ec->bh_handlers)
     E_FREE_LIST(ec->handlers, e_border_hook_del);
   else
     E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   if (ec->mouse_hook) mod->mouse.hooks = eina_list_remove(mod->mouse.hooks, ec);
   if (ec->dialog.icon)
     evas_object_event_callback_del_full(ec->dialog.icon, EVAS_CALLBACK_DEL, mod->obj_del_cb, ec);
   if (ec->dialog.label)
     evas_object_event_callback_del_full(ec->dialog.label, EVAS_CALLBACK_DEL, mod->obj_del_cb, ec);
   free(ec);
}

static Etrophy_Trophy *
_ech_lookup(Echievement_Id id)
{
   Echievement *ec;

   ec = eina_hash_find(mod->trophies, &id);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ec, NULL);
   return ec->trophy;
}

static void
_ech_notify(const char *name, const char *description)
{
   E_Notification *n;
   char summary[256];

   snprintf(summary, sizeof(summary), D_("Echievement: %s"), D_(name));
   n = e_notification_full_new("echievements", 0, PACKAGE_DATA_DIR "/trophy.png",
                               summary, D_(description), -1);
   e_notification_hint_urgency_set(n, E_NOTIFICATION_URGENCY_NORMAL);
   e_notification_send(n, NULL, NULL);
   e_notification_unref(n);
}

static void
_ech_trophy_counter_increment(Echievement *ec, unsigned int value)
{
   if (etrophy_trophy_earned_get(ec->trophy)) return;
   etrophy_trophy_counter_increment(ec->trophy, value);
   ech_cfg_ech_update(ec);
}

static void
_ech_trophy_counter_set(Echievement *ec, unsigned int value)
{
   unsigned int count;

   if (etrophy_trophy_earned_get(ec->trophy)) return;
   etrophy_trophy_goal_get(ec->trophy, NULL, &count);
   if (value == count) return;
   etrophy_trophy_counter_set(ec->trophy, value);
   ech_cfg_ech_update(ec);
}

/* call whenever new echievement is earned, ec may be NULL */
static void
_ech_hook(Echievement_Id id, Echievement *ec)
{
   Etrophy_Trophy *et;

   if (ec)
     et = ec->trophy;
   else
     et = _ech_lookup(id);
   if (!et) return;
   if (ec)
     _ech_trophy_counter_set(ec, Echievement_Goals[id]);
   else //FIXME?
     etrophy_trophy_counter_set(et, Echievement_Goals[id]);
   INF("TROPHY EARNED: %s - %s",
       etrophy_trophy_name_get(et),
       etrophy_trophy_description_get(et));
   _ech_notify(etrophy_trophy_name_get(et),
               etrophy_trophy_description_get(et));
   if (ec) ech_cfg_ech_add(ec);
}

static Eina_Bool
_ech_init_check_idler(void *d EINA_UNUSED)
{
   Echievement *ec;
   unsigned int count = 0;
   Eina_Inlist *l;

   l = mod->itr ? : EINA_INLIST_GET(mod->trophies_list);
   EINA_INLIST_FOREACH(l, ec)
     {
        if (count > 5)
          {
             mod->itr = EINA_INLIST_GET(ec);
             return EINA_TRUE;
          }
        if (etrophy_trophy_earned_get(ec->trophy)) continue;
        if (!Echievement_Callbacks[ec->id]) continue;
        Echievement_Callbacks[ec->id](ec);
        count++;
     }
   _ech_idler = ecore_idler_del(_ech_idler);
   return EINA_FALSE;
}

static Eina_Bool
_ech_init_add_idler(void *d EINA_UNUSED)
{
   Echievement_Id id = 0;
   unsigned int count = 0;

   if (mod->trophies_list)
     {
        Echievement *ec;

        ec = EINA_INLIST_CONTAINER_GET(EINA_INLIST_GET(mod->trophies_list)->last, Echievement);
        id = ec->id + 1;
     }
   for (; id < ECH(LAST); count++, id++)
     {
        if (count > 20) return EINA_TRUE;
        _ech_add(id);
     }
   ecore_idler_del(_ech_idler);
   _ech_idler = ecore_idler_add(_ech_init_check_idler, NULL);
   return EINA_FALSE;
}

static Eina_Bool
NOT_SO_INCOGNITO_helper(const char *str)
{
   unsigned int x;

   if ((!str) || (!str[0])) return EINA_FALSE;
   for (x = 0; Echievement_NOT_SO_INCOGNITO_Strings[x]; x++)
     if (!strncmp(str, Echievement_NOT_SO_INCOGNITO_Strings[x], strlen(Echievement_NOT_SO_INCOGNITO_Strings[x])))
       return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
_ech_PERSISTENT_timer_cb(Echievement *ec)
{
   Eina_List *l;
   E_Border *bd;
   char buf[128];

   _ech_trophy_counter_increment(ec, 1);
   if (!etrophy_trophy_earned_get(ec->trophy)) return EINA_TRUE;
   _ech_hook(ec->id, ec);
   snprintf(buf, sizeof(buf), "PERSISTENT%u", Echievement_Goals[ec->id]);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   E_FREE_LIST(ec->data, ecore_timer_del);
   EINA_LIST_FOREACH(e_border_client_list(), l, bd)
     evas_object_data_del(bd->bg_object, buf);
   return EINA_FALSE;
}

///////////////////////////////////////////////////////////////////////////////////

/* Echievement event handler callbacks:
 *
 * appended in related ECH_INIT, data param is the echievement
 *
 * check conditions, increment/set counter, delete handlers if trophy acquired
 */

ECH_EH(PERSISTENT, E_EVENT_BORDER_REMOVE)
{
   E_Event_Border_Remove *ev = event;
   char buf[128];
   Ecore_Timer *timer;

   snprintf(buf, sizeof(buf), "PERSISTENT%u", Echievement_Goals[ec->id]);
   timer = evas_object_data_del(ev->border->bg_object, buf);
   if (ec->data) ec->data = eina_list_remove(ec->data, timer);
   if (timer) ecore_timer_del(timer);
   return ECORE_CALLBACK_RENEW;
}

ECH_EH(PERSISTENT, E_EVENT_BORDER_ADD)
{
   E_Event_Border_Add *ev = event;
   char buf[128];

   snprintf(buf, sizeof(buf), "PERSISTENT%u", Echievement_Goals[ec->id]);
   ec->data = eina_list_append(ec->data,
     ecore_timer_add(Echievement_Goals[ec->id] * 60 * 60, (Ecore_Task_Cb)_ech_PERSISTENT_timer_cb, ec));
   /* don't try this at home, kids! */
   evas_object_data_set(ev->border->bg_object, buf, eina_list_last_data_get(ec->data));
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(NOTHING_ELSE_MATTERS, E_EVENT_SHELF_DEL)
{
   if (eina_list_count(e_shelf_list()))
     return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(CAVE_DWELLER, E_EVENT_BACKLIGHT_CHANGE)
{
   if (e_backlight_level_get(e_util_zone_current_get(e_manager_current_get())) >= 1.)
     return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(FEAR_OF_THE_DARK, E_EVENT_BACKLIGHT_CHANGE)
{
   if (e_backlight_level_get(e_util_zone_current_get(e_manager_current_get())) <= 99.)
     return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH(DUALIST, E_EVENT_ZONE_ADD)
{
   E_Event_Zone_Add *ev = event;
   _ech_trophy_counter_set(ec, eina_list_count(ev->zone->container->zones));
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(SHELF_POSITIONS, E_EVENT_SHELF_ADD)
{
   _ech_trophy_counter_set(ec, eina_list_count(e_shelf_list()));
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(GOING_HD, E_EVENT_ZONE_ADD)
{
   E_Screen *es;
   const Eina_List *l;

   EINA_LIST_FOREACH(e_xinerama_screens_get(), l, es)
     _ech_trophy_counter_set(ec, es->w * es->h);
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(REAL_ESTATE_MOGUL, E_EVENT_ZONE_ADD)
{
   E_Screen *es;
   const Eina_List *l;
   unsigned int geom = 0;

   EINA_LIST_FOREACH(e_xinerama_screens_get(), l, es)
     geom += (es->w * es->h);
   _ech_trophy_counter_set(ec, geom);
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(MAXIMUM_DEFINITION, E_EVENT_ZONE_ADD)
{
   E_Screen *es;
   const Eina_List *l;
   unsigned int geom = 0;

   EINA_LIST_FOREACH(e_xinerama_screens_get(), l, es)
     geom += ((es->w * es->h) >= (1920 * 1080));
   _ech_trophy_counter_set(ec, geom);
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(EDGY, E_EVENT_MANAGER_KEYS_GRAB)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_edge());
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(SLEEPER, E_EVENT_MANAGER_KEYS_GRAB)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_acpi());
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(SIGNALLER, E_EVENT_MANAGER_KEYS_GRAB)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_signal());
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(WHEELY, E_EVENT_MANAGER_KEYS_GRAB)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_wheel());
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(CLICKER, E_EVENT_MANAGER_KEYS_GRAB)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_mouse());
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(KEYBOARD_USER, E_EVENT_MANAGER_KEYS_GRAB)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_keys());
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(GADGETEER, E_EVENT_GADCON_CLIENT_ADD)
{
   E_Config_Gadcon *cf_gc;
   Eina_List *l;
   unsigned int gadgets = 0;

   /* not gonna fuck with trying to keep an accurate count here,
    * gadman/gadcon is too shitty
    */
   EINA_LIST_FOREACH(e_config->gadcons, l, cf_gc)
     if (!e_util_strcmp(cf_gc->name, "gadman"))
       gadgets += eina_list_count(cf_gc->clients);
   _ech_trophy_counter_set(ec, gadgets);
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH(PHYSICIST, E_EVENT_MODULE_UPDATE)
{
   E_Event_Module_Update *ev = event;

   if ((!ev->enabled) || e_util_strcmp(ev->name, "Physics")) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(LIFE_ON_THE_EDGE, E_EVENT_MODULE_UPDATE)
{
   const char *env;

   env = getenv("E17_TAINTED");
   if (e_util_strcmp(env, "YES")) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH(QUICKDRAW, E_EVENT_MODULE_UPDATE)
{
   E_Event_Module_Update *ev = event;
   if ((!ev->enabled) || e_util_strcmp(ev->name, "Quickaccess")) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   return ECORE_CALLBACK_RENEW;
}

ECH_EH(TILED, E_EVENT_MODULE_UPDATE)
{
   E_Event_Module_Update *ev = event;
   if ((!ev->enabled) || e_util_strcmp(ev->name, "Tiling")) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   return ECORE_CALLBACK_RENEW;
}

ECH_EH(OPAQUE, E_EVENT_MODULE_UPDATE)
{
   E_Event_Module_Update *ev = event;
   if ((ev->enabled) || e_util_strcmp(ev->name, "Composite"))
     return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   return ECORE_CALLBACK_RENEW;
}

ECH_EH_UNUSED(OPAQUE, E_EVENT_MODULE_INIT_END)
{
   if (e_module_find("Composite")) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   (void)event;
   return ECORE_CALLBACK_RENEW;
}

ECH_EH(SECURITY_CONSCIOUS, E_EVENT_DESKLOCK)
{
   unsigned int count;
   E_Event_Desklock *ev = event;

   if (ev->on) return ECORE_CALLBACK_RENEW;
   _ech_trophy_counter_set(ec,  eina_list_count(mod->desklock.timers));
   if (!etrophy_trophy_earned_get(ec->trophy)) return ECORE_CALLBACK_RENEW;
   _ech_hook(ec->id, ec);
   etrophy_trophy_goal_get(ec->trophy, &count, NULL);
   if (count == Echievement_Goals[ECH(CHIEF_OF_SECURITY)])
     E_FREE_LIST(mod->desklock.timers, ecore_timer_del);
   E_FREE_LIST(ec->handlers, ecore_event_handler_del);
   return ECORE_CALLBACK_RENEW;
}

///////////////////////////////////////////////////////////////////////////////////

/* Echievement mouse hook callbacks:
 *
 * called every time the mouse moves
 *
 * mod->mouse.dx/dy are change since last call
 */

ECH_MH(MOUSE_RUNNER)
{
   _ech_trophy_counter_increment(ec,  abs(mod->mouse.dx) + abs(mod->mouse.dy));
   if (!etrophy_trophy_earned_get(ec->trophy)) return;
   _ech_hook(ec->id, ec);
   ECH_MH_DEL;
}

ECH_MH(WINDOW_HAULER)
{
   _ech_trophy_counter_increment(ec,  abs(mod->mouse.dx) + abs(mod->mouse.dy));
   if (!etrophy_trophy_earned_get(ec->trophy)) return;
   _ech_hook(ec->id, ec);
   ECH_MH_DEL;
   E_FREE_LIST(ec->handlers, e_border_hook_del);
}

///////////////////////////////////////////////////////////////////////////////////

/* Echievement border hook callbacks */

ECH_BH(WINDOW_ENTHUSIAST, NEW_BORDER)
{
   /* ignore all windows added before modules are loaded:
    * these are previously-placed windows
    */
   if (!mod->module_init_end) return;

   _ech_trophy_counter_increment(ec,  1);
   if (!etrophy_trophy_earned_get(ec->trophy))
     return;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, e_border_hook_del);
   (void)bd;
}

ECH_BH(WINDOW_HAULER, MOVE_BEGIN)
{
   ECH_MH_ADD(WINDOW_HAULER);
   (void)bd;
}

ECH_BH(TERMINOLOGIST, EVAL_PRE_POST_FETCH)
{
   if (e_util_strcmp(bd->client.icccm.class, "terminology")) return;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, e_border_hook_del);
}

ECH_BH(NOT_SO_INCOGNITO, EVAL_PRE_POST_FETCH)
{
   if ((!NOT_SO_INCOGNITO_helper(bd->client.icccm.title)) && (!NOT_SO_INCOGNITO_helper(bd->client.netwm.name)))
     return;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, e_border_hook_del);
}

ECH_BH(WINDOW_HAULER, MOVE_END)
{
   ECH_MH_DEL;
   (void)bd;
}

ECH_BH(WINDOW_MOVER, MOVE_END)
{
   _ech_trophy_counter_increment(ec,  1);
   if (!etrophy_trophy_earned_get(ec->trophy)) return;
   _ech_hook(ec->id, ec);
   E_FREE_LIST(ec->handlers, e_border_hook_del);
   (void)bd;
}

///////////////////////////////////////////////////////////////////////////////////

/* Echievement init callbacks:
 *
 * called every time the module loads upon loading a trophy if the trophy has not been acquired
 *
 * ** check trophy status
 *  a. if goal met, hook echievement
 *  b. else add handlers
 */

ECH_INIT(NOTHING_ELSE_MATTERS)
{
   if (!eina_list_count(e_shelf_list()))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(NOTHING_ELSE_MATTERS, E_EVENT_SHELF_DEL);
}

ECH_INIT(FEAR_OF_THE_DARK)
{
   if (e_backlight_exists() && (e_backlight_level_get(e_util_zone_current_get(e_manager_current_get())) > 99.))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(FEAR_OF_THE_DARK, E_EVENT_BACKLIGHT_CHANGE);
}

ECH_INIT(CAVE_DWELLER)
{
   if (e_backlight_exists() && (e_backlight_level_get(e_util_zone_current_get(e_manager_current_get())) < 1.))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(CAVE_DWELLER, E_EVENT_BACKLIGHT_CHANGE);
}

ECH_INIT(WINDOW_ENTHUSIAST)
{
   /* only count windows opened while e is running to prevent cheating
    * reuse same handler
    */
   ECH_BH_ADD(WINDOW_ENTHUSIAST, NEW_BORDER);
}

ECH_INIT(WINDOW_MOVER)
{
   /* only count windows moved while e is running to prevent cheating */
   ECH_BH_ADD(WINDOW_MOVER, MOVE_END);
}

ECH_INIT(WINDOW_HAULER)
{
   ECH_BH_ADD(WINDOW_HAULER, MOVE_BEGIN);
   ECH_BH_ADD(WINDOW_HAULER, MOVE_END);
}

ECH_INIT(EDGY)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_edge());
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(EDGY, E_EVENT_MANAGER_KEYS_GRAB);
}

ECH_INIT(SLEEPER)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_acpi());
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(SLEEPER, E_EVENT_MANAGER_KEYS_GRAB);
}

ECH_INIT(SIGNALLER)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_signal());
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(SIGNALLER, E_EVENT_MANAGER_KEYS_GRAB);
}

ECH_INIT(WHEELY)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_wheel());
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(WHEELY, E_EVENT_MANAGER_KEYS_GRAB);
}

ECH_INIT(CLICKER)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_mouse());
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(CLICKER, E_EVENT_MANAGER_KEYS_GRAB);
}

ECH_INIT(KEYBOARD_USER)
{
   _ech_trophy_counter_set(ec, ech_bindings_check_keys());
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(KEYBOARD_USER, E_EVENT_MANAGER_KEYS_GRAB);
}

ECH_INIT(TERMINOLOGIST)
{
   Eina_List *l;
   E_Border *bd;

   EINA_LIST_FOREACH(e_border_client_list(), l, bd)
     if (!e_util_strcmp(bd->client.icccm.class, "terminology"))
       _ech_hook(ec->id, ec);
   if (!etrophy_trophy_earned_get(ec->trophy))
     ECH_BH_ADD(TERMINOLOGIST, EVAL_PRE_POST_FETCH);
}

ECH_INIT(PHYSICIST)
{
   if (e_module_find("Physics"))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(PHYSICIST, E_EVENT_MODULE_UPDATE);
}

ECH_INIT(QUICKDRAW)
{
   if (e_module_find("Quickaccess"))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(QUICKDRAW, E_EVENT_MODULE_UPDATE);
}

ECH_INIT(TILED)
{
   if (e_module_find("Tiling"))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(TILED, E_EVENT_MODULE_UPDATE);
}

ECH_INIT(LIFE_ON_THE_EDGE)
{
   const char *env;

   env = getenv("E17_TAINTED");
   if (!e_util_strcmp(env, "YES"))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(LIFE_ON_THE_EDGE, E_EVENT_MODULE_UPDATE);
}

ECH_INIT(OPAQUE)
{
   ECH_EH_ADD(OPAQUE, E_EVENT_MODULE_UPDATE);
   ECH_EH_ADD(OPAQUE, E_EVENT_MODULE_INIT_END);
}

ECH_INIT(DUALIST)
{
   E_Container *con;

   con = e_util_container_current_get();
   if (con)
     _ech_trophy_counter_set(ec, eina_list_count(con->zones));
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(DUALIST, E_EVENT_ZONE_ADD);
}

ECH_INIT(SHELF_POSITIONS)
{
   _ech_trophy_counter_set(ec, eina_list_count(e_shelf_list()));
   if (etrophy_trophy_earned_get(ec->trophy))
     /* number of shelves equals goal, grant trophy and return */
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(SHELF_POSITIONS, E_EVENT_SHELF_ADD);
}

ECH_INIT(GOING_HD)
{
   E_Screen *es;
   const Eina_List *l;

   EINA_LIST_FOREACH(e_xinerama_screens_get(), l, es)
     _ech_trophy_counter_set(ec, es->w * es->h);
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(GOING_HD, E_EVENT_ZONE_ADD);
}

ECH_INIT(REAL_ESTATE_MOGUL)
{
   E_Screen *es;
   const Eina_List *l;
   unsigned int geom = 0;

   EINA_LIST_FOREACH(e_xinerama_screens_get(), l, es)
     geom += (es->w * es->h);
   _ech_trophy_counter_set(ec, geom);
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(REAL_ESTATE_MOGUL, E_EVENT_ZONE_ADD);
}

ECH_INIT(MAXIMUM_DEFINITION)
{
   E_Screen *es;
   const Eina_List *l;
   unsigned int geom = 0;

   EINA_LIST_FOREACH(e_xinerama_screens_get(), l, es)
     geom += ((es->w * es->h) >= (1920 * 1080));
   _ech_trophy_counter_set(ec, geom);
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(MAXIMUM_DEFINITION, E_EVENT_ZONE_ADD);
}

ECH_INIT(NOT_SO_INCOGNITO)
{
   Eina_List *l;
   E_Border *bd;

   EINA_LIST_FOREACH(e_border_client_list(), l, bd)
     {
        if (NOT_SO_INCOGNITO_helper(bd->client.icccm.title) || NOT_SO_INCOGNITO_helper(bd->client.netwm.name))
          {
             _ech_hook(ec->id, ec);
             return;
          }
     }
   ECH_BH_ADD(NOT_SO_INCOGNITO, EVAL_PRE_POST_FETCH);
}

ECH_INIT(BILINGUAL)
{
   Eina_List *l;

   // this is very expensive, so don't do it if we don't have to
   if (ec->id == ECH(POLYGLOT))
     if (!etrophy_trophy_earned_get(_ech_lookup(ECH(BILINGUAL)))) return;
   l = ech_language_enumerate();
   _ech_trophy_counter_set(ec, eina_list_count(l));
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   eina_list_free(l);
}

ECH_INIT(GADGETEER)
{
   E_Config_Gadcon *cf_gc;
   Eina_List *l;
   unsigned int gadgets = 0;

   EINA_LIST_FOREACH(e_config->gadcons, l, cf_gc)
     if (!e_util_strcmp(cf_gc->name, "gadman"))
       gadgets += eina_list_count(cf_gc->clients);
   _ech_trophy_counter_set(ec, gadgets);
   if (etrophy_trophy_earned_get(ec->trophy))
     _ech_hook(ec->id, ec);
   else
     ECH_EH_ADD(GADGETEER, E_EVENT_GADCON_CLIENT_ADD);
}

ECH_INIT(PERSISTENT)
{
   Eina_List *l;
   E_Border *bd;
   char buf[128];

   snprintf(buf, sizeof(buf), "PERSISTENT%u", Echievement_Goals[ec->id]);
   EINA_LIST_FOREACH(e_border_client_list(), l, bd)
     {
        ec->data = eina_list_append(ec->data,
          ecore_timer_add(60 * 60, (Ecore_Task_Cb)_ech_PERSISTENT_timer_cb, ec));
        /* don't try this at home, kids! */
        evas_object_data_set(bd->bg_object, buf, eina_list_last_data_get(ec->data));
     }
   ECH_EH_ADD(PERSISTENT, E_EVENT_BORDER_ADD);
   ECH_EH_ADD(PERSISTENT, E_EVENT_BORDER_REMOVE);
}

ECH_INIT(SECURITY_CONSCIOUS)
{
   ECH_EH_ADD(SECURITY_CONSCIOUS, E_EVENT_DESKLOCK);
}

ECH_INIT(MOUSE_RUNNER)
{
   ECH_MH_ADD(MOUSE_RUNNER);
}

void
ech_init(void)
{
   /* FIXME: delay this */
   mod->trophies = eina_hash_int32_new((Eina_Free_Cb)_ech_free);
   if (ech_config->gs)
     {
        const Eina_List *l;

        l = etrophy_gamescore_trophies_list_get(ech_config->gs);
        if (l)
          {
             E_LIST_FOREACH(l, _ech_list_add);
             _ech_idler = ecore_idler_add(_ech_init_check_idler, NULL);
             return;
          }
     }
   _ech_idler = ecore_idler_add(_ech_init_add_idler, NULL);
}

void
ech_shutdown(void)
{
   eina_hash_free(mod->trophies);
   mod->trophies = NULL;
   mod->trophies_list = NULL;
   mod->itr = NULL;
   if (_ech_idler) _ech_idler = ecore_idler_del(_ech_idler);
}

