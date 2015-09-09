#include <e.h>
#include "e_mod_config.h"
#include "e_mod_main.h"
#include "e_mod_gadcon.h"
#include "e_mod_net.h"

static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "net", 
     {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, NULL},
   E_GADCON_CLIENT_STYLE_PLAIN
};
  
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   Instance *inst;
   E_Gadcon_Client *gcc;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/net.edj", e_module_dir_get(net_cfg->mod));

   inst = E_NEW(Instance, 1);
   inst->ci = _net_config_item_get(id);
 
   inst->o_net = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(inst->o_net, "base/theme/modules/net",
				"modules/net/main"))
     edje_object_file_set(inst->o_net, buf, "modules/net/main");
   edje_object_signal_callback_add(inst->o_net, "e,action,mouse,in", "",
				   _net_cb_mouse_in, inst);
   edje_object_signal_callback_add(inst->o_net, "e,action,mouse,out", "",
				   _net_cb_mouse_out, inst);
   evas_object_show(inst->o_net);

   if (!inst->ci->show_text)
     edje_object_signal_emit(inst->o_net, "e,state,text,hide", "e");
   else
     edje_object_signal_emit(inst->o_net, "e,state,text,show", "e");
   
   gcc = e_gadcon_client_new(gc, name, id, style, inst->o_net);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->timer = ecore_timer_add(0.5, _net_cb_poll, inst);

   evas_object_event_callback_add(inst->o_net, EVAS_CALLBACK_MOUSE_DOWN, 
				  _net_cb_mouse_down, inst);

   net_cfg->instances = eina_list_append(net_cfg->instances, inst);
   return gcc;
}

static void 
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst;
   
   inst = gcc->data;
   net_cfg->instances = eina_list_remove(net_cfg->instances, inst);
   if (inst->timer) ecore_timer_del(inst->timer);
   if (inst->o_net) 
     {
	evas_object_event_callback_del(inst->o_net, EVAS_CALLBACK_MOUSE_DOWN,
				       _net_cb_mouse_down);
	edje_object_signal_callback_del(inst->o_net, "e,action,mouse,in", "",
					_net_cb_mouse_in);
	edje_object_signal_callback_del(inst->o_net, "e,action,mouse,out", "",
					_net_cb_mouse_out);
	evas_object_del(inst->o_net);
     }
   E_FREE(inst);
}

static void 
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient) 
{
   e_gadcon_client_aspect_set(gcc, 32, 16);
   e_gadcon_client_min_size_set(gcc, 32, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class) 
{
   return D_("Net");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas) 
{
   Evas_Object *o;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/e-module-net.edj", 
            e_module_dir_get(net_cfg->mod));
   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   Config_Item *ci;

   ci = _net_config_item_get(NULL);
   return ci->id;
}

void 
_net_gc_register(void) 
{
   e_gadcon_provider_register(&_gc_class);
}

void 
_net_gc_unregister(void) 
{
   e_gadcon_provider_unregister(&_gc_class);
}

const char *
_net_gc_name(void)
{
   return _gc_class.name;
}
