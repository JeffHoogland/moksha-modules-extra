#include <e.h>
#include "e_mod_config.h"
#include "e_mod_main.h"
#include "e_mod_net.h"
#include "e_mod_gadcon.h"

Config_Item *
_net_config_item_get(const char *id) 
{
   Eina_List *l;
   Config_Item *ci;
   char buf[128];

   if (!id)
     {
	int  num = 0;

	/* Create id */
	if (net_cfg->items)
	  {
	     const char *p;
	     ci = eina_list_last(net_cfg->items)->data;
	     p = strrchr(ci->id, '.');
	     if (p) num = atoi(p + 1) + 1;
	  }
	snprintf(buf, sizeof(buf), "%s.%d", _net_gc_name(), num);
	id = buf;
     }
   else
     {
	for (l = net_cfg->items; l; l = l->next) 
	  {
	     ci = l->data;
	     if (!ci->id) continue;
	     if (!strcmp(ci->id, id)) return ci;
	  }
     }
   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->device = eina_stringshare_add("eth0");
   ci->app = eina_stringshare_add("");
   ci->limit = 0;
   ci->show_text = 1;
   ci->show_popup = 0;
   net_cfg->items = eina_list_append(net_cfg->items, ci);
   return ci;
}

Eina_List *
_net_config_devices_get(void) 
{
   Eina_List *devs = NULL;
#ifndef __FreeBSD__
   FILE *f;
   char buf[256];
   char dev[64];
   unsigned long dummy;
   
   f = fopen("/proc/net/dev", "r");
   if (!f) return NULL;

   while (fgets(buf, 256, f)) 
     {
	int i = 0;
	
	for (; buf[i] != 0; i++)
	  if (buf[i] == ':') buf[i] = ' ';

	if (sscanf (buf, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu "
		    "%lu %lu %lu %lu\n", dev, &dummy, &dummy, &dummy,
		    &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy,
		    &dummy, &dummy, &dummy, &dummy, &dummy, &dummy) < 17)
	  continue;
	devs = eina_list_append(devs, strdup(dev));
     }
   fclose(f);
#else
   DIR *d = NULL;
   struct dirent *dentry = NULL;

   d = opendir("/dev/net");
   if (!d) return NULL;

   while ((dentry = readdir(d))) 
     {
	if (!strstr(dentry->d_name, "."))     
	  devs = eina_list_append(devs, strdup(dentry->d_name));
     } 
   closedir(d);
#endif

   return devs;
}

void 
_net_config_updated(Config_Item *ci)
{
   Eina_List *l;
   
   if (!net_cfg) return;
   for (l = net_cfg->instances; l; l = l->next) 
     {
	Instance *inst;
	
	inst = l->data;
	if (inst->ci != ci) continue;
	if (!inst->timer)
	  inst->timer = ecore_timer_add(0.5, _net_cb_poll, inst);
	if (!ci->show_text)
	  edje_object_signal_emit(inst->o_net, "e,state,text,hide", "e");
	else
	  edje_object_signal_emit(inst->o_net, "e,state,text,show", "e");
     }
}
