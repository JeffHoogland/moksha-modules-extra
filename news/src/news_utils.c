#include "News.h"


/*
 * Public functions
 */

int
news_url_image_get(char *url, void (*func_cb) (News_Feed_Document *doc, int error))
{
   // TODO

   return 0;
}

int
news_util_ecanvas_geometry_get(int *canvas_w, int *canvas_h)
{
   E_Container *c;

   c = e_container_current_get(e_manager_current_get());
   evas_output_viewport_get(c->bg_evas,
                            NULL, NULL,
                            canvas_w, canvas_h);

   return 1;
}

int
news_util_datecmp(const struct tm *t1, const struct tm *t2)
{
   if (t1->tm_year != t2->tm_year)
     return (t1->tm_year - t2->tm_year);
   if (t1->tm_mon != t2->tm_mon)
     return (t1->tm_mon - t2->tm_mon);
   if (t1->tm_mday != t2->tm_mday)
     return (t1->tm_mday - t2->tm_mday);
   if (t1->tm_hour != t2->tm_hour)
     return (t1->tm_hour - t2->tm_hour);
   if (t1->tm_min != t2->tm_min)
     return (t1->tm_min - t2->tm_min);
   if (t1->tm_sec != t2->tm_sec)
     return (t1->tm_sec - t2->tm_sec);
   return 0;
}

int
news_util_browser_open(const char *url)
{
   Ecore_Exe *exe;
   char buf[4096];

   if (!url) return 0;

   if(!ecore_file_app_installed("xdg-open"))
   {
      news_util_message_error_show("<hilight>xdg-open not found !</hilight><br><vr>"
            "News module uses the xdg-open script from freedesktop.org<br>"
            "to open urls.<br>"
            "You need to install the <hilight>xdg-utils package</hilight>, wich includes that script.");
      return 0;
   }

   snprintf(buf, sizeof(buf), "xdg-open \"%s\"", url);
   DBROWSER(("Exec %s", buf));
   exe = ecore_exe_pipe_run(buf, ECORE_EXE_USE_SH, NULL);
   if (exe > 0)
     {
        ecore_exe_free(exe);
     }
   else
     {
        news_util_message_error_show("<hilight>Error when trying to open your browser.</hilight><br><br>"
              "News module uses the <hilight>xdg-open</hilight> script (xdg-utils package) from freedesktop.org<br>"
              "to open urls. The script seems to be present, but maybe it is not"
              "<hilight>configured correctly ?</hilight>");
        return 0;
     }

   return 1;
}

Eina_List *
news_util_lang_detect(void)
{
   Eina_List *list = NULL;
   News_Feed_Lang *lang;
   const char *key, *name;

   lang = E_NEW(News_Feed_Lang, 1);
   lang->key = eina_stringshare_add("en");
   lang->name = eina_stringshare_add("English");
   list = eina_list_append(list, lang);

   //FIXME is that get from env correct ?
   key = getenv("LC_MESSAGES");
   if (!key) key = getenv("LANGUAGE");
   if (!key) key = getenv("LC_ALL");
   if (!key) key = getenv("LANG");
   if (key && strncmp(key, "en", 2) &&
       (name = news_feed_lang_name_get(key)))
     {
        lang = E_NEW(News_Feed_Lang, 1);
        lang->key = eina_stringshare_add(key);
        lang->name = eina_stringshare_add(name);
        list = eina_list_append(list, lang);
     }

   return list;
}

int
news_util_proxy_detect(void)
{
   News_Config *c;
   const char *proxy;
   char *port;
   char buf[1024];

   c = news->config;
   proxy = getenv("HTTP_PROXY");
   if (!proxy) return 0;
   strncpy(buf, proxy, sizeof(buf));
   port = strrchr(buf, ':');
   if (!port || (port == buf)) return 0;
   if (!sscanf(port+1, "%d", &c->proxy.port)) return 0;
   *port = '\0';
   if (c->proxy.host) eina_stringshare_del(c->proxy.host);
   c->proxy.host = eina_stringshare_add(buf);

   return 1;
}

/*
 * Private functions
 *
 */
