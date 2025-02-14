#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   int display_mode;
   char *display_name;
   char *icon_path;
   char *status_cmd;
   char *dblclk_cmd;
   char *okstate_string;
   char *okstate_exitcode;
   char *okstate_lines;
   int okstate_mode;
   int refresh_after_dblclk_cmd;
   double poll_time_hours;
   double poll_time_mins;
   double poll_time_secs;
};

static void *_create_data(E_Config_Dialog * cfd);
static void _free_data(E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog * cfd, Evas * evas, E_Config_Dialog_Data * cfdata);
static int _basic_apply_data(E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata);
static void _fill_data(Config_Item * ci, E_Config_Dialog_Data * cfdata);

void
_config_execwatch_module(Config_Item * ci)
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
	    e_module_dir_get(execwatch_config->module));
   con = e_container_current_get(e_manager_current_get());
   cfd = e_config_dialog_new(con, "Execwatch Settings", "Execwatch",
			    "_e_modules_execwatch_config_dialog", buf, 0, v, ci);
   execwatch_config->config_dialog = cfd;
}

static void
_fill_data(Config_Item * ci, E_Config_Dialog_Data * cfdata)
{
   char buf[32]; 

   if (ci->display_name)
     cfdata->display_name = strdup(ci->display_name);
   else
     cfdata->display_name = strdup("");
   if (ci->icon_path)
     cfdata->icon_path = strdup(ci->icon_path);
   else
     cfdata->icon_path = strdup("");
   if (ci->status_cmd)
     cfdata->status_cmd = strdup(ci->status_cmd);
   else
     cfdata->status_cmd = strdup("");
   if (ci->dblclk_cmd)
     cfdata->dblclk_cmd = strdup(ci->dblclk_cmd);
   else
     cfdata->dblclk_cmd = strdup("");
   if (ci->okstate_string)
     cfdata->okstate_string = strdup(ci->okstate_string);
   else
     cfdata->okstate_string = strdup("");

   snprintf (buf, sizeof(buf), "%d", ci->okstate_exitcode);
   cfdata->okstate_exitcode = strdup(buf);
   snprintf (buf, sizeof(buf), "%d", ci->okstate_lines);
   cfdata->okstate_lines = strdup(buf);
   cfdata->display_mode = ci->display_mode;
   cfdata->okstate_mode = ci->okstate_mode;
   cfdata->refresh_after_dblclk_cmd = ci->refresh_after_dblclk_cmd;
   cfdata->poll_time_hours = ci->poll_time_hours / 3600;
   cfdata->poll_time_mins = ci->poll_time_mins / 60;
   cfdata->poll_time_secs = ci->poll_time_secs;
}

static void *
_create_data(E_Config_Dialog * cfd)
{
   E_Config_Dialog_Data *cfdata;
   Config_Item *ci;

   ci = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);

   _fill_data(ci, cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
   if (!execwatch_config)
     return;

   execwatch_config->config_dialog = NULL;
   free(cfdata);
   cfdata = NULL;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog * cfd, Evas * evas, E_Config_Dialog_Data * cfdata)
{
   Evas_Object *o, *ob, *otb;
   E_Radio_Group *rg;

   otb = e_widget_toolbook_add(evas, (48 * e_scale), (48 * e_scale));


   o = e_widget_table_add(evas, 0);

   ob = e_widget_label_add(evas, D_("Command"));
   e_widget_table_object_append(o, ob, 0, 0, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &cfdata->status_cmd, NULL, NULL, NULL);
   e_widget_size_min_set(ob, 150, 20);
   e_widget_table_object_append(o, ob, 1, 0, 1, 1, 1, 0, 1, 0);
   ob = e_widget_label_add(evas, D_("(Example: ping -c 5 127.0.0.1)"));
   e_widget_table_object_append(o, ob, 1, 1, 2, 1, 1, 0, 1, 0);

   ob = e_widget_label_add(evas, D_("Poll Time"));
   e_widget_table_object_append(o, ob, 0, 2, 2, 1, 1, 0, 1, 0);
   ob = e_widget_slider_add(evas, 1, 0, D_("%2.0f hours"), 0.0, 24.0, 1.0, 0,
			  &(cfdata->poll_time_hours), NULL, 40);
   e_widget_table_object_append(o, ob, 1, 2, 2, 1, 1, 0, 1, 0);
   ob = e_widget_slider_add(evas, 1, 0, D_("%2.0f mins"), 0.0, 59.0, 1.0, 0,
			  &(cfdata->poll_time_mins), NULL, 40);
   e_widget_table_object_append(o, ob, 1, 3, 2, 1, 1, 0, 1, 0);
   ob = e_widget_slider_add(evas, 1, 0, D_("%2.0f secs"), 0.0, 59.0, 1.0, 0,
			  &(cfdata->poll_time_secs), NULL, 40);
   e_widget_table_object_append(o, ob, 1, 4, 2, 1, 1, 0, 1, 0);

   ob = e_widget_label_add(evas, D_("Doubleclick Command"));
   e_widget_table_object_append(o, ob, 0, 5, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &cfdata->dblclk_cmd, NULL, NULL, NULL);
   e_widget_size_min_set(ob, 150, 20);
   e_widget_table_object_append(o, ob, 1, 5, 1, 1, 1, 0, 1, 0);
   ob = e_widget_check_add(evas, D_("Refresh after Execution"), &(cfdata->refresh_after_dblclk_cmd));
   e_widget_table_object_append(o, ob, 1, 6, 1, 1, 1, 0, 1, 0);

   e_widget_toolbook_page_append(otb, NULL, D_("Command"), o, 1, 0, 1, 0, 0.5, 0.0);


   o = e_widget_table_add(evas, 0);

   ob = e_widget_label_add(evas, D_("Title"));
   e_widget_table_object_append(o, ob, 0, 0, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &cfdata->display_name, NULL, NULL, NULL);
   e_widget_size_min_set(ob, 150, 20);
   e_widget_table_object_append(o, ob, 1, 0, 1, 1, 1, 0, 1, 0);

   ob = e_widget_label_add(evas, D_("Mode"));
   e_widget_table_object_append(o, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   rg = e_widget_radio_group_new (&(cfdata->display_mode));
   ob = e_widget_radio_add (evas, D_("Evaluate Command Result"), 0, rg);
   e_widget_table_object_append(o, ob, 1, 1, 1, 1, 1, 0, 1, 0);
   ob = e_widget_radio_add (evas, D_("Show Command Output"), 1, rg);
   e_widget_table_object_append(o, ob, 1, 2, 1, 1, 1, 0, 1, 0);

   ob = e_widget_label_add(evas, D_("Custom Icon"));
   e_widget_table_object_append(o, ob, 0, 3, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &cfdata->icon_path, NULL, NULL, NULL);
   e_widget_size_min_set(ob, 150, 20);
   e_widget_table_object_append(o, ob, 1, 3, 1, 1, 1, 0, 1, 0);

   e_widget_toolbook_page_append(otb, NULL, D_("Display"), o, 1, 0, 1, 0, 0.5, 0.0);


   o = e_widget_table_add(evas, 0);

   rg = e_widget_radio_group_new (&(cfdata->okstate_mode));
   ob = e_widget_radio_add (evas, D_("Expected Exitcode"), 0, rg);
   e_widget_table_object_append(o, ob, 0, 0, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &cfdata->okstate_exitcode, NULL, NULL, NULL);
   e_widget_size_min_set(ob, 150, 20);
   e_widget_table_object_append(o, ob, 1, 0, 1, 1, 1, 0, 1, 0);

   ob = e_widget_radio_add (evas, D_("Expected String"), 1, rg);
   e_widget_table_object_append(o, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &cfdata->okstate_string, NULL, NULL, NULL);
   e_widget_size_min_set(ob, 150, 20);
   e_widget_table_object_append(o, ob, 1, 1, 1, 1, 1, 0, 1, 0);

   ob = e_widget_radio_add (evas, D_("Expected Lines"), 2, rg);
   e_widget_table_object_append(o, ob, 0, 2, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &cfdata->okstate_lines, NULL, NULL, NULL);
   e_widget_size_min_set(ob, 150, 20);
   e_widget_table_object_append(o, ob, 1, 2, 1, 1, 1, 0, 1, 0);

   e_widget_toolbook_page_append(otb, NULL, D_("Evaluation"), o, 1, 0, 1, 0, 0.5, 0.0);


   e_widget_toolbook_page_show(otb, 0);
   return otb;
}

static int
_basic_apply_data(E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
   Config_Item *ci;

   ci = cfd->data;
   if (!strlen(cfdata->status_cmd)) return 0;
   switch (cfdata->okstate_mode)
     {
	case 0: if (!strlen(cfdata->okstate_exitcode)) return 0; break;
	case 1: if (!strlen(cfdata->okstate_string)) return 0; break;
	case 2: if (!strlen(cfdata->okstate_lines)) return 0; break;
     }
   if (!cfdata->poll_time_hours &&
	   !cfdata->poll_time_mins &&
	   !cfdata->poll_time_secs) return 0;

   if (ci->display_name) eina_stringshare_del(ci->display_name);
   ci->display_name = eina_stringshare_add(cfdata->display_name);
   if (ci->icon_path) eina_stringshare_del(ci->icon_path);
   ci->icon_path = eina_stringshare_add(cfdata->icon_path);
   if (ci->status_cmd) eina_stringshare_del(ci->status_cmd);
   ci->status_cmd = eina_stringshare_add(cfdata->status_cmd);
   if (strlen(cfdata->okstate_string))
     {
	if (ci->okstate_string) eina_stringshare_del(ci->okstate_string);
	ci->okstate_string = eina_stringshare_add(cfdata->okstate_string);
     }
   if (ci->dblclk_cmd) eina_stringshare_del(ci->dblclk_cmd);
   if (strlen(cfdata->dblclk_cmd))
     ci->dblclk_cmd = eina_stringshare_add(cfdata->dblclk_cmd);
   if (strlen(cfdata->okstate_exitcode))
     ci->okstate_exitcode = atoi (cfdata->okstate_exitcode);
   if (strlen(cfdata->okstate_lines))
     ci->okstate_lines = atoi (cfdata->okstate_lines);
   ci->display_mode = cfdata->display_mode;
   ci->okstate_mode = cfdata->okstate_mode;
   ci->refresh_after_dblclk_cmd = cfdata->refresh_after_dblclk_cmd;
   ci->poll_time_hours = cfdata->poll_time_hours * 3600;
   ci->poll_time_mins = cfdata->poll_time_mins * 60;
   ci->poll_time_secs = cfdata->poll_time_secs;

   e_config_save_queue();
   _execwatch_config_updated(ci);
   return 1;
}
