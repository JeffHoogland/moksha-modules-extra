#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   int              show_label;
   int              show_background;
   int              orient;
   int              size;
   double           zoomfactor;
   int              autohide;
   int              autohide_show_urgent;
   int              lock_deskswitch;
   int              ecomorph_features;

   double           hide_timeout;
   double           zoom_duration;
   double           zoom_range;

   int              hide_below_windows;
   int              hide_mode;
   int              alpha;
   int              stacking;
   int              mouse_over_anim;

   Eina_List       *boxes;

   Evas_Object     *ilist;

   Evas_Object     *tlist_box;
   Config_Box      *cfg_box;
   Config_Item     *cfg;
   E_Config_Dialog *cfd;

   const char      *app_dir;
};

static void *           _create_data(E_Config_Dialog *cfd);
static void             _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *    _basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int              _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void             _cb_add(void *data, void *data2);
static void             _cb_del(void *data, void *data2);
static void             _cb_config(void *data, void *data2);
static void             _cb_entry_ok(void *data, char *text);
static void             _cb_confirm_dialog_yes(void *data);
static void             _load_ilist(E_Config_Dialog_Data *cfdata);
static void             _cb_slider_change(void *data, Evas_Object *obj);

static void             _cb_box_add_launcher(void *data, void *data2);
static void             _cb_box_add_taskbar(void *data, void *data2);
static void             _cb_box_add_gadcon(void *data, void *data2);
static void             _cb_box_del(void *data, void *data2);
static void             _cb_box_config(void *data, void *data2);
static void             _cb_box_up(void *data, void *data2);
static void             _cb_box_down(void *data, void *data2);
static void             _load_box_tlist(E_Config_Dialog_Data *cfdata);

void
ngi_configure_module(Config_Item *ci)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   Eina_List *l;
   char buf[4096];
   char path[128];
   Config_Item *ci2;
   int i = 0;

   if (!ci->ng)
     return;
   
   if (ci->config_dialog)
      return;

   EINA_LIST_FOREACH(ngi_config->items, l, ci2)
   if (ci == ci2)
      break;
   else i++;

   snprintf(path, sizeof(path), "extensions/engage::%d", i);
   if (e_config_dialog_find("E", path))
      return;

   v = E_NEW(E_Config_Dialog_View, 1);

   /* Dialog Methods */
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = NULL;
   v->advanced.create_widgets = NULL;

   /* Create The Dialog */
   snprintf(buf, sizeof(buf), "%s/e-module-ng.edj", e_module_dir_get(ngi_config->module));
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
                             D_("Engage Configuration"),
                             "E", path, buf, 0, v, ci);

   ci->config_dialog = cfd;
   ngi_bar_lock(ci->ng, 1);
}

static void
_fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata)
{
#define C(_name) cfdata->_name = ci->_name;
   C(show_background);
   C(show_label);
   C(orient);
   C(size);
   C(zoomfactor);
   C(autohide);
   C(autohide_show_urgent);
   C(hide_below_windows);
   C(hide_timeout);
   C(zoom_duration);
   C(zoom_range);
   C(alpha);
   C(stacking);
   C(mouse_over_anim);
   C(lock_deskswitch);
   C(ecomorph_features);
#undef C

   cfdata->cfg = ci;
   cfdata->cfd = ci->config_dialog;

   cfdata->ilist = NULL;
   cfdata->tlist_box = NULL;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;
   Config_Item *ci;

   ci = (Config_Item *)cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(ci, cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   cfdata->cfg->config_dialog = NULL;

   if (cfdata->cfg->ng)
     ngi_bar_lock(cfdata->cfg->ng, 0);

   free(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   E_Radio_Group *rg;
   Evas_Object *ol, *of, *ob, *ot, *otb;

   otb = e_widget_toolbook_add(evas, 0, 0); 
   
   /* _______ first column __________________________________________________*/
   ol = e_widget_list_add(evas, 0, 0); 
   of = e_widget_framelist_add(evas, D_("Appearance"), 0);
   ob = e_widget_label_add (evas, D_("Icon Size:"));
   e_widget_framelist_object_append (of, ob);
   ob = e_widget_slider_add (evas, 1, 0, D_("%1.0f px"), 16.0, 128,
                             1.0, 0, NULL, &(cfdata->size), 100);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_framelist_object_append (of, ob);

   /* ob = e_widget_label_add (evas, D_("Reflection Opacity:"));
    * e_widget_framelist_object_append (of, ob);
    * ob = e_widget_slider_add (evas, 1, 0, D_("%1.0f"), 0, 255,
    *                           1.0, 0, NULL, &(cfdata->rflxn_alpha), 100);
    * e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
    * e_widget_framelist_object_append (of, ob); */

   if (ngi_config->use_composite)
     {
        ob = e_widget_label_add (evas, D_("Background Opacity:"));
        e_widget_framelist_object_append (of, ob);
        ob = e_widget_slider_add (evas, 1, 0, D_("%1.0f \%"), 0, 255,
                                  1.0, 0, NULL, &(cfdata->alpha), 100);
        e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
        e_widget_framelist_object_append (of, ob);
     }
   else
     cfdata->alpha = 255;

   ob = e_widget_check_add(evas, D_("Show Background Box"), &(cfdata->show_background));
   e_widget_framelist_object_append(of, ob);

   ob = e_widget_check_add(evas, D_("Show Mouse-Over Animation"), &(cfdata->mouse_over_anim));
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(ol, of, 1, 0, 0.0);
   
   of = e_widget_framelist_add(evas, D_("Orientation"), 0); 
   rg = e_widget_radio_group_new(&(cfdata->orient));
   ob = e_widget_radio_add(evas, D_("Bottom Edge"), E_GADCON_ORIENT_BOTTOM, rg);
   e_widget_framelist_object_append(of, ob); 
   ob = e_widget_radio_add(evas, D_("Top Edge"), E_GADCON_ORIENT_TOP, rg);
   e_widget_framelist_object_append(of, ob); 
   ob = e_widget_radio_add(evas, D_("Left Edge"), E_GADCON_ORIENT_LEFT, rg);
   e_widget_framelist_object_append(of, ob); 
   ob = e_widget_radio_add(evas, D_("Right Edge"), E_GADCON_ORIENT_RIGHT, rg); 
   e_widget_framelist_object_append(of, ob);

   /* of = e_widget_frametable_add(evas, D_("Orientation"), 1);
    * ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_left",
    * 				24, 24, E_GADCON_ORIENT_LEFT, rg);
    * e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 0);
    * ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_right",
    * 				24, 24, E_GADCON_ORIENT_RIGHT, rg);
    * e_widget_frametable_object_append(of, ob, 2, 1, 1, 1, 1, 1, 1, 0);
    * ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_top",
    * 				24, 24, E_GADCON_ORIENT_TOP, rg);
    * e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 0);
    * ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_bottom",
    * 				24, 24, E_GADCON_ORIENT_BOTTOM, rg);
    * e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 1, 1, 0); */

   e_widget_list_object_append(ol, of, 1, 0, 0.0); 
   e_widget_toolbook_page_append(otb, NULL, D_("Appearance"), ol, 1, 1, 1, 1, 0.5, 0.5); 

   
   of = e_widget_framelist_add(evas, D_("Zoom"), 0);
   ob = e_widget_label_add (evas, D_("Factor:"));
   e_widget_framelist_object_append (of, ob);
   ob = e_widget_slider_add (evas, 1, 0, "%1.2f", 1.0, 5.0,
                             0.01, 0, &(cfdata->zoomfactor), NULL, 100);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_framelist_object_append (of, ob);
   ob = e_widget_label_add (evas, D_("Range:"));
   e_widget_framelist_object_append (of, ob);
   ob = e_widget_slider_add (evas, 1, 0, "%1.2f", 1.0, 4.0,
                             0.01, 0, &(cfdata->zoom_range), NULL, 100);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_framelist_object_append (of, ob);
   ob = e_widget_label_add (evas, D_("Duration:"));
   e_widget_framelist_object_append (of, ob);
   ob = e_widget_slider_add (evas, 1, 0, "%1.2f", 0.1, 0.5,
                             0.01, 0, &(cfdata->zoom_duration), NULL, 100);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_framelist_object_append (of, ob);
   e_widget_toolbook_page_append(otb, NULL, D_("Zoom"), of, 1, 0, 1, 0, 0.5, 0.0); 


   of = e_widget_frametable_add(evas, D_("Bar Items"), 0);
   ol = e_widget_ilist_add(evas, 0, 0, NULL);
   cfdata->ilist = ol;
   _load_box_tlist(cfdata);
   e_widget_size_min_set(ol, 300, 80);
   e_widget_frametable_object_append(of, ol, 0, 0, 1, 1, 1, 1, 1, 1);

   ot = e_widget_table_add(evas, 0);
   ob = e_widget_button_add(evas, D_("Delete"), "widget/del", _cb_box_del, cfdata, NULL);
   e_widget_table_object_append(ot, ob, 0, 1, 1, 1, 1, 1, 1, 0);
   ob = e_widget_button_add(evas, D_("Configure"), "widget/config", _cb_box_config, cfdata, NULL);
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 1, 1, 1, 0);
   ob = e_widget_button_add(evas, D_("Up"), "widget/up_arrow", _cb_box_up, cfdata, NULL);
   e_widget_table_object_append(ot, ob, 1, 0, 1, 1, 1, 1, 1, 0);
   ob = e_widget_button_add(evas, D_("Down"), "widget/down_arrow", _cb_box_down, cfdata, NULL);
   e_widget_table_object_append(ot, ob, 1, 1, 1, 1, 1, 1, 1, 0);
   e_widget_frametable_object_append(of, ot, 0, 1, 1, 1, 1, 1, 1, 0);

   ot = e_widget_table_add(evas, 0);
   ob = e_widget_label_add(evas, D_("Add"));
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 1, 1, 1, 0);
   ob = e_widget_button_add(evas, D_("Taskbar"), "widget/add", _cb_box_add_taskbar, cfdata, NULL);
   e_widget_table_object_append(ot, ob, 0, 1, 1, 1, 1, 1, 1, 0);
   ob = e_widget_button_add(evas, D_("Launcher"), "widget/add", _cb_box_add_launcher, cfdata, NULL);
   e_widget_table_object_append(ot, ob, 1, 1, 1, 1, 1, 1, 1, 0);
   ob = e_widget_button_add(evas, D_("Gadcon"), "widget/add", _cb_box_add_gadcon, cfdata, NULL);
   e_widget_table_object_append(ot, ob, 0, 2, 1, 1, 1, 1, 1, 0);
   e_widget_frametable_object_append(of, ot, 0, 2, 1, 1, 1, 1, 1, 0);
   e_widget_toolbook_page_append(otb, NULL, D_("Bar Items"), of, 1, 1, 1, 1, 0.5, 0.5); 


   of = e_widget_framelist_add(evas, D_("Auto Hide"), 0);
   cfdata->hide_mode = cfdata->autohide;

   rg = e_widget_radio_group_new(&cfdata->hide_mode);
   ob = e_widget_radio_add(evas, D_("None"),
                           AUTOHIDE_NONE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Autohide"),
                           AUTOHIDE_NORMAL, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Hide when Window overlaps"),
                           AUTOHIDE_OVERLAP, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Hide on Fullscreen"),
                           AUTOHIDE_FULLSCREEN, rg);
   e_widget_framelist_object_append(of, ob);

   ob = e_widget_check_add(evas, D_("Show Bar when Urgent"), &(cfdata->autohide_show_urgent));
   e_widget_framelist_object_append(of, ob);
   e_widget_toolbook_page_append(otb, NULL, D_("Auto Hide"), of, 1, 0, 1, 0, 0.5, 0.0); 

   
   /* e_widget_toolbook_page_append(otb, NULL, D_("Orientation"), of, 1, 1, 1, 0, 0.5, 0.0); */

   ol = e_widget_list_add(evas, 0, 0); 
   of = e_widget_framelist_add(evas, D_("Stacking"), 0);
   rg = e_widget_radio_group_new(&cfdata->stacking);
   ob = e_widget_radio_add(evas, D_("Above All"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Below Fullscreen"), 1, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 0, 0.0); 
   
   of = e_widget_framelist_add(evas, D_("Other"), 0);
   ob = e_widget_check_add(evas, D_("Ecomorph Features"), &(cfdata->ecomorph_features));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, D_("Show Icon Label"), &(cfdata->show_label));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 0, 0.0);
   
   e_widget_toolbook_page_append(otb, NULL, D_("Other"), ol, 1, 0, 1, 0, 0.5, 0.0);

   e_widget_toolbook_page_show(otb, 0); 


   return otb;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Config_Item *ci;
   Ng *ng;

   ci = cfd->data;
   ng = ci->ng;

   ci->size = cfdata->size;
   ci->zoomfactor = cfdata->zoomfactor;
   ci->hide_timeout = cfdata->hide_timeout;
   ci->zoom_duration = cfdata->zoom_duration;
   ci->zoom_range = cfdata->zoom_range;
   ci->alpha = cfdata->alpha;
   ci->mouse_over_anim = cfdata->mouse_over_anim;
   ci->autohide = cfdata->hide_mode;
   ci->lock_deskswitch = cfdata->lock_deskswitch;
   ci->ecomorph_features = cfdata->ecomorph_features;
   ci->show_background = cfdata->show_background;
   ci->show_label = cfdata->show_label;
   ci->stacking = cfdata->stacking;
   ci->autohide_show_urgent = cfdata->autohide_show_urgent;
   
   if (ci->show_label)
     evas_object_show(ng->o_label);
   else
     evas_object_hide(ng->o_label);

   if (ci->show_background)
     evas_object_show(ng->bg_clip);
   else
     evas_object_hide(ng->bg_clip);

   if (ci->orient != cfdata->orient)
     {
        ci->orient = cfdata->orient;
	ngi_free(ng);
	ngi_new(ci);
     }
   else
     {
        if (ci->autohide == AUTOHIDE_NORMAL)
          ng->hide = EINA_TRUE;
        if (ng->es)
          {
             ng->es->cfg->autohide = ng->cfg->autohide;
             ng->es->cfg->overlap = (ng->cfg->autohide == AUTOHIDE_OVERLAP);
          }
	ng->hide_step = 0;
	ng->hide_state = show;
	ngi_reposition(ng);
	ngi_input_extents_calc(ng);
	ngi_thaw(ng);
     }

   e_config_domain_save("module.ng", ngi_conf_edd, ngi_config);

   return 1;
}

static void
_update_boxes(Ng *ng)
{
   Eina_List *l;
   Ngi_Box *box;
   Config_Box *cfg_box;

   while(ng->boxes)
     {
        box = ng->boxes->data;
        if (box->cfg->type == taskbar)
           ngi_taskbar_remove(box);
        else if (box->cfg->type == launcher)
           ngi_launcher_remove(box);
        else if (box->cfg->type == gadcon)
           ngi_gadcon_remove(box);
     }

   ngi_freeze(ng);

   EINA_LIST_FOREACH (ng->cfg->boxes, l, cfg_box)
   {
      switch (cfg_box->type)
        {
         case launcher:
            ngi_launcher_new(ng, cfg_box);
            break;

         case taskbar:
            ngi_taskbar_new(ng, cfg_box);
            break;

         case gadcon:
            ngi_gadcon_new(ng, cfg_box);
            break;
        } /* switch */
   }

   ngi_thaw(ng);
}

/***************************************************************************************/

static void
_cb_box_add_taskbar(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   Config_Box *cfg_box;

   cfg_box = E_NEW(Config_Box, 1);
   cfg_box->type = taskbar;
   cfg_box->taskbar_adv_bordermenu = 0;
   cfg_box->taskbar_skip_dialogs = 0;
   cfg_box->taskbar_skip_dialogs = 1;
   cfg_box->taskbar_show_iconified = 1;
   cfg_box->taskbar_show_desktop = 0;

   cfdata->cfg->boxes = eina_list_append(cfdata->cfg->boxes, cfg_box);

   _update_boxes(cfdata->cfg->ng);

   _load_box_tlist(cfdata);
}

static void
_cb_box_add_launcher(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   Config_Box *cfg_box;

   cfg_box = E_NEW(Config_Box, 1);
   cfg_box->type = launcher;
   cfg_box->launcher_app_dir = eina_stringshare_add("default");
   cfdata->cfg->boxes = eina_list_append(cfdata->cfg->boxes, cfg_box);

   _update_boxes(cfdata->cfg->ng);

   _load_box_tlist(cfdata);
}

static void
_cb_box_add_gadcon(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   Config_Box *cfg_box;

   cfg_box = E_NEW(Config_Box, 1);
   cfg_box->type = gadcon;
   //cfg_box->gadcon_items = eina_stringshare_add("ng_gadcon-tmp");
   cfdata->cfg->boxes = eina_list_append(cfdata->cfg->boxes, cfg_box);

   _update_boxes(cfdata->cfg->ng);

   _load_box_tlist(cfdata);
}

static void
_cb_box_del(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   int selected = e_widget_ilist_selected_get(cfdata->ilist);

   if (selected < 0)
      return;

   Eina_List *boxes = cfdata->cfg->boxes;

   Config_Box *cfg_box = eina_list_nth(boxes, selected);

   if (!cfg_box)
      return;

   cfdata->cfg->boxes = eina_list_remove(boxes, cfg_box);

   switch(cfg_box->type)
     {
      case launcher:
         ngi_launcher_remove(cfg_box->box);
         break;

      case taskbar:
         ngi_taskbar_remove(cfg_box->box);
         break;

      case gadcon:
         ngi_gadcon_remove(cfg_box->box);
     } /* switch */

   ngi_thaw(cfdata->cfg->ng);

   _load_box_tlist(cfdata);
}

static Evas_Object *
_basic_create_box_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ol, *of, *ob, *ot;
   E_Radio_Group *rg;

   o = e_widget_list_add(evas, 0, 0);
   if (cfdata->cfg_box->type == launcher)
     {
        cfdata->app_dir = eina_stringshare_add(cfdata->cfg_box->launcher_app_dir);

        of = e_widget_frametable_add(evas, D_("Launcher Settings"), 0);
        ol = e_widget_ilist_add(evas, 0, 0, &cfdata->app_dir);
        cfdata->tlist_box = ol;
        _load_ilist(cfdata);
        e_widget_size_min_set(ol, 140, 140);
        e_widget_frametable_object_append(of, ol, 0, 0, 1, 2, 1, 1, 1, 1);

        ot = e_widget_table_add(evas, 0);
        ob = e_widget_button_add(evas, D_("Add"), "widget/add", _cb_add, cfdata, NULL);
        e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 1, 1, 1, 0);
        ob = e_widget_button_add(evas, D_("Delete"), "widget/del", _cb_del, cfdata, NULL);
        e_widget_table_object_append(ot, ob, 0, 1, 1, 1, 1, 1, 1, 0);
        ob = e_widget_button_add(evas, D_("Configure"), "widget/config", _cb_config, cfdata, NULL);
        e_widget_table_object_append(ot, ob, 0, 2, 1, 1, 1, 1, 1, 0);

        e_widget_frametable_object_append(of, ot, 2, 0, 1, 1, 1, 1, 1, 0);
        e_widget_list_object_append(o, of, 0, 1, 0.5);
     }
   else if (cfdata->cfg_box->type == taskbar)
     {
        of = e_widget_framelist_add(evas, D_("Taskbar Settings"), 0);

        ob = e_widget_check_add(evas, D_("Don't Show Dialogs"),
				&(cfdata->cfg_box->taskbar_skip_dialogs));
        e_widget_framelist_object_append(of, ob);
        ob = e_widget_check_add(evas, D_("Advanced Window Menu"),
				&(cfdata->cfg_box->taskbar_adv_bordermenu));
        e_widget_framelist_object_append(of, ob);
        ob = e_widget_check_add(evas, D_("Only Show Current Desk"),
				&(cfdata->cfg_box->taskbar_show_desktop));
        e_widget_framelist_object_append(of, ob);
        ob = e_widget_check_add(evas, D_("Group Applications by Window Class"),
				&(cfdata->cfg_box->taskbar_group_apps));
        e_widget_framelist_object_append(of, ob);
        ob = e_widget_check_add(evas, D_("Append new Applications on the right Side"),
				&(cfdata->cfg_box->taskbar_append_right));
        e_widget_framelist_object_append(of, ob);

        e_widget_list_object_append(o, of, 1, 1, 0.5);

        of = e_widget_framelist_add(evas, D_("Iconified Applications"), 0);
        rg = e_widget_radio_group_new(&cfdata->cfg_box->taskbar_show_iconified);
        ob = e_widget_radio_add(evas, D_("Not Shown"), 0, rg);
        e_widget_framelist_object_append(of, ob);
        ob = e_widget_radio_add(evas, D_("Show"), 1, rg);
        e_widget_framelist_object_append(of, ob);
        ob = e_widget_radio_add(evas, D_("Only Iconified"), 2, rg);
        e_widget_framelist_object_append(of, ob);

        e_widget_list_object_append(o, of, 1, 1, 0.5);
     }

   return o;
}

static int
_basic_apply_box_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   eina_stringshare_replace(&cfdata->cfg_box->launcher_app_dir, cfdata->app_dir);

   _update_boxes(cfdata->cfg->ng);
   return 1;
}

/* urgh */
static void *
_create_box_data(E_Config_Dialog *cfd)
{
   return cfd->data;
}

static void
_free_box_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   eina_stringshare_del(cfdata->app_dir);
}

static void
_cb_box_config(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;

   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   char buf[4096];

   int selected = e_widget_ilist_selected_get(cfdata->ilist);

   if (selected < 0)
      return;

   Eina_List *boxes = cfdata->cfg->boxes;

   cfdata->cfg_box = eina_list_nth(boxes, selected);

   if (!cfdata->cfg_box)
      return;

   if (cfdata->cfg_box->type == gadcon)
     {
        ngi_gadcon_config(cfdata->cfg_box->box);
     }
   else
     {
        v = E_NEW(E_Config_Dialog_View, 1);

        v->create_cfdata = _create_box_data;
        v->free_cfdata = _free_box_data;
        v->basic.apply_cfdata = _basic_apply_box_data;
        v->basic.create_widgets = _basic_create_box_widgets;
        v->advanced.apply_cfdata = NULL;
        v->advanced.create_widgets = NULL;

        snprintf(buf, sizeof(buf), "%s/e-module-ng.edj", e_module_dir_get(ngi_config->module));
        cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
                                  D_("Engage Bar Configuration"),
                                  "e", "_e_mod_ngi_config_dialog_add_box", buf, 0, v, cfdata);
     }
}

static void *
_create_data2(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;
   Config_Item *ci;

   Ngi_Box *box = cfd->data;

   ci = (Config_Item *)box->ng->cfg;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(ci, cfdata);
   cfdata->cfg_box = box->cfg;

   return cfdata;
}

void
ngi_configure_box(Ngi_Box *box)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   char buf[4096];

   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data2;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_box_data;
   v->basic.create_widgets = _basic_create_box_widgets;
   v->advanced.apply_cfdata = NULL;
   v->advanced.create_widgets = NULL;

   snprintf(buf, sizeof(buf), "%s/e-module-ng.edj", e_module_dir_get(ngi_config->module));
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
                             D_("Engage Bar Configuration"),
                             "e", "_e_mod_ngi_config_dialog_add_box", buf, 0, v, box);
}

static void
_cb_box_up(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   int selected = e_widget_ilist_selected_get(cfdata->ilist);

   if (selected < 0)
      return;

   Eina_List *boxes = cfdata->cfg->boxes;

   Eina_List *l = eina_list_nth_list(boxes, selected);

   if (!l || !l->prev)
      return;

   Config_Box *cfg_box = l->data;

   boxes = eina_list_remove(boxes, cfg_box);
   cfdata->cfg->boxes = eina_list_prepend_relative(boxes, cfg_box, l->prev->data);

   _update_boxes(cfdata->cfg->ng);

   _load_box_tlist(cfdata);

   e_widget_ilist_selected_set(cfdata->ilist, selected - 1);
}

static void
_cb_box_down(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   int selected = e_widget_ilist_selected_get(cfdata->ilist);

   if (selected < 0)
      return;

   Eina_List *boxes = cfdata->cfg->boxes;

   Eina_List *l = eina_list_nth_list(boxes, selected);

   if (!l || !l->next)
      return;

   Config_Box *cfg_box = l->data;

   boxes = eina_list_remove(boxes, cfg_box);
   cfdata->cfg->boxes = eina_list_append_relative(boxes, cfg_box, l->next->data);

   _update_boxes(cfdata->cfg->ng);

   _load_box_tlist(cfdata);

   e_widget_ilist_selected_set(cfdata->ilist, selected + 1);
}

static void
_load_box_tlist(E_Config_Dialog_Data *cfdata)
{
   char buf[4096];

   e_widget_ilist_clear(cfdata->ilist);

   int cnt = 1;
   Eina_List *l;
   Config_Box *cfg_box;
   char *blub;

   for(l = cfdata->cfg->boxes; l; l = l->next)
     {
        cfg_box = l->data;
        switch (cfg_box->type)
          {
           case launcher:
              snprintf(buf, sizeof(buf), "%i Launcher", cnt);
              blub = strdup(buf);

              e_widget_ilist_append(cfdata->ilist, NULL, blub, NULL, cfg_box, blub);
              break;

           case taskbar:
              snprintf(buf, sizeof(buf), "%i Taskbar", cnt);
              blub = strdup(buf);
              e_widget_ilist_append(cfdata->ilist, NULL, blub, NULL, cfg_box, blub);
              break;

           case gadcon:
              snprintf(buf, sizeof(buf), "%i Gadcon", cnt);
              blub = strdup(buf);
              e_widget_ilist_append(cfdata->ilist, NULL, blub, NULL, cfg_box, blub);
              break;
          } /* switch */
        cnt++;
     }
   e_widget_ilist_go(cfdata->ilist);
}

static void
_cb_add(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   e_entry_dialog_show(D_("Create new source"), "enlightenment/e",
                       D_("Enter a name for this new Application Launcher:"), "", NULL, NULL,
                       _cb_entry_ok, NULL, cfdata);
}

static void
_cb_del(void *data, void *data2)
{
   char buf[4096];
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   snprintf(buf, sizeof(buf), D_("You requested to delete \"%s\".<br><br>"
                                 "Are you sure you want to delete this source?"),
            cfdata->app_dir);

   e_confirm_dialog_show(D_("Are you sure you want to delete this source?"),
                         "enlightenment/exit", buf, NULL, NULL,
                         _cb_confirm_dialog_yes, NULL, cfdata, NULL, NULL, NULL);
}

static void
_cb_config(void *data, void *data2)
{
   char path[4096];
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   snprintf(path, sizeof(path), "%s/.e/e/applications/bar/%s/.order",
            e_user_homedir_get(), cfdata->app_dir);

   e_configure_registry_call("internal/ibar_other",
                             e_container_current_get(e_manager_current_get()),
                             path);
}

static void
_cb_entry_ok(void *data, char *text)
{
   char buf[4096];
   char tmp[4096];
   FILE *f;

   snprintf(buf, sizeof(buf), "%s/.e/e/applications/bar/%s",
            e_user_homedir_get(), text);

   if (!ecore_file_exists(buf))
     {
        ecore_file_mkdir(buf);

        snprintf(buf, sizeof(buf), "%s/.e/e/applications/bar/%s/.order",
                 e_user_homedir_get(), text);

        f = fopen(buf, "w");
        if (f)
          {
             // Populate this .order file with some defaults
             snprintf(tmp, sizeof(tmp), "xterm.desktop\n"
                                        "firefox.desktop\n"
                                        "gimp.desktop\n" "xmms.desktop\n");
             fwrite(tmp, sizeof(char), strlen(tmp), f);
             fclose(f);
          }
     }

   _load_ilist((E_Config_Dialog_Data *)data);
}

static void
_cb_confirm_dialog_yes(void *data)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   char buf[4096];

   snprintf(buf, sizeof(buf), "%s/.e/e/applications/bar/%s", e_user_homedir_get(), cfdata->app_dir);

   if (ecore_file_is_dir(buf))
      ecore_file_recursive_rm(buf);

   _load_ilist(cfdata);
}

static void
_load_ilist(E_Config_Dialog_Data *cfdata)
{
   Eina_List *dirs, *l;
   char buf[4096], *file;
   int selnum = -1;
   const char *home;
   int i = 0;
   e_widget_ilist_clear(cfdata->tlist_box);

   home = e_user_homedir_get();
   snprintf(buf, sizeof(buf), "%s/.e/e/applications/bar", home);
   dirs = ecore_file_ls(buf);

   EINA_LIST_FOREACH(dirs, l, file)
   {
      if (file[0] == '.')
         continue;

      snprintf(buf, sizeof(buf), "%s/.e/e/applications/bar/%s", home, file);
      if (ecore_file_is_dir(buf))
        {
           e_widget_ilist_append(cfdata->tlist_box, NULL, file, NULL, NULL, file);
           if ((cfdata->app_dir) && (!strcmp(cfdata->app_dir, file)))
              selnum = i;

           i++;
        }
   }

   e_widget_ilist_go(cfdata->tlist_box);
   if (selnum >= 0)
      e_widget_ilist_selected_set(cfdata->tlist_box, selnum);
}

static void
_cb_slider_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data *)data;
   Ng *ng = cfdata->cfg->ng;
   int a;

   ng->cfg->size = (int)cfdata->size;
   ng->size = ng->cfg->size;

   ng->cfg->zoomfactor = cfdata->zoomfactor;
   ng->cfg->zoom_duration = cfdata->zoom_duration;
   ng->cfg->zoom_range = cfdata->zoom_range;
   ng->cfg->hide_timeout = cfdata->hide_timeout;
   ng->cfg->alpha = cfdata->alpha;

   a = ng->cfg->alpha;
   evas_object_color_set(ng->bg_clip, a, a, a, a);
   
   ngi_win_position_calc(ng->win);
   ngi_thaw(ng);
}

