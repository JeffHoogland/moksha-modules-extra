#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   int disable_timer;
   int random_order;
   int all_desks;
   double poll_time;
   char *dir;
};

/* Protos */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

void
_config_slideshow_module(Config_Item *ci)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   E_Container *con;
   char buf[PATH_MAX];

   v = E_NEW (E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.create_widgets = _basic_create;

   snprintf (buf, sizeof (buf), "%s/e-module-slideshow.edj", 
	     e_module_dir_get(slide_config->module));
   con = e_container_current_get (e_manager_current_get ());
   cfd = e_config_dialog_new (con, D_ ("Slideshow Settings"), 
			      "SlideShow", 
			      "_e_modules_slideshow_config_dialog", buf, 
			      0, v, ci);
   slide_config->config_dialog = cfd;
}

static void
_fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata)
{
   char buf[PATH_MAX];

   cfdata->poll_time = ci->poll_time;
   cfdata->disable_timer = ci->disable_timer;
   cfdata->random_order = ci->random_order;
   cfdata->all_desks = ci->all_desks;
   if (ci->dir)
     cfdata->dir = strdup(ci->dir);
   else
     {
	snprintf(buf, sizeof (buf), "%s/.e/e/backgrounds", e_user_homedir_get());
	cfdata->dir = strdup(buf);
     }
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;
   Config_Item *ci;

   ci = cfd->data;
   cfdata = E_NEW (E_Config_Dialog_Data, 1);

   _fill_data (ci, cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   if (!slide_config) return;
   slide_config->config_dialog = NULL;
   free(cfdata);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ob, *of, *ot;
   E_Radio_Group *rg;

   o = e_widget_list_add (evas, 0, 0);
   of = e_widget_framelist_add (evas, D_ ("Cycle Time"), 0);
   ob =
     e_widget_check_add (evas, D_ ("Disable Timer"), &(cfdata->disable_timer));
   e_widget_framelist_object_append (of, ob);
   ob =
     e_widget_slider_add (evas, 1, 0, D_ ("%3.0f seconds"), 5.0, 2400.0, 
                          1.0, 0, &(cfdata->poll_time), NULL, 200);
   e_widget_framelist_object_append (of, ob);
   e_widget_list_object_append (o, of, 1, 1, 0.5);

   of = e_widget_framelist_add (evas, D_ ("Directory"), 0);
   ot = e_widget_table_add (evas, 1);
   ob = e_widget_label_add (evas, D_ ("Sub-directory to use for backgrounds"));
   e_widget_table_object_append (ot, ob, 0, 0, 1, 1, 0, 0, 1, 0);
   ob = e_widget_entry_add (evas, &cfdata->dir, NULL, NULL, NULL);
   e_widget_table_object_append (ot, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   e_widget_framelist_object_append (of, ot);
   e_widget_list_object_append (o, of, 1, 1, 0.5);

   of = e_widget_framelist_add (evas, D_ ("Extra"), 0);
   ob = e_widget_check_add (evas, D_ ("Randomize order"), &(cfdata->random_order));
   e_widget_framelist_object_append (of, ob);
   rg = e_widget_radio_group_new(&(cfdata->all_desks));
   ob = e_widget_radio_add(evas, D_("Set wallpaper on all desktops"), 1, rg);
   e_widget_framelist_object_append (of, ob);
   ob = e_widget_radio_add(evas, D_("Set wallpaper on current desktop"), 0, rg);
   e_widget_framelist_object_append (of, ob);
   ob = e_widget_radio_add(evas, D_("Set wallpaper on current screen"), 2, rg);
   e_widget_framelist_object_append (of, ob);
   if(ecore_x_xinerama_screen_count_get() > 1)
     {
       ob = e_widget_radio_add(evas, D_("Set wallpaper on current monitor (Xinerama support)"), 2, rg);
       e_widget_framelist_object_append (of, ob);
     }
   e_widget_list_object_append (o, of, 1, 1, 0.5);

   return o;
}

static int
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Config_Item *ci;
   char buf[PATH_MAX];

   ci = cfd->data;
   ci->poll_time = cfdata->poll_time;
   ci->disable_timer = cfdata->disable_timer;
   ci->random_order = cfdata->random_order;
   ci->all_desks = cfdata->all_desks;

   if (ci->dir) eina_stringshare_del (ci->dir);
   if (cfdata->dir)
     ci->dir = eina_stringshare_add (cfdata->dir);
   else
     {
	snprintf (buf, sizeof (buf), "%s/.e/e/backgrounds", e_user_homedir_get ());
	ci->dir = eina_stringshare_add (buf);
     }

   e_config_save_queue ();
   _slide_config_updated (ci);
   return 1;
}
