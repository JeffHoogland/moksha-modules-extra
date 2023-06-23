#include <e.h>
#include "e_mod_main.h"

/* The typedef for this structure is declared inside the E code in order to
 * allow everybody to use this type, you dont need to declare the typedef, 
 * just use the E_Config_Dialog_Data for your data structures declarations */
struct _E_Config_Dialog_Data 
{
   char *fileman;
};

/* Local Function Prototypes */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

/* External Functions */

/* Function for calling our personal dialog menu */
E_Config_Dialog *
e_int_config_trash_module(E_Container *con, const char *params) 
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;
   char buf[4096];

   /* is this config dialog already visible ? */
   if (e_config_dialog_find("Trash", "advanced/trash")) return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;

   /* Icon in the theme */
   snprintf(buf, sizeof(buf), "%s/e-module-trash.edj", trash_conf->module->dir);

   /* create our config dialog */
   cfd = e_config_dialog_new(con, D_("Trash module"), "Trash", 
                             "advanced/trash", buf, 0, v, NULL);

   e_dialog_resizable_set(cfd->dia, 0);
   trash_conf->cfd = cfd;
   return cfd;
}

/* Local Functions */
static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata = NULL;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void 
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   trash_conf->cfd = NULL;
   E_FREE(cfdata->fileman);
   E_FREE(cfdata);
}

static void 
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   /* load a temp copy of the config variables */
   if (trash_conf->fileman) cfdata->fileman = strdup(trash_conf->fileman);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_frametable_add(evas, D_("Setting"), 0);
      
   ow = e_widget_label_add(evas, D_("File manager to open the trash:"));
   e_widget_frametable_object_append(of, ow, 0, 0, 1, 1, 1, 0, 1, 0);
   ow = e_widget_entry_add(evas, &cfdata->fileman, NULL, NULL, NULL);
   e_widget_size_min_set(ow, 100, 28);
   e_widget_frametable_object_append(of, ow, 0, 1, 1, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("Use pcmanfm -n for opening a new instance on the current desktop"));
   e_widget_frametable_object_append(of, ow, 0, 3, 1, 1, 1, 0, 1, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int 
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
  
   if (trash_conf->fileman) eina_stringshare_del(trash_conf->fileman);
   if (cfdata->fileman)
       trash_conf->fileman = eina_stringshare_add(cfdata->fileman);
   
   e_config_save_queue();
   
   return 1;
}
