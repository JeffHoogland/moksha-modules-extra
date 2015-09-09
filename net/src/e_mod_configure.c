#include <e.h>
#include "e_mod_config.h"
#include "e_mod_main.h"
#include "e_mod_configure.h"

struct _E_Config_Dialog_Data 
{
   char *device;
   char *app;
   int limit, show_text;
   int show_popup;
   
   Eina_List *devs;
   int num;
};

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

EINTERN void 
_configure_net_module(void *data) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   E_Container *con;
   Config_Item *ci;
   char buf[PATH_MAX];

   if (e_config_dialog_find("Net", "_e_modules_netmod_config_dialog")) return;
   
   ci = data;
   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _apply_data;
   v->basic.create_widgets = _basic_create;
   
   snprintf(buf, sizeof(buf), "%s/e-module-net.edj", 
            e_module_dir_get(net_cfg->mod));
   con = e_container_current_get(e_manager_current_get());
   cfd = e_config_dialog_new(con, D_("Net Module Settings"), "Net", 
			     "_e_modules_netmod_config_dialog", buf, 0, v, ci);
   net_cfg->cfd = cfd;
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   Config_Item *ci;
   
   ci = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(ci, cfdata);
   return cfdata;
}

static void 
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if (cfdata->devs) eina_list_free(cfdata->devs);
   E_FREE(cfdata->device);
   E_FREE(cfdata->app);
   E_FREE(cfdata);
   if (!net_cfg) return;
   net_cfg->cfd = NULL;
}

static void 
_fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata) 
{
   Eina_List *l;
   char *tmp;
   int i = 0;
   
   cfdata->device = NULL;
   if (ci->device) 
     cfdata->device = strdup(ci->device);
   
   cfdata->app = NULL;
   if (ci->app)
     cfdata->app = strdup(ci->app);

   cfdata->show_text = ci->show_text;
   cfdata->show_popup = ci->show_popup;
   cfdata->limit = ci->limit;
   
   cfdata->devs = _net_config_devices_get();
   if (!cfdata->devs) return;
   EINA_LIST_FOREACH(cfdata->devs, l, tmp)
     {
	if (!cfdata->device) continue;
	if (!strcmp(cfdata->device, tmp)) 
	  {
	     cfdata->num = i;
	     break;
	  }
	i++;
     }
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *ob, *of;
   E_Radio_Group *rg;
   Eina_List *l;
   char *tmp;
   int i = 0;
   
   o = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_framelist_add(evas, D_("General Settings"), 0);
   ob = e_widget_check_add(evas, D_("Show Text"), &(cfdata->show_text));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, D_("Show Popup On Mouse-Over"), &(cfdata->show_popup));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, D_("Launch Application On Double-Click"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_entry_add(evas, &(cfdata->app), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, D_("Activity Notification Level"), 0);
   rg = e_widget_radio_group_new(&(cfdata->limit));
   ob = e_widget_radio_add(evas, D_("High (MB)"), 1048575, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Middle (KB)"), 1023, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Low (B)"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   if (!cfdata->devs) return o;
   
   of = e_widget_framelist_add(evas, D_("Device Settings"), 0);
   rg = e_widget_radio_group_new(&(cfdata->num));
   EINA_LIST_FOREACH(cfdata->devs, l, tmp)
     {
	ob = e_widget_radio_add(evas, tmp, i, rg);
	e_widget_framelist_object_append(of, ob);
	i++;
     }
   
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   return o;
}

static int 
_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   char *tmp;
   Config_Item *ci;
   
   ci = cfd->data;
   tmp = eina_list_nth(cfdata->devs, cfdata->num);
   if (tmp) 
     {
	eina_stringshare_del(ci->device);
	ci->device = eina_stringshare_add(tmp);
     }
   ci->limit = cfdata->limit;
   ci->show_text = cfdata->show_text;
   ci->show_popup = cfdata->show_popup;
   
   if (ci->app) eina_stringshare_del(ci->app);
   if (cfdata->app)
     ci->app = eina_stringshare_add(cfdata->app);
   
   e_config_save_queue();
   _net_config_updated(ci);
   return 1;
}
