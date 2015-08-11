#include <e.h>
#include "e_mod_main.h"

#define MODE_NORMAL 0
#define MODE_WINDOW 1
#define MODE_REGION 2

struct _E_Config_Dialog_Data 
{
   double delay;
   int prompt, use_app, use_bell;
   int quality, use_thumb, thumb_size;
   int mode;
   char *location, *filename, *app;
};

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_adv_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _adv_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _cb_disable_check(void *data, Evas_Object *obj);

E_Config_Dialog *
e_int_config_screenshot_module(E_Container *con, const char *params)
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;
   char buf[PATH_MAX];

   if (e_config_dialog_find("Screenshot", "extensions/screenshot")) 
     return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;
   v->advanced.create_widgets = _adv_create;
   v->advanced.apply_cfdata = _adv_apply;

   snprintf(buf, sizeof(buf), "%s/e-module-screenshot.edj", ss_cfg->mod_dir);
   cfd = e_config_dialog_new(con, D_("Screenshot Settings"), "Screenshot", 
			     "extensions/screenshot", buf, 0, 
                             v, NULL);
   ss_cfg->cfd = cfd;
   return cfd;
}

/* private functions */
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
   ss_cfg->cfd = NULL;
   E_FREE(cfdata->location);
   E_FREE(cfdata->filename);
   E_FREE(cfdata->app);
   E_FREE(cfdata);
}

static void 
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->delay = ss_cfg->delay;
   cfdata->prompt = ss_cfg->prompt;
   cfdata->use_app = ss_cfg->use_app;
   cfdata->use_bell = ss_cfg->use_bell;
   cfdata->quality = ss_cfg->quality;
   cfdata->use_thumb = ss_cfg->use_thumb;
   cfdata->thumb_size = ss_cfg->thumb_size;
   cfdata->mode = ss_cfg->mode;
   if (ss_cfg->location) cfdata->location = strdup(ss_cfg->location);
   if (ss_cfg->filename) cfdata->filename = strdup(ss_cfg->filename);
   if (ss_cfg->app) cfdata->app = strdup(ss_cfg->app);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL;
   E_Radio_Group *rg;

   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_frametable_add(evas, D_("General Settings"), 0);
   ow = e_widget_check_add(evas, D_("Beep when taking screenshot"), 
			   &(cfdata->use_bell));
   e_widget_frametable_object_append(of, ow, 0, 0, 2, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("Delay Time:"));
   e_widget_frametable_object_append(of, ow, 0, 1, 1, 1, 1, 0, 0, 0);
   ow = e_widget_slider_add(evas, 1, 0, "%1.0f seconds", 0.0, 60.0, 1.0, 0, 
			    &(cfdata->delay), NULL, 100);
   e_widget_frametable_object_append(of, ow, 1, 1, 1, 1, 1, 0, 1, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, D_("Capture Mode"), 0);
   rg = e_widget_radio_group_new(&(cfdata->mode));
   ow = e_widget_radio_add(evas, D_("Whole Screen"), 0, rg);
   e_widget_frametable_object_append(of, ow, 0, 0, 1, 1, 1, 0, 0, 0);
   ow = e_widget_radio_add(evas, D_("Select Window"), 1, rg);
   e_widget_frametable_object_append(of, ow, 0, 2, 1, 1, 1, 0, 0, 0);
   ow = e_widget_radio_add(evas, D_("Select Region"), 2, rg);
   e_widget_frametable_object_append(of, ow, 0, 3, 1, 1, 1, 0, 0, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, D_("File Settings"), 0);
   ow = e_widget_check_add(evas, D_("Always prompt for filename"), 
			   &(cfdata->prompt));
   e_widget_frametable_object_append(of, ow, 0, 0, 5, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("Save Directory:"));
   e_widget_frametable_object_append(of, ow, 0, 1, 1, 1, 1, 0, 0, 0);
   ow = e_widget_entry_add(evas, &(cfdata->location), NULL, NULL, NULL);
   e_widget_frametable_object_append(of, ow, 1, 1, 4, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("Filename:"));
   e_widget_frametable_object_append(of, ow, 0, 2, 1, 1, 1, 0, 0, 0);
   ow = e_widget_entry_add(evas, &(cfdata->filename), NULL, NULL, NULL);
   e_widget_frametable_object_append(of, ow, 1, 2, 4, 1, 1, 0, 1, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, D_("Application Settings"), 0);
   ow = e_widget_check_add(evas, D_("Launch application after taking screenshot"), 
			   &(cfdata->use_app));
   e_widget_frametable_object_append(of, ow, 0, 0, 4, 1, 1, 0, 1, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, D_("Thumbnail Settings"), 0);
   ow = e_widget_check_add(evas, D_("Generate thumbnail from screenshot"), 
			   &(cfdata->use_thumb));
   e_widget_frametable_object_append(of, ow, 0, 0, 2, 1, 1, 0, 1, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int 
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   ss_cfg->use_bell = cfdata->use_bell;
   ss_cfg->delay = cfdata->delay;
   ss_cfg->prompt = cfdata->prompt;
   ss_cfg->mode = cfdata->mode;
   if (ss_cfg->location) eina_stringshare_del(ss_cfg->location);
   if (cfdata->location) 
     ss_cfg->location = eina_stringshare_add(cfdata->location);
   else
     ss_cfg->location = eina_stringshare_add(e_user_homedir_get());
   if (ss_cfg->filename) eina_stringshare_del(ss_cfg->filename);
   if (cfdata->filename)
     ss_cfg->filename = eina_stringshare_add(cfdata->filename);
   ss_cfg->use_app = cfdata->use_app;
   ss_cfg->use_thumb = cfdata->use_thumb;

   e_config_save_queue();
   return 1;
}

static Evas_Object *
_adv_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL;
   Evas_Object *launch_check = NULL, *app_entry = NULL;
   Evas_Object *thumb_slider = NULL, *thumb_check = NULL;
   E_Radio_Group *rg;

   o = e_widget_table_add(evas, 0);
   of = e_widget_frametable_add(evas, D_("General Settings"), 0);
   ow = e_widget_check_add(evas, D_("Beep when taking screenshot"), 
			   &(cfdata->use_bell));
   e_widget_frametable_object_append(of, ow, 0, 0, 2, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("Delay Time:"));
   e_widget_frametable_object_append(of, ow, 0, 1, 1, 1, 1, 0, 0, 0);
   ow = e_widget_slider_add(evas, 1, 0, "%1.0f seconds", 0.0, 60.0, 1.0, 0, 
			    &(cfdata->delay), NULL, 100);
   e_widget_frametable_object_append(of, ow, 1, 1, 1, 1, 1, 0, 1, 0);

   ow = e_widget_label_add(evas, D_("Image Quality:"));
   e_widget_frametable_object_append(of, ow, 0, 2, 1, 1, 1, 0, 0, 0);
   ow = e_widget_slider_add(evas, 1, 0, "%1.0f %%", 1.0, 100.0, 1.0, 0, 
			    NULL, &(cfdata->quality), 100);
   e_widget_frametable_object_append(of, ow, 1, 2, 1, 1, 1, 0, 1, 0);
   e_widget_table_object_append(o, of, 0, 0, 1, 1, 1, 0, 1, 0);

   of = e_widget_frametable_add(evas, D_("Capture Mode"), 0);
   rg = e_widget_radio_group_new(&(cfdata->mode));
   ow = e_widget_radio_add(evas, D_("Whole Screen"), 0, rg);
   e_widget_frametable_object_append(of, ow, 0, 0, 1, 1, 1, 0, 0, 0);
   ow = e_widget_radio_add(evas, D_("Select Window"), 1, rg);
   e_widget_frametable_object_append(of, ow, 0, 2, 1, 1, 1, 0, 0, 0);
   ow = e_widget_radio_add(evas, D_("Select Region"), 2, rg);
   e_widget_frametable_object_append(of, ow, 0, 3, 1, 1, 1, 0, 0, 0);
   e_widget_table_object_append(o, of, 0, 1, 1, 1, 1, 1, 1, 0);

   of = e_widget_frametable_add(evas, D_("File Settings"), 0);
   ow = e_widget_check_add(evas, D_("Always prompt for filename"), 
			   &(cfdata->prompt));
   e_widget_frametable_object_append(of, ow, 0, 0, 5, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("Save Directory:"));
   e_widget_frametable_object_append(of, ow, 0, 1, 1, 1, 1, 0, 0, 0);
   ow = e_widget_entry_add(evas, &(cfdata->location), NULL, NULL, NULL);
   e_widget_frametable_object_append(of, ow, 1, 1, 4, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("Filename:"));
   e_widget_frametable_object_append(of, ow, 0, 2, 1, 1, 1, 0, 0, 0);
   ow = e_widget_entry_add(evas, &(cfdata->filename), NULL, NULL, NULL);
   e_widget_frametable_object_append(of, ow, 1, 2, 4, 1, 1, 0, 1, 0);
   e_widget_table_object_append(o, of, 0, 2, 2, 1, 1, 0, 1, 0);

   of = e_widget_frametable_add(evas, D_("Application Settings"), 0);
   ow = e_widget_check_add(evas, D_("Launch application after taking screenshot"), 
			   &(cfdata->use_app));
   launch_check = ow;
   e_widget_frametable_object_append(of, ow, 0, 0, 4, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("Application:"));
   e_widget_frametable_object_append(of, ow, 0, 1, 1, 1, 1, 0, 0, 0);
   app_entry = e_widget_entry_add(evas, &(cfdata->app), NULL, NULL, NULL);
   e_widget_frametable_object_append(of, app_entry, 1, 1, 3, 1, 1, 0, 1, 0);
   e_widget_table_object_append(o, of, 1, 0, 1, 1, 1, 1, 1, 0);

   // set state from saved config
   e_widget_disabled_set(app_entry, !cfdata->use_app);

   // handler for enable/disable widget
   e_widget_on_change_hook_set(launch_check, _cb_disable_check, app_entry);

   of = e_widget_frametable_add(evas, D_("Thumbnail Settings"), 0);
   thumb_check = e_widget_check_add(evas, D_("Generate thumbnail from screenshot"), 
			   &(cfdata->use_thumb));
   e_widget_frametable_object_append(of, thumb_check, 0, 0, 4, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("Size:"));
   e_widget_frametable_object_append(of, ow, 0, 1, 1, 1, 1, 0, 0, 0);
   thumb_slider = e_widget_slider_add(evas, 1, 0, "%1.0f %%", 10.0, 100.0, 5.0, 0, 
			    NULL, &(cfdata->thumb_size), 100);
   e_widget_frametable_object_append(of, thumb_slider, 1, 1, 3, 1, 1, 0, 1, 0);
   ow = e_widget_label_add(evas, D_("(Percentage of original image to use for thumbnail)"));
   e_widget_frametable_object_append(of, ow, 0, 2, 4, 1, 1, 0, 1, 0);
   e_widget_table_object_append(o, of, 1, 1, 1, 1, 1, 0, 1, 0);

   // set state from saved config
   e_widget_disabled_set(thumb_slider, !cfdata->use_thumb);

   // handler for enable/disable widget
   e_widget_on_change_hook_set(thumb_check, _cb_disable_check, thumb_slider);

   return o;
}

static int 
_adv_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   ss_cfg->delay = cfdata->delay;
   ss_cfg->prompt = cfdata->prompt;
   ss_cfg->use_app = cfdata->use_app;
   ss_cfg->use_bell = cfdata->use_bell;
   ss_cfg->quality = cfdata->quality;
   ss_cfg->use_thumb = cfdata->use_thumb;
   ss_cfg->thumb_size = cfdata->thumb_size;
   ss_cfg->mode = cfdata->mode;
   if (ss_cfg->location) eina_stringshare_del(ss_cfg->location);
   if (cfdata->location) 
     ss_cfg->location = eina_stringshare_add(cfdata->location);
   else
     ss_cfg->location = eina_stringshare_add(e_user_homedir_get());
   if (ss_cfg->filename) eina_stringshare_del(ss_cfg->filename);
   if (cfdata->filename)
     ss_cfg->filename = eina_stringshare_add(cfdata->filename);
   if (ss_cfg->app) eina_stringshare_del(ss_cfg->app);
   if (cfdata->app)
     ss_cfg->app = eina_stringshare_add(cfdata->app);
   e_config_save_queue();
   return 1;
}

/*!
 * @param data A Evas_Object to chain together with the checkbox
 * @param obj A Evas_Object checkbox created with e_widget_check_add()
 */
static void
_cb_disable_check(void *data, Evas_Object *obj)
{
   e_widget_disabled_set((Evas_Object *) data, 
                         !e_widget_check_checked_get(obj));
}
