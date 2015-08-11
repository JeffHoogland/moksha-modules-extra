#include <e.h>
#include "config.h"
#include "e_mod_main.h"
#include "e_mod_config.h"

/* module private routines */
static Rain *_rain_init(E_Module *m);
static void _rain_shutdown(Rain *rain);
static Eina_Bool _rain_cb_animator(void *data);
static void _rain_clouds_load(Rain *rain);
static void _rain_drops_load(char type, Rain *rain);

EAPI E_Module *rain_module = NULL;

/* public module routines. all modules must have these */
EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION, "Rain"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   Rain *rain;
   char buf[PATH_MAX];

   /* Set up module's message catalogue */
   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   snprintf(buf, sizeof(buf), "%s/e-module-rain.edj", e_module_dir_get(m));
   e_configure_registry_category_add("appearance", 10, D_("Look"), NULL, 
				     "preferences-look");
   e_configure_registry_item_add("appearance/rain", 150, D_("Rain"), NULL, 
				 buf, e_int_config_rain_module);

   rain = _rain_init(m);
   rain_module = m;
   return rain;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   Rain *rain;

   e_configure_registry_item_del("appearance/rain");
   e_configure_registry_category_del("appearance");

   rain = m->data;
   if (rain)
     {
        if (rain->config_dialog)
          {
             e_object_del(E_OBJECT(rain->config_dialog));
             rain->config_dialog = NULL;
          }
        _rain_shutdown(rain);
     }
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   Rain *rain;

   rain = m->data;
   if (!rain) return 1;
   e_config_domain_save("module.rain", rain->conf_edd, rain->conf);
   return 1;
}

/* module private routines */
static Rain *
_rain_init(E_Module *m)
{
   Rain *rain;
   Eina_List *managers, *l, *l2;

   rain = calloc(1, sizeof(Rain));
   if (!rain) return NULL;

   rain->module = m;
   rain->conf_edd = E_CONFIG_DD_NEW("Rain_Config", Config);
#undef T
#undef D
#define T Config
#define D rain->conf_edd
   E_CONFIG_VAL(D, T, cloud_count, INT);
   E_CONFIG_VAL(D, T, drop_count, INT);
   E_CONFIG_VAL(D, T, show_clouds, INT);

   rain->conf = e_config_domain_load("module.rain", rain->conf_edd);
   if (!rain->conf)
     {
        rain->conf = E_NEW(Config, 1);

        rain->conf->cloud_count = 10;
        rain->conf->drop_count = 60;
        rain->conf->show_clouds = 1;
     }

   E_CONFIG_LIMIT(rain->conf->show_clouds, 0, 1);

   managers = e_manager_list();
   for (l = managers; l; l = l->next)
     {
        E_Manager *man;

        man = l->data;
        for (l2 = man->containers; l2; l2 = l2->next)
          {
             E_Container *con;

             con = l2->data;
             rain->cons = eina_list_append(rain->cons, con);
             rain->canvas = con->bg_evas;
          }
     }

   evas_output_viewport_get(rain->canvas, NULL, NULL, &rain->width, &rain->height);

   if (rain->conf->show_clouds)
      _rain_clouds_load(rain);
   _rain_drops_load('s', rain);
   _rain_drops_load('m', rain);
   _rain_drops_load('l', rain);

   rain->animator = ecore_animator_add(_rain_cb_animator, rain);

   return rain;
}

static void
_rain_clouds_free(Rain *rain)
{
   while (rain->clouds)
     {
        Evas_Object *cloud;

        cloud = rain->clouds->data;
        evas_object_del(cloud);
        rain->clouds = eina_list_remove_list(rain->clouds, rain->clouds);
     }
}

static void
_rain_drops_free(Rain *rain)
{
   while (rain->drops)
     {
        Rain_Drop *drop;

        drop = rain->drops->data;
        evas_object_del(drop->drop);
        rain->drops = eina_list_remove_list(rain->drops, rain->drops);
        free(drop);
	drop = NULL;
     }
}

static void
_rain_shutdown(Rain *rain)
{
   free(rain->conf);
   E_CONFIG_DD_FREE(rain->conf_edd);
   while (rain->cons)
     {
        E_Container *con;

        con = rain->cons->data;
        rain->cons = eina_list_remove_list(rain->cons, rain->cons);
     }

   _rain_clouds_free(rain);
   _rain_drops_free(rain);
   if (rain->animator)
      ecore_animator_del(rain->animator);
   free(rain);
   rain = NULL;
}

static void
_rain_canvas_reset(Rain *rain)
{
   _rain_clouds_free(rain);
   _rain_drops_free(rain);

   if (rain->conf->show_clouds)
      _rain_clouds_load(rain);
   _rain_drops_load('s', rain);
   _rain_drops_load('m', rain);
   _rain_drops_load('l', rain);
}

static void
_rain_clouds_load(Rain *rain)
{
   Evas_Object *o;
   char buf[PATH_MAX];
   int tw, th, i;

   o = evas_object_image_add(rain->canvas);
   snprintf(buf, sizeof(buf), "%s/cloud.png", e_module_dir_get(rain->module));
   evas_object_image_file_set(o, buf, "");
   evas_object_image_size_get(o, &tw, &th);

   for (i = 0; i < rain->conf->cloud_count; i++)
     {
        Evas_Coord tx, ty;

        if (i != 0)
          {
             o = evas_object_image_add(rain->canvas);
	     snprintf(buf, sizeof(buf), "%s/cloud.png", 
		      e_module_dir_get(rain->module));
	     evas_object_image_file_set(o, buf, "");
          }
        evas_object_resize(o, tw, th);
        evas_object_image_alpha_set(o, 1);
        evas_object_image_fill_set(o, 0, 0, tw, th);

        tx = random() % (rain->width - tw);
        ty = random() % ((rain->height / 3) - th);
        evas_object_move(o, tx, ty);
        evas_object_pass_events_set(o, 1);
        evas_object_show(o);
        rain->clouds = eina_list_append(rain->clouds, o);
     }
}

static void
_rain_drops_load(char type, Rain *rain)
{
   Evas_Object *o;
   Evas_Coord xx, yy, ww, hh;
   char buf[PATH_MAX];
   int tw, th, i;
   Rain_Drop *drop;

   evas_output_viewport_get(rain->canvas, &xx, &yy, &ww, &hh);
   snprintf(buf, sizeof(buf), "%s/drop-%c.png", e_module_dir_get(rain->module), type);

   o = evas_object_image_add(rain->canvas);
   evas_object_image_file_set(o, buf, "");
   evas_object_image_size_get(o, &tw, &th);

   for (i = 0; i < rain->conf->drop_count / 3; i++)
     {
        Evas_Coord tx, ty;

        drop = malloc(sizeof(Rain_Drop));
        if (i != 0)
          {
             o = evas_object_image_add(rain->canvas);
             evas_object_image_file_set(o, buf, "");
          }
        evas_object_resize(o, tw, th);
        evas_object_image_alpha_set(o, 1);
        evas_object_image_fill_set(o, 0, 0, tw, th);

        tx = random() % (ww - tw);
        ty = random() % (hh - th);

        evas_object_move(o, tx, ty);
        evas_object_pass_events_set(o, 1);
        evas_object_show(o);
        drop->drop = o;
        drop->start_time = ecore_time_get() + (double)(random() % (th * 10)) / (double)th;
        switch (type)
          {
          case 's':
             drop->speed = 1;
             break;
          case 'm':
             drop->speed = 2;
             break;
          case 'l':
             drop->speed = 3;
             break;
          }
        rain->drops = eina_list_append(rain->drops, drop);
     }
}

static Eina_Bool
_rain_cb_animator(void *data)
{
   Rain *rain;
   Eina_List *next;
   double d;

   rain = data;
   next = rain->drops;
   while (next)
     {
        Rain_Drop *drop;
        Evas_Coord x, y;

        drop = next->data;
        d = ecore_time_get() - drop->start_time;
        y = 300 * d * drop->speed;
        evas_object_geometry_get(drop->drop, &x, NULL, NULL, NULL);
        if (y > rain->height)
           drop->start_time = ecore_time_get() + (double)(random() % 600) / (double)600;
        evas_object_move(drop->drop, x, y);

        next = eina_list_next(next);
     }
   return EINA_TRUE;
}

void
_rain_cb_config_updated(void *data)
{
   Rain *r;

   if (!(r = data)) return;
   _rain_canvas_reset(r);
}
