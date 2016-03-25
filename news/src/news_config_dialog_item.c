#include "News.h"

#define DIALOG_CLASS "_e_modules_news_item_config_dialog"

struct _E_Config_Dialog_Data
{
   int view_mode;
   int openmethod;
   int browser_open_home;

   int apply_to_all;

   News_Item *ni;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata, News_Item *pi);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

/*
 * Public functions
 */

int
news_config_dialog_item_show(News_Item *ni)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("News Gadget Configuration"),
                             "News", DIALOG_CLASS,
                             news_theme_file_get(NEWS_THEME_CAT_ICON), 0, v, ni);

   return 1;
}

void
news_config_dialog_item_hide(News_Item *ni)
{
   e_object_del(E_OBJECT(ni->config_dialog));
   ni->config_dialog = NULL;
}

/*
 * Private functions
 *
 */

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   News_Item *ni;

   ni = cfd->data;
   
   ni->config_dialog = cfd;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata, ni);
   cfd->cfdata = cfdata;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   cfdata->ni->config_dialog = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata, News_Item *ni)
{
   News_Config_Item *nic;

   nic = ni->config;
   cfdata->ni = ni;
   cfdata->view_mode = nic->view_mode;
   cfdata->openmethod = nic->openmethod;
   cfdata->browser_open_home = nic->browser_open_home;
   cfdata->apply_to_all = nic->apply_to_all;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o;
   Evas_Object *of, *ob;
   E_Radio_Group *rg;
   
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, D_("View Mode"), 0);

   rg = e_widget_radio_group_new(&(cfdata->view_mode));
   ob = e_widget_radio_add(evas, D_("Show one icon for all feeds"), NEWS_ITEM_VIEW_MODE_ONE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Show all feeds"), NEWS_ITEM_VIEW_MODE_FEED, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Show unread feeds"), NEWS_ITEM_VIEW_MODE_FEED_UNREAD, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Show all feeds marked as important"), NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("Show unread feeds marked as important"), NEWS_ITEM_VIEW_MODE_FEED_IMPORTANT_UNREAD, rg);
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o, of, 1, 1, 0.0);

   of = e_widget_framelist_add(evas, D_("Feed open in"), 0);

   rg = e_widget_radio_group_new(&(cfdata->openmethod));
   ob = e_widget_radio_add(evas, D_("the feed viewer"), NEWS_ITEM_OPENMETHOD_VIEWER, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, D_("my favorite browser"), NEWS_ITEM_OPENMETHOD_BROWSER, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, D_("Browser opens home url"), &(cfdata->browser_open_home));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, D_("instead of feed url"));
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o, of, 1, 1, 0.0);

   ob = e_widget_label_add(evas, "");
   e_widget_list_object_append(o, ob, 1, 1, 0.0);
   ob = e_widget_check_add(evas, D_("Apply this to all News gadgets"), &(cfdata->apply_to_all));
   e_widget_list_object_append(o, ob, 1, 1, 0.0);
   
   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   News_Config_Item *nic, *nic2;
   News_Item *ni, *ni2;
   Eina_List *l;
   
   ni = cfdata->ni;
   nic = ni->config;

   if (!cfdata->apply_to_all)
     {
        nic->openmethod = cfdata->openmethod;
        nic->browser_open_home = cfdata->browser_open_home;
        if (nic->view_mode != cfdata->view_mode)
          {
             nic->view_mode = cfdata->view_mode;
             news_item_refresh(ni, 1, 0, 1);
          }
     }
   else
     {
        for (l=news->items; l; l=eina_list_next(l))
          {
             ni2 = l->data;
             nic2 = ni2->config;
             nic2->openmethod = cfdata->openmethod;
             nic2->browser_open_home = cfdata->browser_open_home;
             if (nic2->view_mode != cfdata->view_mode)
               {
                  nic2->view_mode = cfdata->view_mode;
                  news_item_refresh(ni2, 1, 0, 1);
               }
          }
     }
   nic->apply_to_all = cfdata->apply_to_all;
   
   news_config_save();
   return 1;
}
