#include "e_mod_main.h"

static void *           _create_data(E_Config_Dialog *cfd);
static void             _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *    _basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void             _ilist_fill(E_Config_Dialog_Data *cfdata);
static void             _ilist_cb_selected(void *data);
static void             _cb_add(void *data, void *data2);
static void             _cb_delete(void *data, void *data2);
static void             _cb_dialog_yes(void *data);
static void             _cb_dialog_destroy(void *data);
static void             _cb_config(void *data, void *data2);

struct _E_Config_Dialog_Data
{
   Evas_Object *o_list;
   Evas_Object *o_delete;
   Evas_Object *o_config;

   const char  *cur_ng;
};

typedef struct _Ngi_Del_Confirm_Data   Ngi_Del_Confirm_Data;
struct _Ngi_Del_Confirm_Data
{
   E_Config_Dialog_Data *cfdata;
   Config_Item          *cfg;
};

EAPI E_Config_Dialog *
ngi_instances_config(E_Container *con, const char *params)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (params)
     {
        int i = atoi(params);
        Config_Item *ci = eina_list_nth(ngi_config->items, i);
        if (ci)
          {
             ngi_configure_module(ci);
             return ci->config_dialog;
          }

        return NULL;
     }

   if(ngi_config->cfd)
      return NULL;

   if (e_config_dialog_find("E", "extensions/engage"))
      return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v)
      return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create_widgets;

   cfd = e_config_dialog_new(con,
                             D_("Engage Instances"),
                             "E", "extensions/engage",
                             "enlightenment/module/ng-instances", 0, v, NULL);
   ngi_config->cfd = cfd;
   return cfd;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   ngi_config->cfd = NULL;
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ot, *ob;

   o = e_widget_list_add(evas, 0, 1);

   of = e_widget_framelist_add(evas, D_("Engage Instances"), 0);
   cfdata->o_list = e_widget_ilist_add(evas, 24, 24, &(cfdata->cur_ng));
   e_widget_ilist_selector_set(cfdata->o_list, 1);
   e_widget_size_min_set(cfdata->o_list, 200, 250);
   e_widget_framelist_object_append(of, cfdata->o_list);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   ot = e_widget_table_add(evas, 0);
   ob = e_widget_button_add(evas, D_("Add"), "widget/add", _cb_add, cfdata, NULL);
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 1, 1, 0, 0);
   cfdata->o_delete = e_widget_button_add(evas, D_("Delete"), "widget/del", _cb_delete, cfdata, NULL);
   e_widget_table_object_append(ot, cfdata->o_delete, 0, 1, 1, 1, 1, 1, 0, 0);
   cfdata->o_config = e_widget_button_add(evas, D_("Configure"), "widget/config", _cb_config, cfdata, NULL);
   e_widget_table_object_append(ot, cfdata->o_config, 0, 2, 1, 1, 1, 1, 0, 0);

   e_widget_disabled_set(cfdata->o_delete, 1);
   e_widget_disabled_set(cfdata->o_config, 1);

   e_widget_list_object_append(o, ot, 1, 0, 0.0);

   _ilist_fill(cfdata);

   e_dialog_resizable_set(cfd->dia, 1);
   return o;
}

/* private functions */
static void
_ilist_fill(E_Config_Dialog_Data *cfdata)
{
   Evas *evas;
   Eina_List *l;
   int n = -1;
   char buf[256];
   Ng *ng;
   Config_Item *ci;
   Evas_Object *ob;
   const char *label;

   if (!cfdata)
      return;

   if (!cfdata->o_list)
      return;

   evas = evas_object_evas_get(cfdata->o_list);

   if (e_widget_ilist_count(cfdata->o_list) > 0)
      n = e_widget_ilist_selected_get(cfdata->o_list);

   evas_event_freeze(evas);
   edje_freeze();
   e_widget_ilist_freeze(cfdata->o_list);

   e_widget_ilist_clear(cfdata->o_list);
   e_widget_ilist_go(cfdata->o_list);

   EINA_LIST_FOREACH (ngi_config->items, l, ci)
   {
      if (!ci)
         continue;

      ng = ci->ng;

      /* FIXME*/
      /*if(ci->launcher)
         label = D_("Launcher NG");
         else if(ci->taskbar)
         label = D_("Taskbar NG");
       */
      label = D_("Bar");

      snprintf(buf, sizeof(buf), "%s / Zone %d", label, ci->zone);

      ob = edje_object_add(evas);
      switch (ci->orient)
        {
         case E_GADCON_ORIENT_LEFT:
            e_util_edje_icon_set(ob, "enlightenment/shelf_position_left");
            break;

         case E_GADCON_ORIENT_RIGHT:
            e_util_edje_icon_set(ob, "enlightenment/shelf_position_right");
            break;

         case E_GADCON_ORIENT_TOP:
            e_util_edje_icon_set(ob, "enlightenment/shelf_position_top");
            break;

         case E_GADCON_ORIENT_BOTTOM:
            e_util_edje_icon_set(ob, "enlightenment/shelf_position_bottom");
            break;

         default:
            e_util_edje_icon_set(ob, "enlightenment/e");
            break;
        } /* switch */
      e_widget_ilist_append(cfdata->o_list, ob, buf, _ilist_cb_selected, cfdata, buf);
   }

   e_widget_size_min_set(cfdata->o_list, 155, 250);
   e_widget_ilist_go(cfdata->o_list);

   e_widget_ilist_thaw(cfdata->o_list);
   edje_thaw();
   evas_event_thaw(evas);

   if (n > -1)
     {
        e_widget_disabled_set(cfdata->o_delete, 0);
        e_widget_disabled_set(cfdata->o_config, 0);
        e_widget_ilist_selected_set(cfdata->o_list, n);
     }
   else
     {
        e_widget_disabled_set(cfdata->o_delete, 1);
        e_widget_disabled_set(cfdata->o_config, 1);
     }
}

static void
_ilist_cb_selected(void *data)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   if (!cfdata)
      return;

   e_widget_disabled_set(cfdata->o_delete, 0);
   e_widget_disabled_set(cfdata->o_config, 0);
}

static void
_cb_add(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   Config_Item *cfg;
   Config_Box *cfg_box;
   E_Container *con;
   E_Zone *zone;

   if (!cfdata)
      return;

   con = e_container_current_get(e_manager_current_get());
   zone = e_zone_current_get(con);

   cfg = ngi_bar_config_new(con->num, zone->num);
   
   ngi_new(cfg);

   _ilist_fill(cfdata);
}

static void
_cb_delete(void *data, void *data2)
{
   Ngi_Del_Confirm_Data *d;
   char buf[4096];
   Config_Item *ci;

   d = E_NEW(Ngi_Del_Confirm_Data, 1);
   if (!d)
      return;

   d->cfdata = (E_Config_Dialog_Data *)data;
   if (!d->cfdata)
      return;

   if (!d->cfdata->cur_ng)
      return;

   ci = (Config_Item *)eina_list_nth(ngi_config->items, e_widget_ilist_selected_get(d->cfdata->o_list));
   if (!ci)
      return;

   d->cfg = ci;

   snprintf(buf, sizeof(buf), D_("You requested to delete \"%s\".<br><br>"
                                 "Are you sure you want to delete this Instance?"),
            d->cfdata->cur_ng);

   e_confirm_dialog_show(D_("Are you sure you want to delete this Instance?"),
                         "enlightenment/exit", buf, NULL, NULL, _cb_dialog_yes, NULL, d, NULL,
                         _cb_dialog_destroy, d);
}

static void
_cb_dialog_yes(void *data)
{
   Ngi_Del_Confirm_Data *d = (Ngi_Del_Confirm_Data *)data;
   if (!data)
      return;

   if(d->cfg->ng)
      ngi_free(d->cfg->ng);

   ngi_config->items = eina_list_remove(ngi_config->items, d->cfg);

   e_config_domain_save("module.ng", ngi_conf_edd, ngi_config);
}

static void
_cb_dialog_destroy(void *data)
{
   Ngi_Del_Confirm_Data *d = (Ngi_Del_Confirm_Data *)data;
   _ilist_fill(d->cfdata);
   E_FREE(d);
}

static void
_cb_config(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   Config_Item *ci;
   int item = 0;

   if (!cfdata)
      return;

   item = e_widget_ilist_selected_get(cfdata->o_list);

   ci = (Config_Item *)eina_list_nth(ngi_config->items, item);
   if (!ci)
      return;

   ngi_configure_module(ci);
}

