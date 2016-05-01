#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   double poll_time;
   double days;
   int    display;
   int    degrees;
   char  *code;
   int    show_text;
   int    by_code;
   int    popup_on_hover;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas,
                                          E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd,
                                      E_Config_Dialog_Data *cfdata);
static void         _fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata);

void
_config_forecasts_module(Config_Item *ci)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   E_Container *con;
   char buf[4096];

   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->override_auto_apply = 1;

   snprintf(buf, sizeof(buf), "%s/module.edj",
            e_module_dir_get(forecasts_config->module));
   con = e_container_current_get(e_manager_current_get());
   cfd =
     e_config_dialog_new(con, D_("Forecasts Settings"), "Forecasts", "_e_modules_forecasts_config_dialog", buf, 0, v, ci);
   forecasts_config->config_dialog = cfd;
}

static void
_fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata)
{
   cfdata->poll_time = (ci->poll_time / 60.0);
   cfdata->days = (ci->days / 5.0);
   cfdata->degrees = ci->degrees;
   if (ci->code)
     cfdata->code = strdup(ci->code);
   cfdata->show_text = ci->show_text;
   cfdata->popup_on_hover = ci->popup_on_hover;
   cfdata->by_code = ci->by_code;
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
   if (!forecasts_config)
     return;

   forecasts_config->config_dialog = NULL;
   free(cfdata);
   cfdata = NULL;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas,
                      E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob;
   E_Radio_Group *dg;
   char buf[4096];
   //~ char *ss;
   //~ ss="zourbuth.com/tools/woeid";

   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, D_("Display Settings"), 0);
   ob = e_widget_label_add(evas, D_("Poll Time"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, D_("%2.0f minutes"), 15.0, 60.0, 1.0, 0,&(cfdata->poll_time), NULL, 40);
   e_widget_framelist_object_append(of, ob);
   
   ob = e_widget_label_add(evas, D_("Forecasts days"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, D_("%2.0f days       "), 2.0, 5.0, 1.0, 0, &(cfdata->days), NULL, 40);
   e_widget_framelist_object_append(of, ob);
   
   ob = e_widget_check_add(evas, D_("Show Description"), &(cfdata->show_text));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, D_("Popup on mouse over"), &(cfdata->popup_on_hover));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, D_("Unit Settings"), 0);
   dg = e_widget_radio_group_new(&(cfdata->degrees));
   ob = e_widget_radio_add(evas, D_("Metric"), DEGREES_C, dg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("English"), DEGREES_F, dg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   

   of = e_widget_frametable_add(evas, D_("GeoPlanet WOEID (Where on Earth Identifier)"), 0);
   
   dg = e_widget_radio_group_new(&(cfdata->by_code));
   ob = e_widget_radio_add(evas, D_("Code"), WOEID_CODE, dg);
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   ob = e_widget_radio_add(evas, D_("City name"), WOEID_CITY, dg);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 0, 1, 0);
   
   ob = e_widget_label_add(evas, D_("Forecasts WOEID Code or city name: "));
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &cfdata->code, NULL, NULL, NULL);
   e_widget_size_min_set(ob, 120, 28);
   e_widget_frametable_object_append(of, ob, 1, 3, 1, 1, 1, 0, 1, 0);
   ob = e_widget_label_add(evas, D_("To find the code, go to:"));
   e_widget_frametable_object_append(of, ob, 0, 4, 1, 1, 1, 0, 1, 0);
   
   //~ ob = e_widget_entry_add(evas, &ss, NULL, NULL, NULL);
   //~ e_widget_size_min_set(ob, 150, 28);
   //~ e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 0, 1, 0);
   
   snprintf(buf, sizeof(buf), D_("%s, find your area, and look at the URL"), "http://zourbuth.com/tools/woeid");
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 0, 5, 3, 1, 1, 0, 1, 0);
   e_widget_list_object_append(o, of, 1, 7, 0.5);

   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Config_Item *ci;

   ci = cfd->data;
   if (!cfdata->code)
     return 0;

   if (!strlen(cfdata->code))
     return 0;

   ci->degrees = cfdata->degrees;
   ci->poll_time = (cfdata->poll_time * 60.0);
   ci->days = (cfdata->days * 5.0);
   
   if (ci->code)
     eina_stringshare_del(ci->code);

   char *t;
   t = strdup(cfdata->code);
   *t = toupper(*t);
   ci->code = eina_stringshare_add(t);
   ci->show_text = cfdata->show_text;
   ci->popup_on_hover = cfdata->popup_on_hover;
   ci->by_code = cfdata->by_code;

   e_config_save_queue();
   _forecasts_config_updated(ci);
   return 1;
}

