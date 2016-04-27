#include "News.h"

#define DIALOG_CLASS "_e_modules_news_langs_config_dialog"

struct _Lang_Choice
{
   News_Feed_Lang *lang;
   Evas_Object    *check;
   int ok;
};

typedef struct _Lang_Choice Lang_Choice;

struct _E_Config_Dialog_Data
{
   Eina_List *choices;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void         _cb_lang_change(void *data, Evas_Object *obj);

/*
 * Public functions
 */

int
news_config_dialog_langs_show(void)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("News", DIALOG_CLASS))
     return 0;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create_widgets;
   
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("News Feeds Language Configuration"),
                             "News", DIALOG_CLASS,
                             news_theme_file_get(NEWS_THEME_CAT_ICON), 0, v, NULL);
   return 1;
}

void
news_config_dialog_langs_hide()
{
   e_object_del(E_OBJECT(news->config_dialog_langs));
   news->config_dialog_langs = NULL;
}

/*
 * Private functions
 *
 */

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;

   news->config_dialog_langs = cfd;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   cfd->cfdata = cfdata;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   Lang_Choice *ch;

   while ((ch = eina_list_data_get(cfdata->choices)))
     {
        free(ch);
        cfdata->choices = eina_list_remove_list(cfdata->choices, cfdata->choices);
     }

   news->config_dialog_langs = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   News_Feed_Lang *lang;
   Lang_Choice *choice;
   Eina_List *l;

   for (l=news->langs; l; l=eina_list_next(l))
     {
        lang = l->data;
        choice = E_NEW(Lang_Choice, 1);
        choice->lang = lang;
        choice->ok = news_feed_lang_selected_is(lang->key);
        cfdata->choices = eina_list_append(cfdata->choices, choice);
     }
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *ol, *of, *o;
   Evas_Object *ob;
   Lang_Choice *choice;
   Eina_List *l;
   char buf[4096];
   int line;
   int count;

   o = e_widget_list_add(evas, 0, 0);

   if (news->config->feed.langs_notset)
     {
        ob = e_widget_label_add(evas, D_("Welcome to News Module !"));
        e_widget_list_object_append(o, ob, 1, 1, 0.5);
        ob = e_widget_label_add(evas, D_("Please select the languages that you are able to read"));
        e_widget_list_object_append(o, ob, 1, 1, 0.0);
        ob = e_widget_label_add(evas, "");
        e_widget_list_object_append(o, ob, 1, 1, 0.0);
     }

   l = cfdata->choices;
   line = 0;
   of = e_widget_framelist_add(evas, D_("Languages"), 1);
   ol = e_widget_list_add(evas, 0, 0);
   for (l=cfdata->choices; l; l=eina_list_next(l))
     {
        choice = l->data;

        count = 0;
        
        NEWS_FEED_FOREACH_BEG();
        if (!strncmp(_feed->language, choice->lang->key, 2))
          count++;
        NEWS_FEED_FOREACH_END();

        if (count)
          snprintf(buf, sizeof(buf), "%s (%d)", choice->lang->name, count);
        else
          snprintf(buf, sizeof(buf), "%s", choice->lang->name);
        ob = e_widget_check_add(evas, buf, &(choice->ok));
        e_widget_on_change_hook_set(ob, _cb_lang_change, cfdata);
        e_widget_list_object_append(ol, ob, 1, 1, 0.0);

        line++;
        if (line > 10)
          {
             e_widget_framelist_object_append(of, ol);
             ol = e_widget_list_add(evas, 0, 0);
             line = 0;
          }
     }
   e_widget_framelist_object_append(of, ol);

   e_widget_list_object_append(o, of, 1, 1, 0.0);

   return o;
}

static void
_cb_lang_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Lang *lang;
   Lang_Choice *choice;
   Eina_List *list, *l;

   cfdata = data;

   news_feed_lang_list_free(news->config->feed.langs);
   list = NULL;
   for (l=cfdata->choices; l; l=eina_list_next(l))
     {
        choice = l->data;
        if (choice->ok)
          {
             lang = E_NEW(News_Feed_Lang, 1);
             lang->key = eina_stringshare_add(choice->lang->key);
             lang->name = eina_stringshare_add(choice->lang->name);
             list = eina_list_append(list, lang);
          }
     }
   news->config->feed.langs = list;

   /* refreshes ! */
   news_feed_lang_list_refresh();
   news_feed_lists_refresh(0);

   news_config_save();
}
