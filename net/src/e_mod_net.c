#include <e.h>
#include "e_mod_config.h"
#include "e_mod_main.h"
#include "e_mod_net.h"
#include "e_mod_configure.h"
#ifdef __FreeBSD__
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_mib.h>
#endif

static void _bytes_to_string(bytes_t bytes, char *string, int size);
static void _cb_post(void *data, E_Menu *m);
static void _cb_configure(void *data, E_Menu *m, E_Menu_Item *mi);

#ifdef __FreeBSD__
static int get_ifmib_general(int row, struct ifmibdata *ifmd); 

static int 
get_ifmib_general(int row, struct ifmibdata *ifmd) 
{ 
   int name[6]; 
   size_t len; 

   name[0] = CTL_NET; 
   name[1] = PF_LINK; 
   name[2] = NETLINK_GENERIC; 
   name[3] = IFMIB_IFDATA; 
   name[4] = row; 
   name[5] = IFDATA_GENERAL; 
   
   len = sizeof(*ifmd); 
   
   return sysctl(name, 6, ifmd, &len, (void *)0, 0);
}
#endif

Eina_Bool 
_net_cb_poll(void *data) 
{
   Instance *inst;
   char buf[256], popbuf[256], tmp[100];
   long bin, bout;
   bytes_t in, out = 0;

   inst = data;

#ifndef __FreeBSD__	
   FILE *f;
   char dev[64];
   bytes_t dummy = 0;
   int found = 0;
    
   f = fopen("/proc/net/dev", "r");
   if (!f) return EINA_TRUE;

   while (fgets(buf, 256, f))
     {
	int i = 0;

	for (; buf[i] != 0; i++)
	  if (buf[i] == ':') buf[i] = ' ';
	
	if (sscanf (buf, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu "
		    "%lu %lu %lu %lu\n", dev, &in, &dummy, &dummy,
		    &dummy, &dummy, &dummy, &dummy, &dummy, &out, &dummy,
		    &dummy, &dummy, &dummy, &dummy, &dummy, &dummy) < 17)
	  continue;

	if (!strcmp(dev, inst->ci->device)) found = 1;
	if (found) break;
     }
   fclose(f);
   if (!found) return EINA_TRUE;
#else
   struct ifmibdata *ifmd;
   int i, count, len;
   
   len = sizeof(count);
   sysctlbyname("net.link.generic.system.ifcount", &count, &len, (void *)0, 0);

   ifmd = malloc(sizeof(struct ifmibdata));
   for(i=1; i <= count; ++i)
     { 
	get_ifmib_general(i, ifmd);
	if (!strcmp(ifmd->ifmd_name, inst->ci->device)) break;
     }

   in = ifmd->ifmd_data.ifi_ibytes; 
   out = ifmd->ifmd_data.ifi_obytes;

   free(ifmd);
#endif

   bin = in - inst->in;
   bout = out - inst->out;
   bin = bin / 0.5;
   bout = bout / 0.5;
   inst->in = in;
   inst->out = out;

   if (bin <= inst->ci->limit)
     edje_object_signal_emit(inst->o_net, "e,state,receive,idle", "e");
   else
     edje_object_signal_emit(inst->o_net, "e,state,receive,active", "e");
   if (bout <= inst->ci->limit)
     edje_object_signal_emit(inst->o_net, "e,state,send,idle", "e");
   else
     edje_object_signal_emit(inst->o_net, "e,state,send,active", "e");

   _bytes_to_string(bin, tmp, sizeof(tmp));
   snprintf(buf, sizeof(buf), "%s/s", tmp);
   edje_object_part_text_set(inst->o_net, "e.text.recv", buf);
   if (inst->popup) 
     {
	_bytes_to_string(in, tmp, sizeof(tmp));
	snprintf(popbuf, sizeof(popbuf), "Rx: %s", tmp);
	edje_object_part_text_set(inst->pop_bg, "e.text.recv", popbuf);
     }
   
   _bytes_to_string(bout, tmp, sizeof(tmp));
   snprintf(buf, sizeof(buf), "%s/s", tmp);
   edje_object_part_text_set(inst->o_net, "e.text.send", buf);
   if (inst->popup) 
     {
	_bytes_to_string(out, tmp, sizeof(tmp));
	snprintf(popbuf, sizeof(popbuf), "Tx: %s", tmp);
	edje_object_part_text_set(inst->pop_bg, "e.text.send", popbuf);
     }
   
   return EINA_TRUE;
}

void 
_net_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event) 
{
   Instance *inst;
   Ecore_Exe *x;
   Evas_Event_Mouse_Down *ev;
   
   inst = data;
   ev = event;
   if ((ev->button == 1) && (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)) 
     {
	if (inst->ci->app) 
	  {
	     x = ecore_exe_run(inst->ci->app, NULL);
	     if (x) ecore_exe_free(x);
	  }
     }
   else if ((ev->button == 1) && (!net_cfg->menu))
     e_gadcon_popup_toggle_pinned(inst->popup);
   else if ((ev->button == 3) && (!net_cfg->menu)) 
     {
	E_Menu *m;
	E_Menu_Item *mi;
	int x, y;
	
	m = e_menu_new();
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, D_("Settings"));
	e_util_menu_item_theme_icon_set(mi, "preferences-system");
	e_menu_item_callback_set(mi, _cb_configure, inst);
	
	m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
	
	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, 
					  NULL, NULL);
	e_menu_activate_mouse(m, 
			      e_util_zone_current_get(e_manager_current_get()),
			      x + ev->output.x, y + ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
     }
}

void 
_net_cb_mouse_in(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Instance *inst;
   Evas_Object *bg;
   char buf[PATH_MAX], tmp[100];

   inst = data;
   if (inst->popup) return;
   if (!inst->ci->show_popup) return;
   
   inst->popup = e_gadcon_popup_new(inst->gcc);
   snprintf(buf, sizeof(buf), "%s/net.edj", e_module_dir_get(net_cfg->mod));
   
   bg = edje_object_add(inst->popup->win->evas);
   if (!e_theme_edje_object_set(bg, "base/theme/modules", 
				"modules/net/popup"))
     edje_object_file_set(bg, buf, "modules/net/popup");
   snprintf(buf, sizeof(buf), D_("Device - %s"), inst->ci->device);
   edje_object_part_text_set(bg, "e.text.title", buf);
   inst->pop_bg = bg;

   _bytes_to_string(inst->in, tmp, sizeof(tmp));
   snprintf(buf, sizeof(buf), "Rx: %s", tmp);
   edje_object_part_text_set(bg, "e.text.recv", buf);

   _bytes_to_string(inst->out, tmp, sizeof(tmp));
   snprintf(buf, sizeof(buf), "Tx: %s", tmp);
   edje_object_part_text_set(bg, "e.text.send", buf);
   
   e_gadcon_popup_content_set(inst->popup, bg);
   e_gadcon_popup_show(inst->popup);
}

void 
_net_cb_mouse_out(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Instance *inst;
   
   if (!(inst = data)) return;
   if ((!inst->popup) || (inst->popup->pinned)) return;
   e_object_del(E_OBJECT(inst->popup));
   inst->popup = NULL;
}

static void
_bytes_to_string(bytes_t bytes, char *string, int size)
{
   if (bytes > 1048576)
     snprintf(string, size, "%.2f MB", ((float) bytes / 1048576));
   else if ((bytes > 1024) && (bytes < 1048576))
     snprintf(string, size, "%lu KB", (bytes / 1024));
   else
     snprintf(string, size, "%lu B", bytes);
}

static void 
_cb_post(void *data, E_Menu *m) 
{
   if (!net_cfg->menu) return;
   e_menu_post_deactivate_callback_set(net_cfg->menu, NULL, NULL);
   e_object_del(E_OBJECT(net_cfg->menu));
   net_cfg->menu = NULL;
}

static void 
_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi) 
{
   Instance *inst;
   
   inst = data;
   _configure_net_module(inst->ci);
}
