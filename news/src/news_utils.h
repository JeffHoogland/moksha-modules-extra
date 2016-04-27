#ifdef E_MOD_NEWS_TYPEDEFS

#else

#ifndef NEWS_UTILS_H_INCLUDED
#define NEWS_UTILS_H_INCLUDED

#define news_util_debug(args...)                             \
{                                                            \
   char __tmpbuf[4096];                                      \
                                                             \
   snprintf(__tmpbuf, sizeof(__tmpbuf), ##args);             \
   e_module_dialog_show(news->module, D_("News Module DEBUG DEBUG DEBUG DEBUG"), __tmpbuf); \
}
#define news_util_message_show(args...)                            \
{                                                                  \
   char __tmpbuf[4096];                                            \
                                                                   \
   snprintf(__tmpbuf, sizeof(__tmpbuf), ##args);                   \
   e_module_dialog_show(news->module, D_("News Module"), __tmpbuf); \
}
#define news_util_message_error_show(args...)                            \
{                                                                        \
   char __tmpbuf[4096];                                                  \
                                                                         \
   snprintf(__tmpbuf, sizeof(__tmpbuf), ##args);                         \
   e_module_dialog_show(news->module, D_("News Module Error"), __tmpbuf); \
}

int         news_util_url_image_get(char *url, void (*func_cb) (News_Feed_Document *doc, int error));
int         news_util_ecanvas_geometry_get(int *canvas_w, int *canvas_h);
const char *news_util_convert_locale_to_utf8(char *txt);
const char *news_util_convert_utf8_to_locale(char *txt);
int         news_util_datecmp(const struct tm *t1, const struct tm *t2);
int         news_util_browser_open(const char *url);
Eina_List  *news_util_lang_detect(void);
int         news_util_proxy_detect(void);

#endif
#endif
