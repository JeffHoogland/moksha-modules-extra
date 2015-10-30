#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
  int show_date, show_time, show_tip;
  char *time_format, *date_format, *tip_format;
};

/* Protos */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd __UNUSED__, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _cb_time_check(void *data, Evas_Object *obj);
static void _cb_date_check(void *data, Evas_Object *obj);
static void _cb_tooltip_check(void *data, Evas_Object *obj);

void
_config_tclock_module(Config_Item *ci)
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;
   E_Container *con = NULL;
   char buf[PATH_MAX];

   if (e_config_dialog_find("TClock", "_e_modules_tclock_config_dialog")) 
     return;

   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.create_widgets = _basic_create;

   snprintf(buf, sizeof(buf), "%s/e-module-tclock.edj", 
            tclock_config->mod_dir);
   con = e_container_current_get(e_manager_current_get());
   cfd = e_config_dialog_new(con, D_("Tclock Settings"), "TClock", 
                              "_e_modules_tclock_config_dialog", buf, 0, v, ci);
   tclock_config->config_dialog = cfd;
}

static void
_fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata)
{
   cfdata->show_time = ci->show_time;
   cfdata->show_date = ci->show_date;
   cfdata->show_tip = ci->show_tip;
   if (ci->time_format) cfdata->time_format = strdup(ci->time_format);
   if (ci->date_format) cfdata->date_format = strdup(ci->date_format);
   if (ci->tip_format) cfdata->tip_format = strdup(ci->tip_format);
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata = NULL;
   Config_Item *ci = NULL;

   ci = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(ci, cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata)
{
   if (!tclock_config) return;
   tclock_config->config_dialog = NULL;
   free(cfdata->time_format);
   free(cfdata->date_format);
   free(cfdata->tip_format);
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd __UNUSED__, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o = NULL, *of = NULL, *ob = NULL;
   Evas_Object *time_entry = NULL, *time_check = NULL;
   Evas_Object *date_entry = NULL, *date_check = NULL;
   Evas_Object *tooltip_entry = NULL, *tooltip_check = NULL;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_frametable_add(evas, D_("Top"), 1);
   time_check =
     e_widget_check_add(evas, D_("Show Top Line"), &(cfdata->show_time));
   e_widget_frametable_object_append(of, time_check, 0, 0, 1, 1, 1, 0, 1, 0);
   time_entry = e_widget_entry_add(evas, &cfdata->time_format, NULL, NULL, NULL);
   e_widget_on_change_hook_set(time_check, _cb_time_check, time_entry);
   e_widget_disabled_set(time_entry, !cfdata->show_time);
   e_widget_size_min_set(time_entry, 150, 20);
   e_widget_frametable_object_append(of, time_entry, 0, 1, 1, 1, 1, 0, 1, 0);
   ob =
     e_widget_label_add(evas, D_("Consult strftime(3) for format syntax"));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 0, 1, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_frametable_add (evas, D_ ("Bottom"), 1);
   date_check =
     e_widget_check_add(evas, D_("Show Bottom Line"), &(cfdata->show_date));
   e_widget_frametable_object_append(of, date_check, 0, 0, 1, 1, 1, 0, 1, 0);
   date_entry = e_widget_entry_add(evas, &(cfdata->date_format), 
                                   NULL, NULL, NULL);
   e_widget_on_change_hook_set(date_check, _cb_date_check, date_entry);
   e_widget_disabled_set(date_entry, !cfdata->show_date);
   e_widget_size_min_set(date_entry, 150, 20);
   e_widget_frametable_object_append(of, date_entry, 0, 1, 1, 1, 1, 0, 1, 0);
   ob =
     e_widget_label_add(evas, D_("Consult strftime(3) for format syntax"));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 0, 1, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, D_("Tool Tip"), 1);
   tooltip_check =
     e_widget_check_add(evas, D_("Show Tooltip"), &(cfdata->show_tip));
   e_widget_frametable_object_append(of, tooltip_check, 0, 0, 1, 1, 1, 0, 1, 0);
   tooltip_entry = e_widget_entry_add(evas, &(cfdata->tip_format), 
                                      NULL, NULL, NULL);
   e_widget_on_change_hook_set(tooltip_check, _cb_tooltip_check, tooltip_entry);
   e_widget_disabled_set(tooltip_entry, !cfdata->show_tip);
   e_widget_size_min_set(tooltip_entry, 150, 20);
   e_widget_frametable_object_append(of, tooltip_entry, 
                                     0, 1, 1, 1, 1, 0, 1, 0);
   ob =
     e_widget_label_add(evas, D_("Consult strftime(3) for format syntax"));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 0, 1, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Config_Item *ci = NULL;

   ci = cfd->data;
   ci->show_date = cfdata->show_date;
   ci->show_time = cfdata->show_time;
   ci->show_tip = cfdata->show_tip;
   if (ci->time_format) eina_stringshare_del(ci->time_format);
   ci->time_format = eina_stringshare_add(cfdata->time_format);
   if (ci->date_format) eina_stringshare_del(ci->date_format);
   ci->date_format = eina_stringshare_add(cfdata->date_format);
   if (ci->tip_format) eina_stringshare_del(ci->tip_format);
   ci->tip_format = eina_stringshare_add(cfdata->tip_format);

   e_config_save_queue();

   _tclock_config_updated(ci);
   return 1;
}

static void
_cb_time_check(void *data, Evas_Object *obj)
{
   int checked = 0;

   checked = e_widget_check_checked_get(obj);
   e_widget_disabled_set(data, !checked);
}

static void
_cb_date_check(void *data, Evas_Object *obj)
{
   int checked = 0;

   checked = e_widget_check_checked_get(obj);
   e_widget_disabled_set(data, !checked);
}

static void
_cb_tooltip_check(void *data, Evas_Object *obj)
{
   int checked = 0;

   checked = e_widget_check_checked_get(obj);
   e_widget_disabled_set(data, !checked);
}
