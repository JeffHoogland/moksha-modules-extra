#include <e.h>
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "config.h"

#define DENSITY_SPRINKLE 0
#define DENSITY_DRIZZLE 1
#define DENSITY_DOWNPOUR 2

struct _E_Config_Dialog_Data
{
   int show_clouds;
   int density;
};

/* Protos */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _fill_data(Rain *rn, E_Config_Dialog_Data *cfdata);

E_Config_Dialog *e_int_config_rain_module(E_Container *con, const char *params)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   Rain *r;
   char buf[PATH_MAX];

   r = rain_module->data;

   v = E_NEW(E_Config_Dialog_View, 1);

   if (v)
     {
        v->create_cfdata = _create_data;
        v->free_cfdata = _free_data;
        v->basic.apply_cfdata = _basic_apply_data;
        v->basic.create_widgets = _basic_create_widgets;
	
	snprintf(buf, sizeof(buf), "%s/e-module-rain.edj", e_module_dir_get(r->module));
        cfd = e_config_dialog_new(con, D_("Rain Module"), "Rain", "appearance/rain", buf, 0, v, r);
        r->config_dialog = cfd;
     }
   return cfd;
}

static void
_fill_data(Rain *rn, E_Config_Dialog_Data *cfdata)
{
   cfdata->show_clouds = rn->conf->show_clouds;
   switch (rn->conf->cloud_count)
     {
     case 5:
        cfdata->density = DENSITY_SPRINKLE;
        break;
     case 10:
        cfdata->density = DENSITY_DRIZZLE;
        break;
     case 20:
        cfdata->density = DENSITY_DOWNPOUR;
        break;
     default:
        break;
     }
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;
   Rain *r;

   r = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);

   _fill_data(r, cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Rain *rn;

   rn = cfd->data;
   rn->config_dialog = NULL;
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
   ob = e_widget_check_add(evas, D_("Show Clouds"), &(cfdata->show_clouds));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, D_("Rain Density"), 0);
   rg = e_widget_radio_group_new(&(cfdata->density));
   ob = e_widget_radio_add(evas, D_("Sprinkle"), DENSITY_SPRINKLE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Drizzle"), DENSITY_DRIZZLE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Downpour"), DENSITY_DOWNPOUR, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Rain *rn;

   rn = cfd->data;
   e_border_button_bindings_ungrab_all();
   switch (cfdata->density)
     {
     case 0:
        rn->conf->cloud_count = 5;
        rn->conf->drop_count = 20;
        break;
     case 1:
        rn->conf->cloud_count = 10;
        rn->conf->drop_count = 60;
        break;
     case 2:
        rn->conf->cloud_count = 20;
        rn->conf->drop_count = 150;
        break;
     default:
        break;
     }
   rn->conf->show_clouds = cfdata->show_clouds;

   e_config_save_queue();
   e_border_button_bindings_grab_all();

   _rain_cb_config_updated(rn);
   return 1;
}
