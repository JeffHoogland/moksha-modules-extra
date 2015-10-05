#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data 
{
   Eina_List *disks;
   
   int diskpos;
};

/* Local Function Prototypes */
static void *_create_data(E_Config_Dialog *cfd);
static void _fill_data(Config_Item * ci, E_Config_Dialog_Data *cfdata);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

/* External Functions */
E_Config_Dialog *
e_int_config_diskio_module(E_Container *con, Config_Item *ci) 
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;
   char buf[4096];

   /* is this config dialog already visible ? */
   if (e_config_dialog_find("DiskIO", "_e_module_diskio_cfg_dlg")) return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;

   /* Icon in the theme */
   snprintf(buf, sizeof(buf), "%s/e-module-diskio.edj", diskio_conf->module->dir);

   /* create new config dialog */
   cfd = e_config_dialog_new(con, "DiskIO Module", "DiskIO", 
                             "_e_module_diskio_cfg_dlg", buf, 0, v, ci);

   e_dialog_resizable_set(cfd->dia, 1);
   diskio_conf->cfd = cfd;
   return cfd;
}

/* Local Functions */
static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata = NULL;
   Config_Item *ci;

   ci = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);

   _fill_data(ci, cfdata);
   return cfdata;
}

static void 
_fill_data(Config_Item * ci, E_Config_Dialog_Data *cfdata) 
{
   // load available disk devices
   cfdata->disks = ecore_file_ls("/sys/block");
}

static void 
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   diskio_conf->cfd = NULL;
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Config_Item *ci;
   Eina_List *l;
   Evas_Object *o = NULL, *of = NULL, *ob = NULL;
   E_Radio_Group *rg;
   char path[128], *disk;
   int pos = -1, disk_found = 0;

   ci = cfd->data;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, "Disk", 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);
   rg = e_widget_radio_group_new (&(cfdata->diskpos));

   if (cfdata->disks)
     {
		EINA_LIST_FOREACH(cfdata->disks, l, disk)
		  {
		     pos++;

			 snprintf (path, sizeof (path), "/sys/block/%s/device", disk);
			 if (!ecore_file_exists(path)) continue;

			 ob = e_widget_radio_add (evas, D_ (disk), pos, rg);
			 if (strcmp(disk, ci->disk)==0)
			   {
			 	  e_widget_radio_toggle_set(ob, 1);
				  disk_found = 1;
			   }
			 e_widget_framelist_object_append(of, ob);
		  }
     }

   if (!disk_found)
	 {
		ob = e_widget_radio_add (evas, D_ (ci->disk), -1, rg);
		e_widget_radio_toggle_set(ob, 1);
		e_widget_framelist_object_append(of, ob);
	 }

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int 
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   Config_Item *ci;
   Eina_List *l;
   char *disk;
   int pos = -1;

   ci = cfd->data;

   for (l = cfdata->disks; l; l = eina_list_next(l))
	 {
	    disk = eina_list_data_get(l);
        pos++;

	    if (pos == cfdata->diskpos)
		  {
			 if (ci->disk) eina_stringshare_del(ci->disk);
			 ci->disk = eina_stringshare_add(disk);
			 break;
		  }
	}

   e_config_save_queue();
   return 1;
}
