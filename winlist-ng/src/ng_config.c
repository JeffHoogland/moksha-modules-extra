#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
  double size;
  double zoomfactor;
  int    autohide;
  double hide_timeout;
  double zoom_duration;
  double zoom_range;

  int hide_animation;
  
  Evas_Object *ilist;

  Config_Item *cfg;

};

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);


E_Config_Dialog *
ngw_configure_module(Config_Item *ci)
{
  E_Config_Dialog *cfd;
  E_Config_Dialog_View *v;
  char buf[4096];
  if(ci->ng->cfd) return NULL;

  v = E_NEW(E_Config_Dialog_View, 1);

  /* Dialog Methods */
  v->create_cfdata = _create_data;
  v->free_cfdata = _free_data;
  v->basic.apply_cfdata = _basic_apply_data;
  v->basic.create_widgets = _basic_create_widgets;
  v->advanced.apply_cfdata = NULL;
  v->advanced.create_widgets = NULL;

  /* Create The Dialog */
  snprintf(buf, sizeof(buf), "%s/e-module-ng.edj", e_module_dir_get(ngw_config->module));
  cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			    D_("Winlist NG Configuration"),
			    "E", "_e_mod_ngw_config_dialog", buf, 0, v, ci);
  ci->ng->cfd = cfd; // TODO: REMOVE

  return cfd;
}


static void
_fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata)
{
  cfdata->size = ci->size;
  cfdata->zoomfactor = ci->zoomfactor;
  cfdata->hide_timeout = ci->hide_timeout;
  cfdata->zoom_duration = ci->zoom_duration;
  cfdata->zoom_range = ci->zoom_range;
  cfdata->hide_animation = ci->hide_animation;
  cfdata->cfg = ci;

  cfdata->ilist = NULL;
}


static void *
_create_data(E_Config_Dialog *cfd)
{
  E_Config_Dialog_Data *cfdata;
  Config_Item *ci;

  ci = (Config_Item*) cfd->data;
  cfdata = E_NEW(E_Config_Dialog_Data, 1);
  _fill_data(ci, cfdata);
  return cfdata;
}


static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
  cfdata->cfg->ng->cfd = NULL;

  free(cfdata);
}


static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
  Evas_Object *o, *of, *ob;
  E_Radio_Group *rg;

  o = e_widget_list_add(evas, 0, 0);
  /*
  of = e_widget_framelist_add(evas, D_("Mode"), 0);
  ob = e_widget_check_add(evas, D_("Use Composite"), &(cfdata->use_composite));
  e_widget_framelist_object_append(of, ob);
  e_widget_list_object_append(o, of, 1, 1, 0.5);
  */
  of = e_widget_framelist_add(evas, D_("Fade Animation"), 0);

  rg = e_widget_radio_group_new(&cfdata->hide_animation);
  ob = e_widget_radio_add(evas, "None", 0, rg);
  e_widget_framelist_object_append(of, ob); 
  ob = e_widget_radio_add(evas, "Opacity", 1, rg);
  e_widget_framelist_object_append(of, ob);
  ob = e_widget_radio_add(evas, "Shrink", 2, rg);
  e_widget_framelist_object_append(of, ob);
  e_widget_list_object_append(o, of, 1, 1, 0.5);

  of = e_widget_framelist_add(evas, D_("Size"), 0);
  ob = e_widget_label_add (evas, D_("Icon Size:"));
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_slider_add (evas, 1, 0, D_("%1.0f px"), 16.0, 128,
			    1.0, 0, &(cfdata->size), NULL, 100);
  e_widget_framelist_object_append (of, ob);
  e_widget_list_object_append(o, of, 1, 1, 0.5);

  of = e_widget_framelist_add(evas, D_("Zooming"), 0);
  ob = e_widget_label_add (evas, D_("Zoomfactor:"));
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_slider_add (evas, 1, 0, D_("%0.1f"), 1.0, 3.0,
			    0.1, 0, &(cfdata->zoomfactor), NULL, 100);
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_label_add (evas, D_("Zoom Range:"));
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_slider_add (evas, 1, 0, D_("%0.1f"), 0.2, 4.0,
			    0.1, 0, &(cfdata->zoom_range), NULL, 100);
  e_widget_framelist_object_append (of, ob);
  e_widget_list_object_append(o, of, 1, 1, 0.5);

  return o;
}


static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
  Config_Item *ci;

  ci = (Config_Item*) cfd->data;

  ci->size = (int) cfdata->size;

  ci->zoomfactor = cfdata->zoomfactor;

  ci->hide_timeout = cfdata->hide_timeout;
  ci->zoom_duration = cfdata->zoom_duration;
  ci->zoom_range = cfdata->zoom_range;
  ci->hide_animation = cfdata->hide_animation;
  
  e_config_domain_save("module.winlist_ng", conf_edd, ngw_config);

  return 1;
}
