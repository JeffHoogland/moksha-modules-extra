#include <e.h>
#include "e_mod_main.h"

//#define FORECASTS    5
#define KM_TO_MI     1.609344
#define MB_TO_IN     68.946497518

#define GOLDEN_RATIO 1.618033989

#define ENABLE_DEBUG 0
#define DEBUG(f, ...) if (ENABLE_DEBUG) \
    printf("[forecasts] "f "\n", __VA_ARGS__)

/* Gadcon Function Protos */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name,
                                 const char *id, const char *style);
static void             _gc_shutdown(E_Gadcon_Client *gcc);
static void             _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char      *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object     *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char      *_gc_id_new(const E_Gadcon_Client_Class *client_class);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *forecasts_config = NULL;

/* Define Gadcon Class */
static const E_Gadcon_Client_Class _gadcon_class = {
   GADCON_CLIENT_CLASS_VERSION,
   "forecasts", {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, NULL},
   E_GADCON_CLIENT_STYLE_PLAIN
};

/* Module specifics */
typedef struct _Instance  Instance;
typedef struct _Forecasts Forecasts;

struct _Instance
{
   E_Gadcon_Client     *gcc;
   Evas_Object         *forecasts_obj;
   Forecasts           *forecasts;
   Ecore_Timer         *check_timer;
   Ecore_Con_Server    *server;
   Ecore_Event_Handler *add_handler;
   Ecore_Event_Handler *del_handler;
   Ecore_Event_Handler *data_handler;

   struct
   {
      int  temp, code;
      char update[52];
      char desc[256];
   } condition;

   struct
   {
      char temp, distance[3], pressure[3], speed[4];
   } units;

   struct
   {
      struct
      {
         int chill, direction, speed;
      } wind;

      struct
      {
         int   humidity, rising;
         float pressure, visibility;
      } atmosphere;

      struct
      {
         char sunrise[9], sunset[9];
      } astronomy;
   } details;

   struct
   {
      char day[4];
      char date[12];
      int  low, high, code;
      char desc[256];
   } forecast[5];

   Eina_Strbuf    *buffer;
   const char     *location;
   const char     *area;

   E_Gadcon_Popup *popup;
   Config_Item    *ci;
};

struct _Forecasts
{
   Instance    *inst;
   Evas_Object *forecasts_obj;
   Evas_Object *icon_obj;
};

struct
{
   const char *host;
   int         port;
} proxy = {
   NULL, 0
};

/* Module Function Protos */
static void         _forecasts_cb_mouse_down(void *data, Evas *e, Evas_Object *obj,
                                             void *event_info);
static void         _forecasts_menu_cb_configure(void *data, E_Menu *m,
                                                 E_Menu_Item *mi);
static void         _forecasts_menu_cb_post(void *data, E_Menu *m);
static Eina_Bool    _forecasts_cb_check(void *data);
static Config_Item *_forecasts_config_item_get(const char *id);
static Forecasts   *_forecasts_new(Evas *evas);
static void         _forecasts_free(Forecasts *w);
static void         _forecasts_get_proxy(void);
static Eina_Bool    _forecasts_server_add(void *data, int type, void *event);
static Eina_Bool    _forecasts_server_del(void *data, int type, void *event);
static Eina_Bool    _forecasts_server_data(void *data, int type, void *event);
static int          _forecasts_parse(void *data);
static void         _forecasts_converter(Instance *inst);
static void         _forecasts_convert_degrees(int *value, int dir);
static void         _forecasts_convert_distances(int *value, int dir);
static void         _forecasts_convert_distances_float(float *value, int dir);
static void         _forecasts_convert_pressures(float *value, int dir);
static void         _forecasts_display_set(Instance *inst, int ok);
static void         _forecasts_popup_content_create(Instance *inst);
static void         _cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Evas_Object *_forecasts_popup_icon_create(Evas *evas, int code);
static void         _forecasts_popup_destroy(Instance *inst);

/* Gadcon Functions */
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Forecasts *w;
   Instance *inst;

   inst = E_NEW(Instance, 1);

   inst->ci = _forecasts_config_item_get(id);
   inst->area = eina_stringshare_add(inst->ci->code);
   inst->buffer = eina_strbuf_new();

   w = _forecasts_new(gc->evas);
   w->inst = inst;
   inst->forecasts = w;

   o = w->forecasts_obj;
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->popup = NULL;
   inst->forecasts_obj = o;
   evas_object_event_callback_add(inst->forecasts_obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _cb_mouse_down, inst);
   evas_object_event_callback_add(inst->forecasts_obj, EVAS_CALLBACK_MOUSE_IN,
                                  _cb_mouse_in, inst);
   evas_object_event_callback_add(inst->forecasts_obj, EVAS_CALLBACK_MOUSE_OUT,
                                  _cb_mouse_out, inst);

   if (!inst->add_handler)
     inst->add_handler =
       ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
                               _forecasts_server_add, inst);
   if (!inst->del_handler)
     inst->del_handler =
       ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
                               _forecasts_server_del, inst);
   if (!inst->data_handler)
     inst->data_handler =
       ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
                               _forecasts_server_data, inst);

   evas_object_event_callback_add(w->forecasts_obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _forecasts_cb_mouse_down, inst);
   forecasts_config->instances =
     eina_list_append(forecasts_config->instances, inst);

   _forecasts_cb_check(inst);
   inst->check_timer =
     ecore_timer_add(inst->ci->poll_time, _forecasts_cb_check, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;
   Forecasts *w;

   inst = gcc->data;
   w = inst->forecasts;

   if (inst->popup) _forecasts_popup_destroy(inst);
   if (inst->check_timer)
     ecore_timer_del(inst->check_timer);
   if (inst->add_handler)
     ecore_event_handler_del(inst->add_handler);
   if (inst->data_handler)
     ecore_event_handler_del(inst->data_handler);
   if (inst->del_handler)
     ecore_event_handler_del(inst->del_handler);
   if (inst->server)
     ecore_con_server_del(inst->server);
   if (inst->area)
     eina_stringshare_del(inst->area);
   eina_strbuf_free(inst->buffer);

   inst->server = NULL;
   forecasts_config->instances =
     eina_list_remove(forecasts_config->instances, inst);

   evas_object_event_callback_del(w->forecasts_obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _forecasts_cb_mouse_down);

   _forecasts_free(w);
   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   Instance *inst;

   inst = gcc->data;

   switch (orient)
     {
      case E_GADCON_ORIENT_FLOAT:
        edje_object_signal_emit(inst->forecasts_obj, "e,state,orientation,float", "e");
        e_gadcon_client_aspect_set(gcc, 240, 120);
        e_gadcon_client_min_size_set(gcc, 240, 120);
        break;

      default:
        edje_object_signal_emit(inst->forecasts_obj, "e,state,orientation,default", "e");
        e_gadcon_client_aspect_set(gcc, 16, 16);
        e_gadcon_client_min_size_set(gcc, 16, 16);
        break;
     }
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class)
{
   return D_("Forecasts");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas)
{
   Evas_Object *o;
   char buf[4096];

   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-forecasts.edj",
            e_module_dir_get(forecasts_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   Config_Item *ci;

   ci = _forecasts_config_item_get(NULL);
   return ci->id;
}

static void
_forecasts_cb_mouse_down(void *data, Evas *e, Evas_Object *obj,
                         void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;
   if ((ev->button == 3) && (!forecasts_config->menu))
     {
        E_Menu *m;
        E_Menu_Item *mi;
        int x, y, w, h;

        m = e_menu_new();
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Settings"));
        e_util_menu_item_theme_icon_set(mi, "preferences-system");
        e_menu_item_callback_set(mi, _forecasts_menu_cb_configure, inst);

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
        e_menu_post_deactivate_callback_set(m, _forecasts_menu_cb_post, inst);
        forecasts_config->menu = m;

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, &w, &h);
        e_menu_activate_mouse(m,
                              e_util_zone_current_get(e_manager_current_get
                                                        ()), x + ev->output.x,
                              y + ev->output.y, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void
_forecasts_menu_cb_post(void *data, E_Menu *m)
{
   if (!forecasts_config->menu)
     return;
   e_object_del(E_OBJECT(forecasts_config->menu));
   forecasts_config->menu = NULL;
}

static void
_forecasts_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Instance *inst;

   inst = data;
   _config_forecasts_module(inst->ci);
}

static Config_Item *
_forecasts_config_item_get(const char *id)
{
   Eina_List *l;
   Config_Item *ci;
   char buf[128];

   if (!id)
     {
        int num = 0;

        /* Create id */
        if (forecasts_config->items)
          {
             const char *p;
             ci = eina_list_last(forecasts_config->items)->data;
             p = strrchr(ci->id, '.');
             if (p) num = strtol(p + 1, NULL, 10) + 1;
          }
        snprintf(buf, sizeof(buf), "%s.%d", _gadcon_class.name, num);
        id = buf;
     }
   else
     {
        for (l = forecasts_config->items; l; l = l->next)
          {
             ci = l->data;
             if (!ci->id)
               continue;
             if (!strcmp(ci->id, id))
               return ci;
          }
     }

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->poll_time = 60.0;
   ci->days = 15.0;
   ci->degrees = DEGREES_C;
   ci->host = eina_stringshare_add("xml.weather.yahoo.com");
   ci->code = eina_stringshare_add("BUXX0005");
   ci->show_text = 1;
   ci->popup_on_hover = 1;

   forecasts_config->items = eina_list_append(forecasts_config->items, ci);
   return ci;
}

/* Gadman Module Setup */
EAPI E_Module_Api e_modapi = {
   E_MODULE_API_VERSION,
   "Forecasts"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[4095];

   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   conf_item_edd = E_CONFIG_DD_NEW("Forecasts_Config_Item", Config_Item);
#undef T
#undef D
#define T Config_Item
#define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, poll_time, DOUBLE);
   E_CONFIG_VAL(D, T, days, DOUBLE);
   E_CONFIG_VAL(D, T, degrees, INT);
   E_CONFIG_VAL(D, T, host, STR);
   E_CONFIG_VAL(D, T, code, STR);
   E_CONFIG_VAL(D, T, show_text, INT);
   E_CONFIG_VAL(D, T, popup_on_hover, INT);

   conf_edd = E_CONFIG_DD_NEW("Forecasts_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);

   forecasts_config = e_config_domain_load("module.forecasts", conf_edd);
   if (!forecasts_config)
     {
        Config_Item *ci;

        forecasts_config = E_NEW(Config, 1);

        ci = E_NEW(Config_Item, 1);
        ci->poll_time = 60.0;
        ci->days = 15.0;
        ci->degrees = DEGREES_C;
        ci->host = eina_stringshare_add("xml.weather.yahoo.com");
        ci->code = eina_stringshare_add("BUXX0005");
        ci->id = eina_stringshare_add("0");
        ci->show_text = 1;
        ci->popup_on_hover = 1;

        forecasts_config->items = eina_list_append(forecasts_config->items, ci);
     }
   _forecasts_get_proxy();

   forecasts_config->module = m;
   e_gadcon_provider_register(&_gadcon_class);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   forecasts_config->module = NULL;
   e_gadcon_provider_unregister(&_gadcon_class);

   if (forecasts_config->config_dialog)
     e_object_del(E_OBJECT(forecasts_config->config_dialog));
   if (forecasts_config->menu)
     {
        e_menu_post_deactivate_callback_set(forecasts_config->menu, NULL, NULL);
        e_object_del(E_OBJECT(forecasts_config->menu));
        forecasts_config->menu = NULL;
     }

   while (forecasts_config->items)
     {
        Config_Item *ci;

        ci = forecasts_config->items->data;
        if (ci->id)
          eina_stringshare_del(ci->id);
        if (ci->host)
          eina_stringshare_del(ci->host);
        if (ci->code)
          eina_stringshare_del(ci->code);
        forecasts_config->items =
          eina_list_remove_list(forecasts_config->items, forecasts_config->items);
        free(ci);
        ci = NULL;
     }

   E_FREE(forecasts_config);
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   e_config_domain_save("module.forecasts", conf_edd, forecasts_config);
   return 1;
}

static Forecasts *
_forecasts_new(Evas *evas)
{
   Forecasts *w;
   char buf[4096];

   w = E_NEW(Forecasts, 1);

   w->forecasts_obj = edje_object_add(evas);

   snprintf(buf, sizeof(buf), "%s/forecasts.edj",
            e_module_dir_get(forecasts_config->module));
   if (!e_theme_edje_object_set(w->forecasts_obj, "base/theme/modules/forecasts",
                                "modules/forecasts/main"))
     edje_object_file_set(w->forecasts_obj, buf, "modules/forecasts/main");
   evas_object_show(w->forecasts_obj);

   w->icon_obj = edje_object_add(evas);
   if (!e_theme_edje_object_set(w->icon_obj, "base/theme/modules/forecasts/icons",
                                "modules/forecasts/icons/3200"))
     edje_object_file_set(w->icon_obj, buf, "modules/forecasts/icons/3200");
   edje_object_part_swallow(w->forecasts_obj, "icon", w->icon_obj);

   return w;
}

static void
_forecasts_free(Forecasts *w)
{
   evas_object_del(w->forecasts_obj);
   evas_object_del(w->icon_obj);
   free(w);
   w = NULL;
}

static void
_forecasts_get_proxy(void)
{
   const char *env;
   const char *host = NULL;
   const char *p;
   int port = 0;

   env = getenv("http_proxy");
   if ((!env) || (!*env)) env = getenv("HTTP_PROXY");
   if ((!env) || (!*env)) return;
   if (strncmp(env, "http://", 7)) return;

   host = strchr(env, ':');
   host += 3;
   p = strchr(host, ':');
   if (p)
     {
        if (sscanf(p + 1, "%d", &port) != 1)
          port = 0;
     }
   if ((host) && (port))
     {
        if (proxy.host) eina_stringshare_del(proxy.host);
        proxy.host = eina_stringshare_add_length(host, p - host);
        proxy.port = port;
     }
}

static Eina_Bool
_forecasts_cb_check(void *data)
{
   Instance *inst;

   /* check that data is valid */
   if (!(inst = data)) return EINA_FALSE;

   /* if we have a previous server, delete it */
   if (inst->server) ecore_con_server_del(inst->server);

   /* server deleted, set variable to NULL */
   inst->server = NULL;

   if (proxy.port != 0)
     inst->server =
       ecore_con_server_connect(ECORE_CON_REMOTE_NODELAY,
                                proxy.host, proxy.port, inst);
   else
     inst->server =
       ecore_con_server_connect(ECORE_CON_REMOTE_NODELAY, inst->ci->host, 80, inst);

   if (!inst->server) return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_forecasts_server_add(void *data, int type, void *event)
{
   Instance *inst;
   Ecore_Con_Event_Server_Add *ev;
   char buf[1024];
   char forecast[1024];
   char degrees;

   inst = data;
   if (!inst)
     return EINA_TRUE;

   ev = event;
   if ((!inst->server) || (inst->server != ev->server))
     return EINA_TRUE;

   if (inst->ci->degrees == DEGREES_F)
     degrees = 'f';
   else
     degrees = 'c';

   snprintf(forecast, sizeof(forecast), "/forecastrss?p=%s&u=%c", inst->ci->code, degrees);
   snprintf(buf, sizeof(buf), "GET http://%s%s HTTP/1.1\r\n"
                              "Host: %s\r\n"
                              "Connection: close\r\n\r\n",
            inst->ci->host, forecast, inst->ci->host);
   DEBUG("Server: %s", buf);
   ecore_con_server_send(inst->server, buf, strlen(buf));
   return EINA_FALSE;
}

static Eina_Bool
_forecasts_server_del(void *data, int type, void *event)
{
   Instance *inst;
   Ecore_Con_Event_Server_Del *ev;
   int ret;

   inst = data;
   ev = event;
   if ((!inst->server) || (inst->server != ev->server))
     return EINA_TRUE;

   ecore_con_server_del(inst->server);
   inst->server = NULL;

   ret = _forecasts_parse(inst);
   _forecasts_converter(inst);
   _forecasts_display_set(inst, ret);

   eina_strbuf_string_free(inst->buffer);

   return EINA_FALSE;
}

static Eina_Bool
_forecasts_server_data(void *data, int type, void *event)
{
   Instance *inst;
   Ecore_Con_Event_Server_Data *ev;
   void *tmp;

   inst = data;
   ev = event;

   if ((!inst->server) || (inst->server != ev->server))
     return EINA_TRUE;
   eina_strbuf_append_length(inst->buffer, ev->data, ev->size);
   return EINA_FALSE;
}

static int
_forecasts_parse(void *data)
{
   Instance *inst;
   char *needle;
   char city[256];
   char region[256];
   char location[512];
   float visibility;
   int i;

   inst = data;
   if (!inst)
     return 0;
   if (!inst->buffer)
     return 0;

   /* Location */
   needle = strstr(eina_strbuf_string_get(inst->buffer), "<yweather:location city=");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%255[^\"]\"", city);

   region[0] = '\0';
   needle = strstr(needle, "region=\"");
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%255[^\"]\"", region);

   if (strlen(region))
     snprintf(location, 512, "%s, %s", city, region);
   else
     snprintf(location, 512, "%s", city);

   eina_stringshare_replace(&inst->location, location);

   /* Units */
   needle = strstr(eina_strbuf_string_get(inst->buffer), "<yweather:units temperature=");
   if (!needle)
     goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%c\"", &inst->units.temp);
   needle = strstr(needle, "distance=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%2[^\"]\"", inst->units.distance);
   needle = strstr(needle, "pressure=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%2[^\"]\"", inst->units.pressure);
   needle = strstr(needle, "speed=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%3[^\"]\"", inst->units.speed);

   /* Current conditions */
   needle = strstr(eina_strbuf_string_get(inst->buffer), "<yweather:condition  text=");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%255[^\"]\"", inst->condition.desc);
   needle = strstr(needle, "code=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%d\"", &inst->condition.code);
   needle = strstr(needle, "temp=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%d\"", &inst->condition.temp);
   needle = strstr(needle, "date=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%51[^\"]\"", inst->condition.update);

   /* Details */
   /* Wind */
   needle = strstr(eina_strbuf_string_get(inst->buffer), "<yweather:wind chill=");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%d\"", &inst->details.wind.chill);
   needle = strstr(needle, "direction=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%d\"", &inst->details.wind.direction);
   needle = strstr(needle, "speed=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%d\"", &inst->details.wind.speed);

   /* Atmosphere */
   needle = strstr(eina_strbuf_string_get(inst->buffer), "<yweather:atmosphere humidity=");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%d\"", &inst->details.atmosphere.humidity);
   needle = strstr(needle, "visibility=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%f\"", &visibility);
   inst->details.atmosphere.visibility = visibility;
   needle = strstr(needle, "pressure=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%f\"", &inst->details.atmosphere.pressure);
   needle = strstr(needle, "rising=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%d\"", &inst->details.atmosphere.rising);

   /* Astronomy */
   needle = strstr(eina_strbuf_string_get(inst->buffer), "<yweather:astronomy sunrise=");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%8[^\"]\"", inst->details.astronomy.sunrise);
   needle = strstr(needle, "sunset=\"");
   if (!needle) goto error;
   needle = strstr(needle, "\"");
   sscanf(needle, "\"%8[^\"]\"", inst->details.astronomy.sunset);

   /* Forecasts */
   for (i = 0; i < inst->ci->days / 5; i++)
     {
        needle = strstr(needle, "<yweather:forecast day=");
        if (!needle) goto error;
        needle = strstr(needle, "\"");
        sscanf(needle, "\"%4[^\"]\"", inst->forecast[i].day);
        needle = strstr(needle, "date=\"");
        if (!needle) goto error;
        needle = strstr(needle, "\"");
        sscanf(needle, "\"%12[^\"]\"", inst->forecast[i].date);
        needle = strstr(needle, "low=\"");
        if (!needle) goto error;
        needle = strstr(needle, "\"");
        sscanf(needle, "\"%d\"", &inst->forecast[i].low);
        needle = strstr(needle, "high=\"");
        if (!needle) goto error;
        needle = strstr(needle, "\"");
        sscanf(needle, "\"%d\"", &inst->forecast[i].high);
        needle = strstr(needle, "text=\"");
        if (!needle) goto error;
        needle = strstr(needle, "\"");
        sscanf(needle, "\"%255[^\"]\"", inst->forecast[i].desc);
        needle = strstr(needle, "code=\"");
        if (!needle) goto error;
        needle = strstr(needle, "\"");
        sscanf(needle, "\"%d\"", &inst->forecast[i].code);
     }
   return 1;

error:
   fprintf(stderr, "ERROR: Couldn't parse info from xml.weather.yahoo.com\n");
   return 0;
}

void
_forecasts_converter(Instance *inst)
{
   int i, dir = -1;

   if ((inst->units.temp == 'F') && (inst->ci->degrees == DEGREES_C))
     {
        dir = DEGREES_C;
        inst->units.temp = 'C';
        snprintf(inst->units.distance, 3, "km");
        snprintf(inst->units.pressure, 3, "mb");
        snprintf(inst->units.speed, 4, "kph");
     }
   else if ((inst->units.temp == 'C') && (inst->ci->degrees == DEGREES_F))
     {
        dir = DEGREES_F;
        inst->units.temp = 'F';
        snprintf(inst->units.distance, 3, "mi");
        snprintf(inst->units.pressure, 3, "in");
        snprintf(inst->units.speed, 4, "mph");
     }
   if (dir == -1) return;

   _forecasts_convert_degrees(&inst->condition.temp, dir);

   _forecasts_convert_degrees(&inst->details.wind.chill, dir);
   _forecasts_convert_distances(&inst->details.wind.speed, dir);
   _forecasts_convert_distances_float(&inst->details.atmosphere.visibility, dir);
   _forecasts_convert_pressures(&inst->details.atmosphere.pressure, dir);
   for (i = 0; i < inst->ci->days / 5; i++)
     {
        _forecasts_convert_degrees(&inst->forecast[i].low, dir);
        _forecasts_convert_degrees(&inst->forecast[i].high, dir);
     }
}

static void
_forecasts_convert_degrees(int *value, int dir)
{
   if ((dir == DEGREES_C))
     *value = (*value - 32) * 5.0 / 9.0;
   else
     *value = (*value * 9.0 / 5.0) + 32;
}

static void
_forecasts_convert_distances(int *value, int dir)
{
   if ((dir == DEGREES_C))
     *value = (*value) * KM_TO_MI;
   else
     *value = (*value) / KM_TO_MI;
}

static void
_forecasts_convert_distances_float(float *value, int dir)
{
   if ((dir == DEGREES_C))
     *value = (*value) * KM_TO_MI;
   else
     *value = (*value) / KM_TO_MI;
}

static void
_forecasts_convert_pressures(float *value, int dir)
{
   if ((dir == DEGREES_C))
     *value = (*value) * MB_TO_IN;
   else
     *value = (*value) / MB_TO_IN;
}

static void
_forecasts_display_set(Instance *inst, int ok)
{
   char buf[4096];
   char m[4096];

   if (!inst)
     return;

   snprintf(m, sizeof(m), "%s/forecasts.edj",
            e_module_dir_get(forecasts_config->module));
   snprintf(buf, sizeof(buf), "modules/forecasts/icons/%d", inst->condition.code);
   if (!e_theme_edje_object_set(inst->forecasts->icon_obj,
                                "base/theme/modules/forecasts/icons", buf))
     edje_object_file_set(inst->forecasts->icon_obj, m, buf);
   edje_object_part_swallow(inst->forecasts->forecasts_obj, "icon", inst->forecasts->icon_obj);

   if (!inst->ci->show_text)
     edje_object_signal_emit(inst->forecasts_obj, "e,state,description,hide", "e");
   else
     edje_object_signal_emit(inst->forecasts_obj, "e,state,description,show", "e");

   snprintf(buf, sizeof(buf), "%d°%c", inst->condition.temp, inst->units.temp);
   edje_object_part_text_set(inst->forecasts->forecasts_obj, "e.text.temp", buf);
   edje_object_part_text_set(inst->forecasts->forecasts_obj, "e.text.description", inst->condition.desc);
   edje_object_part_text_set(inst->forecasts->forecasts_obj, "e.text.location", inst->location);

   if (inst->gcc->gadcon->orient == E_GADCON_ORIENT_FLOAT)
     {
        char buf[4096], name[60];
        int i;

        for (i = 0; i < inst->ci->days / 5; i++)
          {
             Evas_Object *swallow;

             snprintf(name, sizeof(name), "e.text.day%d.date", i);
             edje_object_part_text_set(inst->forecasts->forecasts_obj, name, inst->forecast[i].date);

             snprintf(name, sizeof(name), "e.text.day%d.description", i);
             edje_object_part_text_set(inst->forecasts->forecasts_obj, name, inst->forecast[i].desc);

             snprintf(name, sizeof(name), "e.text.day%d.high", i);
             snprintf(buf, sizeof(buf), "%d°%c", inst->forecast[i].high, inst->units.temp);
             edje_object_part_text_set(inst->forecasts->forecasts_obj, name, buf);

             snprintf(name, sizeof(name), "e.text.day%d.low", i);
             snprintf(buf, sizeof(buf), "%d°%c", inst->forecast[i].low, inst->units.temp);
             edje_object_part_text_set(inst->forecasts->forecasts_obj, name, buf);

             snprintf(name, sizeof(name), "e.swallow.day%d.icon", i);
             swallow = edje_object_part_swallow_get(inst->forecasts->forecasts_obj, name);
             if (swallow)
               evas_object_del(swallow);
             edje_object_part_swallow(inst->forecasts->forecasts_obj, name,
                                      _forecasts_popup_icon_create(inst->gcc->gadcon->evas, inst->forecast[i].code));
          }
     }

   if (inst->popup) _forecasts_popup_destroy(inst);
   inst->popup = NULL;
}

void
_forecasts_config_updated(Config_Item *ci)
{
   Eina_List *l;
   char buf[4096];

   if (!forecasts_config)
     return;
   for (l = forecasts_config->instances; l; l = l->next)
     {
        Instance *inst;

        inst = l->data;
        if (inst->ci != ci) continue;
        int area_changed = 0;

        if (inst->area && strcmp(inst->area, inst->ci->code))
          area_changed = 1;

        if (inst->area) eina_stringshare_del(inst->area);
        inst->area = eina_stringshare_add(inst->ci->code);
        _forecasts_converter(inst);

        if (inst->popup) _forecasts_popup_destroy(inst);
        inst->popup = NULL;

        snprintf(buf, sizeof(buf), "%d°%c", inst->condition.temp, inst->units.temp);
        edje_object_part_text_set(inst->forecasts->forecasts_obj, "e.text.temp", buf);

        if (!inst->ci->show_text)
          edje_object_signal_emit(inst->forecasts_obj, "e,state,description,hide", "e");
        else
          edje_object_signal_emit(inst->forecasts_obj, "e,state,description,show", "e");

        if (area_changed)
          _forecasts_cb_check(inst);
          
        if (!inst->check_timer)
          inst->check_timer = ecore_timer_add(inst->ci->poll_time, _forecasts_cb_check, inst);
        else
          ecore_timer_interval_set(inst->check_timer, inst->ci->poll_time);
          
          
     }
}

static void
_forecasts_popup_content_create(Instance *inst)
{
   Evas_Object *o, *ol, *of, *ob, *oi;
   Evas *evas;
   char buf[4096];
   int row = 0, i;
   Evas_Coord w, h, mw, mh;

   if (!inst->location) return;

   inst->popup = e_gadcon_popup_new(inst->gcc);

   evas = inst->popup->win->evas;
   o = e_widget_list_add(evas, 0, 0);
   snprintf(buf, sizeof(buf), D_("%s: Current Conditions"), inst->location);
   of = e_widget_frametable_add(evas, buf, 0);

   snprintf(buf, sizeof(buf), "%s: %d°%c", inst->condition.desc, inst->condition.temp, inst->units.temp);
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 0, row, 2, 1, 0, 1, 1, 0);

   oi = _forecasts_popup_icon_create(inst->popup->win->evas, inst->condition.code);
   edje_object_size_max_get(oi, &w, &h);
   DEBUG("Icon size %dx%d", w, h);
   if (w > 160) w = 160;  /* For now there is a limit to how big the icon should be */
   if (h > 160) h = 160;  /* In the future, the icon should be set from the theme, not part of the table */
   ob = e_widget_image_add_from_object(evas, oi, w, h);
   e_widget_frametable_object_append(of, ob, 2, row, 1, 2, 1, 0, 1, 1);

   ob = e_widget_label_add(evas, D_("Wind Chill"));
   e_widget_frametable_object_append(of, ob, 0, ++row, 1, 1, 1, 0, 0, 0);
   snprintf(buf, sizeof(buf), "%d°%c", inst->details.wind.chill, inst->units.temp);
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 0, 0);

   ob = e_widget_label_add(evas, D_("Wind Speed"));
   e_widget_frametable_object_append(of, ob, 0, ++row, 1, 1, 1, 0, 0, 0);
   snprintf(buf, sizeof(buf), "%d %s", inst->details.wind.speed, inst->units.speed);
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 0, 0);

   ob = e_widget_label_add(evas, D_("Humidity"));
   e_widget_frametable_object_append(of, ob, 0, ++row, 1, 1, 1, 0, 0, 0);
   snprintf(buf, sizeof(buf), "%d %%", inst->details.atmosphere.humidity);
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 0, 0);

   ob = e_widget_label_add(evas, D_("Visibility"));
   e_widget_frametable_object_append(of, ob, 0, ++row, 1, 1, 1, 0, 0, 0);
   snprintf(buf, sizeof(buf), "%.2f %s", inst->details.atmosphere.visibility, inst->units.distance);
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 0, 0);

   ob = e_widget_label_add(evas, D_("Pressure"));
   e_widget_frametable_object_append(of, ob, 0, ++row, 1, 1, 1, 0, 0, 0);
   snprintf(buf, sizeof(buf), "%.2f %s", inst->details.atmosphere.pressure, inst->units.pressure);
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 0, 0);

   if (inst->details.atmosphere.rising == 1)
     snprintf(buf, sizeof(buf), D_("Rising"));
   else if (inst->details.atmosphere.rising == 2)
     snprintf(buf, sizeof(buf), D_("Falling"));
   else
     snprintf(buf, sizeof(buf), D_("Steady"));
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 2, row, 1, 1, 1, 0, 1, 0);

   ob = e_widget_label_add(evas, D_("Sunrise / Sunset"));
   e_widget_frametable_object_append(of, ob, 0, ++row, 1, 1, 1, 0, 0, 0);
   snprintf(buf, sizeof(buf), "%s", inst->details.astronomy.sunrise);
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);

   snprintf(buf, sizeof(buf), "%s", inst->details.astronomy.sunset);
   ob = e_widget_label_add(evas, buf);
   e_widget_frametable_object_append(of, ob, 2, row, 1, 1, 1, 0, 1, 0);

   e_widget_list_object_append(o, of, 1, 1, 0.5);
   ol = e_widget_list_add(evas, 1, 1);

   for (i = 0; i < inst->ci->days / 5; i++)
     {
        int row = 0;

        if (!i)
          snprintf(buf, sizeof(buf), D_("Today"));
        else if (i == 1)
          snprintf(buf, sizeof(buf), D_("Tomorrow"));
        else
          snprintf(buf, sizeof(buf), "%s", inst->forecast[i].date);
        of = e_widget_frametable_add(evas, buf, 0);

        ob = e_widget_label_add(evas, inst->forecast[i].desc);
        e_widget_frametable_object_append(of, ob, 0, row, 3, 1, 0, 1, 1, 1);

        ob = e_widget_label_add(evas, D_("High"));
        e_widget_frametable_object_append(of, ob, 0, ++row, 1, 1, 1, 0, 1, 0);
        snprintf(buf, sizeof(buf), "%d°%c", inst->forecast[i].high, inst->units.temp);
        ob = e_widget_label_add(evas, buf);
        e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);

        ob = e_widget_image_add_from_object(evas,_forecasts_popup_icon_create(inst->popup->win->evas,inst->forecast[i].code), 0, 0);
        e_widget_frametable_object_append(of, ob, 2, row, 1, 2, 1, 1, 0, 0);

        ob = e_widget_label_add(evas, D_("Low"));
        e_widget_frametable_object_append(of, ob, 0, ++row, 1, 1, 1, 0, 1, 0);
        snprintf(buf, sizeof(buf), "%d°%c", inst->forecast[i].low, inst->units.temp);
        ob = e_widget_label_add(evas, buf);
        e_widget_frametable_object_append(of, ob, 1, row, 1, 1, 1, 0, 1, 0);
        e_widget_list_object_append(ol, of, 1, 1, 0.5);
     }

   e_widget_list_object_append(o, ol, 1, 1, 0.5);
   e_widget_size_min_get(o, &mw, &mh);
   if ((double)mw / mh > GOLDEN_RATIO)
     mh = mw / GOLDEN_RATIO;
   else if ((double)mw / mh < GOLDEN_RATIO - (double)1)
     mw = mh * (GOLDEN_RATIO - (double)1);
   e_widget_size_min_set(o, mw, mh);

   e_gadcon_popup_content_set(inst->popup, o);
}

static Evas_Object *
_forecasts_popup_icon_create(Evas *evas, int code)
{
   char buf[4096];
   char m[4096];
   Evas_Object *o;

   snprintf(m, sizeof(m), "%s/forecasts.edj",
            e_module_dir_get(forecasts_config->module));
   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "modules/forecasts/icons/%d", code);
   if (!e_theme_edje_object_set(o, "base/theme/modules/forecasts/icons", buf))
     edje_object_file_set(o, m, buf);

   return o;
}

static void
_forecasts_popup_destroy(Instance *inst)
{
   if (!inst->popup) return;
   e_object_del(E_OBJECT(inst->popup));
}

static void
_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   if (!(inst = data)) return;

   if (!inst->ci->popup_on_hover)
     {
        if (!inst->popup) _forecasts_popup_content_create(inst);
        e_gadcon_popup_show(inst->popup);
        return;
     }

   ev = event_info;
   if (ev->button == 1)
     {
        e_gadcon_popup_toggle_pinned(inst->popup);
     }
}

static void
_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;

   if (!(inst = data)) return;
   if (!inst->ci->popup_on_hover) return;

   if (!inst->popup) _forecasts_popup_content_create(inst);
   e_gadcon_popup_show(inst->popup);
}

static void
_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;

   if (!(inst = data)) return;
   if (!(inst->popup)) return;

   if (inst->popup->pinned) return;
   e_gadcon_popup_hide(inst->popup);
}

