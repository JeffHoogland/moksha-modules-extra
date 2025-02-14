#include "e.h"
#include "e_mod_main.h"

/* local function prototypes */
static void *_create_data(E_Config_Dialog *cfd __UNUSED__);
static void _fill_data(E_Config_Dialog_Data *cfdata);
static void _free_data(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd __UNUSED__, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_check_changed(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata);
static int _adv_apply(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata);
static Evas_Object *_adv_create(E_Config_Dialog *cfd __UNUSED__, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _adv_check_changed(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata);
static void _cb_animate_changed(void *data, Evas_Object *obj __UNUSED__);

struct _E_Config_Dialog_Data
{
   /* Basic */
   int max_exe_list, max_eap_list, max_hist_list;
   int scroll_animate;
   /* Advanced */
   double scroll_speed, pos_align_x, pos_align_y;
   double pos_size_w, pos_size_h;
   int pos_min_w, pos_min_h, pos_max_w, pos_max_h;
   char *term_cmd;
   struct
     {
	Evas_Object *scroll_speed_lbl, *scroll_speed_slider;
     } gui;
};

E_Config_Dialog *
exebuf_config_dialog(E_Container *con, const char *params __UNUSED__)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "advanced/run_command")) return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.check_changed = _basic_check_changed;
   v->advanced.create_widgets = _adv_create;
   v->advanced.apply_cfdata = _adv_apply;
   v->advanced.check_changed = _adv_check_changed;

   cfd = e_config_dialog_new(con, D_("Run Command Settings"), "E",
			     "advanced/run_command", "system-run", 0, v, NULL);
   return cfd;
}

/* local function prototypes */
static void *
_create_data(E_Config_Dialog *cfd __UNUSED__)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->max_exe_list = exebuf_conf->max_exe_list;
   cfdata->max_eap_list = exebuf_conf->max_eap_list;
   cfdata->max_hist_list = exebuf_conf->max_hist_list;
   cfdata->scroll_animate = exebuf_conf->scroll_animate;
   cfdata->scroll_speed = exebuf_conf->scroll_speed;
   cfdata->pos_align_x = exebuf_conf->pos_align_x;
   cfdata->pos_align_y = exebuf_conf->pos_align_y;
   cfdata->pos_size_w = exebuf_conf->pos_size_w;
   cfdata->pos_size_h = exebuf_conf->pos_size_h;
   cfdata->pos_min_w = exebuf_conf->pos_min_w;
   cfdata->pos_min_h = exebuf_conf->pos_min_h;
   cfdata->pos_max_w = exebuf_conf->pos_max_w;
   cfdata->pos_max_h = exebuf_conf->pos_max_h;
   if (exebuf_conf->term_cmd)
     cfdata->term_cmd = strdup(exebuf_conf->term_cmd);
}

static void
_free_data(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata)
{
   E_FREE(cfdata->term_cmd);
   E_FREE(cfdata);
}

static int
_basic_apply(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata)
{
   exebuf_conf->max_exe_list = cfdata->max_exe_list;
   exebuf_conf->max_eap_list = cfdata->max_eap_list;
   exebuf_conf->max_hist_list = cfdata->max_hist_list;
   exebuf_conf->scroll_animate = cfdata->scroll_animate;
   e_config_save_queue();
   return 1;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd __UNUSED__, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ow;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, D_("General"), 0);
   ow = e_widget_label_add(evas, D_("Maximum Apps to List"));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_slider_add(evas, 1, 0, D_("%1.0f"), 10, 50, 5, 0, NULL,
			    &(cfdata->max_eap_list), 100);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_label_add(evas, D_("Maximum Exes to List"));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_slider_add(evas, 1, 0, D_("%1.0f"), 10, 50, 5, 0, NULL,
			    &(cfdata->max_exe_list), 100);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_label_add(evas, D_("Maximum History to List"));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_slider_add(evas, 1, 0, D_("%1.0f"), 10, 200, 5, 0, NULL,
			    &(cfdata->max_hist_list), 100);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(o, of, 1, 0, 0.5);

   of = e_widget_framelist_add(evas, D_("Animations"), 0);
   ow = e_widget_check_add(evas, D_("Animate"), &(cfdata->scroll_animate));
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(o, of, 1, 0, 0.5);

   return o;
}

static int
_basic_check_changed(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata)
{
#define CHK_CHG(_e_config, _cfdata) \
   if (exebuf_conf->_e_config != cfdata->_cfdata) return 1;

   CHK_CHG(max_exe_list, max_exe_list);
   CHK_CHG(max_eap_list, max_eap_list);
   CHK_CHG(max_hist_list, max_hist_list);
   CHK_CHG(scroll_animate, scroll_animate);

#undef CHK_CHG

   return 0;
}

static int
_adv_apply(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata)
{
   exebuf_conf->max_exe_list = cfdata->max_exe_list;
   exebuf_conf->max_eap_list = cfdata->max_eap_list;
   exebuf_conf->max_hist_list = cfdata->max_hist_list;
   exebuf_conf->scroll_animate = cfdata->scroll_animate;
   exebuf_conf->scroll_speed = cfdata->scroll_speed;
   exebuf_conf->pos_align_x = cfdata->pos_align_x;
   exebuf_conf->pos_align_y = cfdata->pos_align_y;
   exebuf_conf->pos_min_w = cfdata->pos_min_w;
   exebuf_conf->pos_min_h = cfdata->pos_min_h;
   exebuf_conf->pos_max_w = cfdata->pos_max_w;
   exebuf_conf->pos_max_h = cfdata->pos_max_h;
   eina_stringshare_replace(&exebuf_conf->term_cmd, cfdata->term_cmd);
   e_config_save_queue();
   return 1;
}

static Evas_Object *
_adv_create(E_Config_Dialog *cfd __UNUSED__, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *otb, *ol, *ow;

   otb = e_widget_toolbook_add(evas, (48 * e_scale), (48 * e_scale));

   /* General Page */
   ol = e_widget_list_add(evas, 0, 0);
   ow = e_widget_label_add(evas, D_("Maximum Apps to List"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%1.0f"), 10, 50, 5, 0, NULL,
			    &(cfdata->max_eap_list), 100);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_label_add(evas, D_("Maximum Exes to List"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%1.0f"), 10, 50, 5, 0, NULL,
			    &(cfdata->max_exe_list), 100);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_label_add(evas, D_("Maximum History to List"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%1.0f"), 10, 200, 5, 0, NULL,
			    &(cfdata->max_hist_list), 100);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   e_widget_toolbook_page_append(otb, NULL, D_("General"), ol, 1, 0, 1, 0,
				 0.5, 0.0);

   /* Scroll Page */
   ol = e_widget_list_add(evas, 0, 0);
   ow = e_widget_check_add(evas, D_("Animate"), &(cfdata->scroll_animate));
   e_widget_on_change_hook_set(ow, _cb_animate_changed, cfdata);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_label_add(evas, D_("Speed"));
   cfdata->gui.scroll_speed_lbl = ow;
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%1.2f"), 0.0, 1.0, 0.01, 0,
			    &(cfdata->scroll_speed), NULL, 100);
   cfdata->gui.scroll_speed_slider = ow;
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   e_widget_toolbook_page_append(otb, NULL, D_("Animations"), ol, 1, 0, 1, 0,
				 0.5, 0.0);

   /* Size Page */
   ol = e_widget_list_add(evas, 0, 0);
   ow = e_widget_label_add(evas, D_("Minimum Width"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%4.0f"), 0, 4000, 50, 0,
			    NULL, &(cfdata->pos_min_w), 100);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_label_add(evas, D_("Minimum Height"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%4.0f"), 0, 4000, 50, 0,
			    NULL, &(cfdata->pos_min_h), 100);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_label_add(evas, D_("Maximum Width"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%4.0f"), 0, 4000, 50, 0,
			    NULL, &(cfdata->pos_max_w), 100);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_label_add(evas, D_("Maximum Height"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%4.0f"), 0, 4000, 50, 0,
			    NULL, &(cfdata->pos_max_h), 100);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_label_add(evas, D_("X-Axis Alignment"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%1.2f"), 0.0, 1.0, 0.01, 0,
			    &(cfdata->pos_align_x), NULL, 100);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_label_add(evas, D_("Y-Axis Alignment"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, D_("%1.2f"), 0.0, 1.0, 0.01, 0,
			    &(cfdata->pos_align_y), NULL, 100);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   e_widget_toolbook_page_append(otb, NULL, D_("Geometry"), ol, 1, 0, 1, 0,
				 0.5, 0.0);

   /* Terminal Page */
   ol = e_widget_list_add(evas, 0, 0);
   ow = e_widget_label_add(evas, D_("Command (CTRL+RETURN to utilize)"));
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   ow = e_widget_entry_add(evas, &(cfdata->term_cmd), NULL, NULL, NULL);
   e_widget_list_object_append(ol, ow, 1, 0, 0.5);
   e_widget_toolbook_page_append(otb, NULL, D_("Terminal"), ol, 1, 0, 1, 0,
				 0.5, 0.0);

   e_widget_toolbook_page_show(otb, 0);
   return otb;
}

static int
_adv_check_changed(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata)
{
#define CHK_CHG(_e_config, _cfdata) \
   if (exebuf_conf->_e_config != cfdata->_cfdata) return 1;

   CHK_CHG(max_exe_list, max_exe_list);
   CHK_CHG(max_eap_list, max_eap_list);
   CHK_CHG(max_hist_list, max_hist_list);
   CHK_CHG(scroll_animate, scroll_animate);
   CHK_CHG(scroll_speed, scroll_speed);
   CHK_CHG(pos_align_x, pos_align_x);
   CHK_CHG(pos_align_y, pos_align_y);
   CHK_CHG(pos_min_w, pos_min_w);
   CHK_CHG(pos_min_h, pos_min_h);
   CHK_CHG(pos_max_w, pos_max_w);
   CHK_CHG(pos_max_h, pos_max_h);

   if (strcmp(exebuf_conf->term_cmd, cfdata->term_cmd)) return 1;
#undef CHK_CHG

   return 0;
}

static void
_cb_animate_changed(void *data, Evas_Object *obj __UNUSED__)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;
   e_widget_disabled_set(cfdata->gui.scroll_speed_lbl, !cfdata->scroll_animate);
   e_widget_disabled_set(cfdata->gui.scroll_speed_slider, !cfdata->scroll_animate);
}
