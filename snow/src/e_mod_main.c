#include <e.h>
#include "config.h"
#include "e_mod_main.h"
#include "e_mod_config.h"

/* module private routines */
static Snow *_snow_init(E_Module *m);
static void _snow_shutdown(Snow *snow);
static Eina_Bool _snow_cb_animator(void *data);
static void _snow_trees_load(Snow *snow);
static void _snow_flakes_load(char type, Snow *snow);

EAPI E_Module *snow_module = NULL;

/* public module routines. all modules must have these */
EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION, "Snow"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   Snow *snow;
   char buf[PATH_MAX];

   /* Set up module's message catalogue */
   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   snprintf(buf, sizeof(buf), "%s/e-module-snow.edj", e_module_dir_get(m));
   e_configure_registry_category_add("appearance", 10, D_("Look"), NULL, 
				     "preferences-look");
   e_configure_registry_item_add("appearance/snow", 150, D_("Snow"), NULL, 
				 buf, e_int_config_snow_module);

   snow = _snow_init(m);
   snow_module = m;
   return snow;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   Snow *snow;

   e_configure_registry_item_del("appearance/snow");
   e_configure_registry_category_del("appearance");

   snow = m->data;
   if (snow)
     {
        if (snow->config_dialog)
          {
             e_object_del(E_OBJECT(snow->config_dialog));
             snow->config_dialog = NULL;
          }
        _snow_shutdown(snow);
     }
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   Snow *snow;

   if (!(snow = m->data)) return 1;
   e_config_domain_save("module.snow", snow->conf_edd, snow->conf);
   return 1;
}

/* module private routines */
static Snow *
_snow_init(E_Module *m)
{
   Snow *snow;
   Eina_List *managers, *l, *l2;

   snow = calloc(1, sizeof(Snow));
   if (!snow) return NULL;

   snow->module = m;
   snow->conf_edd = E_CONFIG_DD_NEW("Snow_Config", Config);
#undef T
#undef D
#define T Config
#define D snow->conf_edd
   E_CONFIG_VAL(D, T, tree_count, INT);
   E_CONFIG_VAL(D, T, flake_count, INT);
   E_CONFIG_VAL(D, T, show_trees, INT);

   snow->conf = e_config_domain_load("module.snow", snow->conf_edd);
   if (!snow->conf)
     {
        snow->conf = E_NEW(Config, 1);

        snow->conf->tree_count = 10;
        snow->conf->flake_count = 60;
        snow->conf->show_trees = 1;
     }

   E_CONFIG_LIMIT(snow->conf->show_trees, 0, 1);

   managers = e_manager_list();
   for (l = managers; l; l = l->next)
     {
        E_Manager *man;

        man = l->data;
        for (l2 = man->containers; l2; l2 = l2->next)
          {
             E_Container *con;

             con = l2->data;
             snow->cons = eina_list_append(snow->cons, con);
             snow->canvas = con->bg_evas;
          }
     }

   evas_output_viewport_get(snow->canvas, NULL, NULL, &snow->width, &snow->height);

   if (snow->conf->show_trees)
      _snow_trees_load(snow);
   _snow_flakes_load('s', snow);
   _snow_flakes_load('m', snow);
   _snow_flakes_load('l', snow);

   snow->animator = ecore_animator_add(_snow_cb_animator, snow);

   return snow;
}

static void
_snow_trees_free(Snow *snow)
{
   while (snow->trees)
     {
        Evas_Object *tree;

        tree = snow->trees->data;
        evas_object_del(tree);
        snow->trees = eina_list_remove_list(snow->trees, snow->trees);
     }
}

static void
_snow_flakes_free(Snow *snow)
{
   while (snow->flakes)
     {
        Snow_Flake *flake;

        flake = snow->flakes->data;
        evas_object_del(flake->flake);
        snow->flakes = eina_list_remove_list(snow->flakes, snow->flakes);
        free(flake);
	flake = NULL;
     }
}

static void
_snow_shutdown(Snow *snow)
{
   free(snow->conf);
   snow->conf = NULL;
   E_CONFIG_DD_FREE(snow->conf_edd);
   while (snow->cons)
     {
        E_Container *con;

        con = snow->cons->data;
        snow->cons = eina_list_remove_list(snow->cons, snow->cons);
     }
   _snow_trees_free(snow);
   _snow_flakes_free(snow);
   if (snow->animator)
      ecore_animator_del(snow->animator);
   free(snow);
   snow = NULL;
}

static void
_snow_canvas_reset(Snow *snow)
{
   _snow_trees_free(snow);
   _snow_flakes_free(snow);

   if (snow->conf->show_trees)
      _snow_trees_load(snow);
   _snow_flakes_load('s', snow);
   _snow_flakes_load('m', snow);
   _snow_flakes_load('l', snow);
}

static void
_snow_trees_load(Snow *snow)
{
   Evas_Object *o;
   int tw, th, i;
   char buf[PATH_MAX];

   o = evas_object_image_add(snow->canvas);
   snprintf(buf, sizeof(buf), "%s/tree.png", e_module_dir_get(snow->module));
   evas_object_image_file_set(o, buf, "");
   evas_object_image_size_get(o, &tw, &th);

   for (i = 0; i < snow->conf->tree_count; i++)
     {
        Evas_Coord tx, ty;

        if (i != 0)
          {
             o = evas_object_image_add(snow->canvas);
	     snprintf(buf, sizeof(buf), "%s/tree.png", e_module_dir_get(snow->module));
	     evas_object_image_file_set(o, buf, "");
          }
        evas_object_resize(o, tw, th);
        evas_object_image_alpha_set(o, 1);
        evas_object_image_fill_set(o, 0, 0, tw, th);

        tx = random() % (snow->width - tw);
        ty = random() % (snow->height - th);
        evas_object_move(o, tx, ty);
        evas_object_pass_events_set(o, 1);
        evas_object_show(o);
        snow->trees = eina_list_append(snow->trees, o);
     }
}

static void
_snow_flakes_load(char type, Snow *snow)
{
   Evas_Object *o;
   Evas_Coord xx, yy, ww, hh;
   char buf[PATH_MAX];
   int tw, th, i;
   Snow_Flake *flake;

   evas_output_viewport_get(snow->canvas, &xx, &yy, &ww, &hh);
   snprintf(buf, sizeof(buf), "%s/flake-%c.png", e_module_dir_get(snow->module), type);

   o = evas_object_image_add(snow->canvas);
   evas_object_image_file_set(o, buf, "");
   evas_object_image_size_get(o, &tw, &th);

   for (i = 0; i < snow->conf->flake_count / 3; i++)
     {
        Evas_Coord tx, ty;

        flake = malloc(sizeof(Snow_Flake));
        if (i != 0)
          {
             o = evas_object_image_add(snow->canvas);
             evas_object_image_file_set(o, buf, "");
          }
        evas_object_resize(o, tw, th);
        evas_object_image_alpha_set(o, 1);
        evas_object_image_fill_set(o, 0, 0, tw, th);

        tx = random() % ww;
        ty = random() % hh;
        evas_object_move(o, tx, ty);
        evas_object_show(o);
        flake->flake = o;
        flake->start_time = ecore_time_get() + (double)(random() % (th * 10)) / (double)th;
        switch (type)
          {
          case 's':
             flake->speed = 1;
             break;
          case 'm':
             flake->speed = 2;
             break;
          case 'l':
             flake->speed = 3;
             break;
          }
        snow->flakes = eina_list_append(snow->flakes, flake);
     }

}

static Eina_Bool
_snow_cb_animator(void *data)
{
   Snow *snow;
   Eina_List *next;
   double d;

   snow = data;
   next = snow->flakes;
   while (next)
     {
        Snow_Flake *flake;
        Evas_Coord x, y;

        flake = next->data;
        d = ecore_time_get() - flake->start_time;
        y = 30 * d * flake->speed;
        evas_object_geometry_get(flake->flake, &x, NULL, NULL, NULL);
        if (y > snow->height)
           flake->start_time = ecore_time_get() + (double)(random() % 100) / (double)100;
        evas_object_move(flake->flake, x, y);

        next = eina_list_next(next);
     }
   return EINA_TRUE;
}

void
_snow_cb_config_updated(void *data)
{
   Snow *s;

   if (!(s = data)) return;
   _snow_canvas_reset(s);
}
