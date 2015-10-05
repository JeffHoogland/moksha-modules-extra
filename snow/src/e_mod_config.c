#include <e.h>
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "config.h"

#define DENSITY_SPARSE 0
#define DENSITY_MEDIUM 1
#define DENSITY_DENSE 2

struct _E_Config_Dialog_Data
{
   int show_trees;
   int density;
};

/* Protos */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

E_Config_Dialog *
e_int_config_snow_module(E_Container *con, const char *params)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   Snow *s;
   char buf[PATH_MAX];

   s = snow_module->data;

   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = NULL;
   v->advanced.create_widgets = NULL;

   snprintf(buf, sizeof(buf), "%s/e-module-snow.edj", e_module_dir_get(s->module));
   cfd = e_config_dialog_new(con, D_("Snow Configuration"), "Snow", "appearance/snow", buf, 0, v, s);
   s->config_dialog = cfd;
   return cfd;
}

static void
_fill_data(Snow *sn, E_Config_Dialog_Data *cfdata)
{
   cfdata->show_trees = sn->conf->show_trees;
   switch (sn->conf->flake_count)
     {
     case 5:
        cfdata->density = DENSITY_SPARSE;
        break;
     case 10:
        cfdata->density = DENSITY_MEDIUM;
        break;
     case 20:
        cfdata->density = DENSITY_DENSE;
        break;
     default:
        break;
     }
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;
   Snow *s;

   s = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);

   _fill_data(s, cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Snow *sn;

   sn = cfd->data;
   sn->config_dialog = NULL;
   free(cfdata);
   cfdata = NULL;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob;
   E_Radio_Group *rg;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, D_("General Settings"), 0);
   ob = e_widget_check_add(evas, D_("Show Trees"), &(cfdata->show_trees));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, D_("Snow Density"), 0);
   rg = e_widget_radio_group_new(&(cfdata->density));
   ob = e_widget_radio_add(evas, D_("Sparse"), DENSITY_SPARSE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Medium"), DENSITY_MEDIUM, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Dense"), DENSITY_DENSE, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Snow *sn;

   sn = cfd->data;
   e_border_button_bindings_ungrab_all();
   switch (cfdata->density)
     {
     case 0:
        sn->conf->tree_count = 5;
        sn->conf->flake_count = 20;
        break;
     case 1:
        sn->conf->tree_count = 10;
        sn->conf->flake_count = 60;
        break;
     case 2:
        sn->conf->tree_count = 20;
        sn->conf->flake_count = 150;
        break;
     default:
        break;
     }
   sn->conf->show_trees = cfdata->show_trees;

   e_config_save_queue();
   e_border_button_bindings_grab_all();

   _snow_cb_config_updated(sn);
   return 1;
}
