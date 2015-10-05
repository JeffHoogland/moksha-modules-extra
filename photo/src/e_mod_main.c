#include "Photo.h"
/* module requirements */
EAPI E_Module_Api e_modapi = 
   {
      E_MODULE_API_VERSION,
      "Photo"
   };

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static const E_Gadcon_Client_Class _gadcon_class =
   {
      GADCON_CLIENT_CLASS_VERSION,
      "photo",
      {
         _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
         e_gadcon_site_is_not_toolbar
      },
      E_GADCON_CLIENT_STYLE_PLAIN
   };

/* photo functions */
static int  _e_canvas_init(void);
static void _e_canvas_shutdown(void);
static int  _theme_init(void);
static void _theme_shutdown(void);

/*
 * Module functions
 */

EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[4096];

   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   photo = E_NEW(Photo, 1);
   photo->module = m;

   DMAIN(("Initialisation ..."));

   if (!_e_canvas_init())
     {
        e_module_dialog_show(m, D_("Photo Module"), D_("E Canvas init failed !"));
        e_modapi_shutdown(m);
        return NULL;
     }

   if (!_theme_init())
     {
        e_module_dialog_show(m, D_("Photo Module"), D_("Theme init failed !"));
        e_modapi_shutdown(m);
        return NULL;
     }

   if (!photo_config_init())
     {
        e_module_dialog_show(m, D_("Photo Module"), D_("Config init failed"));
        e_modapi_shutdown(m);
        return NULL;
     }

   if (!photo_picture_init())
     {
        e_module_dialog_show(m, D_("Photo Module"), D_("Picture subsystem init failed"));
        e_modapi_shutdown(m);
        return NULL;
     }

   if (!photo_popup_warn_init())
     {
        e_module_dialog_show(m, D_("Photo Module"), D_("Popup warn subsystem init failed"));
        e_modapi_shutdown(m);
        return NULL;
     }

   if (!photo_popup_info_init())
     {
        e_module_dialog_show(m, D_("Photo Module"), D_("Popup info subsystem init failed"));
        e_modapi_shutdown(m);
        return NULL;
     }

   e_gadcon_provider_register((const E_Gadcon_Client_Class *)&_gadcon_class);

   DMAIN(("Initialisation END"));

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   if (!photo) return 0;

   e_gadcon_provider_unregister((const E_Gadcon_Client_Class *)&_gadcon_class);

   photo_popup_info_shutdown();
   photo_popup_warn_shutdown();
   photo_picture_shutdown();
   photo_config_shutdown();

   if (photo->config_dialog)
     photo_config_dialog_hide();
   if (photo->config_dialog_adddir)
     photo_config_dialog_dir_hide(NULL);

   _theme_shutdown();
   _e_canvas_shutdown();

   E_FREE(photo);

   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   if (!photo) return 0;

   photo_config_save();

   return 1;
}


/*
 * Gadcon functions
 */

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Photo_Item *pi;

   DMAIN(("GCC init"));

   o = edje_object_add(gc->evas);
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   pi = photo_item_add(gcc, o, id);
   gcc->data = pi;

   photo->items = eina_list_append(photo->items, pi);

   return pi->gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Photo_Item *pi;
   
   pi = gcc->data;

   DMAIN(("GCC shutdown"));
   photo->items = eina_list_remove(photo->items, pi);
   photo_item_del(pi);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}
   
static const char *
_gc_label(const E_Gadcon_Client_Class *client_class)
{
   return D_("Photo");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas)
{
   Evas_Object *o;
   char buf[4096];
   
   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-photo.edj",
	    e_module_dir_get(photo->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   return _gadcon_class.name;
}


/*
 * Photo functions
 */

static int _e_canvas_init(void)
{
  E_Container *c;

  c = e_container_current_get(e_manager_current_get());
  evas_output_viewport_get(c->bg_evas,
			   NULL, NULL,
			   &photo->canvas_w, &photo->canvas_h);
  
  return 1;
}

static void _e_canvas_shutdown(void)
{
  
}

static int _theme_init(void)
{
   char buf[4096];
   const char *path;
   char *version;

   path = e_theme_edje_file_get(PHOTO_THEME_IN_E, PHOTO_THEME_ITEM);
   if (path && path[0])
     {
        version = edje_file_data_get(path, "version");
        DD(("THEME E path %s version %s", path, version));
        if ( !version || strcmp(version, PHOTO_THEME_VERSION) )
          {
             free(version);
             return 0;
          }
        photo->theme = NULL;
     }
   else
     {
        snprintf(buf, sizeof(buf), "%s/photo.edj", e_module_dir_get(photo->module));
        version = edje_file_data_get(buf, "version");
        DD(("THEME own version %s", version));
        if ( !version || strcmp(version, PHOTO_THEME_VERSION) )
          {
             free(version);
             return 0;
          }
        photo->theme = strdup(buf);
     }
   free(version);

   return 1;
}

static void _theme_shutdown(void)
{
  E_FREE(photo->theme);
}
