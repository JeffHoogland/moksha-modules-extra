#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
  double poll_time;
  int always_text;
  int show_percent;
#ifdef __linux__
  int real_ignore_buffers;
  int real_ignore_cached;
#endif
};

/* Protos */
static void *_create_data (E_Config_Dialog * cfd);
static void _free_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata);
static Evas_Object *_basic_create_widgets (E_Config_Dialog * cfd, Evas * evas,
					   E_Config_Dialog_Data * cfdata);
static int _basic_apply_data (E_Config_Dialog * cfd,
			      E_Config_Dialog_Data * cfdata);
static void _fill_data (Config_Item * ci, E_Config_Dialog_Data * cfdata);

/* Config Calls */
void
_config_mem_module (Config_Item * ci)
{
  E_Config_Dialog *cfd;
  E_Config_Dialog_View *v;
  E_Container *con;
  char buf[4096];

  v = E_NEW (E_Config_Dialog_View, 1);

  v->create_cfdata = _create_data;
  v->free_cfdata = _free_data;
  v->basic.apply_cfdata = _basic_apply_data;
  v->basic.create_widgets = _basic_create_widgets;

  snprintf (buf, sizeof (buf), "%s/e-module-mem.edj",
	    e_module_dir_get (mem_config->module));
  con = e_container_current_get (e_manager_current_get ());
  cfd = e_config_dialog_new (con, D_ ("Mem Settings"), "Mem", 
			     "_e_modules_mem_config_dialog", buf, 0, v, ci);
  mem_config->config_dialog = cfd;
}

static void
_fill_data (Config_Item * ci, E_Config_Dialog_Data * cfdata)
{
  cfdata->poll_time = ci->poll_time;
#ifdef __linux__
  cfdata->real_ignore_buffers = ci->real_ignore_buffers;
  cfdata->real_ignore_cached = ci->real_ignore_cached;
#endif
//   cfdata->show_text = c->conf->show_text;
  cfdata->always_text = ci->always_text;
  cfdata->show_percent = ci->show_percent;
}

static void *
_create_data (E_Config_Dialog * cfd)
{
  E_Config_Dialog_Data *cfdata;
  Config_Item *ci;

  ci = cfd->data;
  cfdata = E_NEW (E_Config_Dialog_Data, 1);

  _fill_data (ci, cfdata);
  return cfdata;
}

static void
_free_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
  if (!mem_config)
    return;
  mem_config->config_dialog = NULL;
  free (cfdata);
  cfdata = NULL;
}

static Evas_Object *
_basic_create_widgets (E_Config_Dialog * cfd, Evas * evas,
		       E_Config_Dialog_Data * cfdata)
{
  Evas_Object *o, *of, *ob;
  E_Radio_Group *rg;

  o = e_widget_list_add (evas, 0, 0);
  of = e_widget_framelist_add (evas, D_ ("General Settings"), 0);
  rg = e_widget_radio_group_new (&(cfdata->always_text));
  ob = e_widget_radio_add (evas, D_ ("Always Show Text"), 1, rg);
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_radio_add (evas, D_ ("Show Text On Mouse Over"), 0, rg);
  e_widget_framelist_object_append (of, ob);
  ob =
    e_widget_check_add (evas, D_ ("Show Text As Percent"),
			&(cfdata->show_percent));
  e_widget_framelist_object_append (of, ob);
#ifdef __linux__
  ob =
    e_widget_check_add (evas, D_ ("Ignore Buffers"),
			&(cfdata->real_ignore_buffers));
  e_widget_framelist_object_append (of, ob);
  ob =
    e_widget_check_add (evas, D_ ("Ignore Cached"),
			&(cfdata->real_ignore_cached));
  e_widget_framelist_object_append (of, ob);
#endif
  ob = e_widget_label_add (evas, D_ ("Check Interval:"));
  e_widget_framelist_object_append (of, ob);
  ob =
    e_widget_slider_add (evas, 1, 0, D_ ("%1.0f seconds"), 1.0, 60.0, 1.0, 0,
			 &(cfdata->poll_time), NULL, 150);
  e_widget_framelist_object_append (of, ob);
  e_widget_list_object_append (o, of, 1, 1, 0.5);

  return o;
}

static int
_basic_apply_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
  Config_Item *ci;

  ci = cfd->data;
  ci->poll_time = cfdata->poll_time;
#ifdef __linux__
  ci->real_ignore_buffers = cfdata->real_ignore_buffers;
  ci->real_ignore_cached = cfdata->real_ignore_cached;
#endif
  ci->always_text = cfdata->always_text;
  ci->show_percent = cfdata->show_percent;
  e_config_save_queue ();
  _mem_config_updated (ci);

  return 1;
}
