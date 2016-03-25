#include "News.h"


struct _E_Config_Dialog_Data
{
   struct
   {
      Evas_Object *button_icon;
      Evas_Object *icon;
      Evas_Object *icon_sel;
      E_Dialog    *icon_sel_dia;
   } gui;

   News_Feed_Category *cat;

   char *name;
   char *icon;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata, News_Feed_Category *cat);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void         _icon_select(void *data1, void *data2);
static void         _icon_select_cb(void *data, Evas_Object *obj);
static void         _icon_select_ok(void *data, E_Dialog *dia);
static void         _icon_select_cancel(void *data, E_Dialog *dia);
static void         _icon_select_changed(void *data);

/*
 * Public functions
 */

int
news_config_dialog_category_show(News_Feed_Category *fcat)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     D_("News Category Configuration"),
                             "News", "_e_modules_news_category_config_dialog",
                             news_theme_file_get(NEWS_THEME_CAT_ICON), 0, v, fcat);

   return 1;
}

void
news_config_dialog_category_hide(News_Feed_Category *fcat)
{
   if (fcat)
     {
        e_object_del(E_OBJECT(fcat->config_dialog));
        fcat->config_dialog = NULL;
     }
   else
     {
        e_object_del(E_OBJECT(news->config_dialog_category_new));
        news->config_dialog_category_new = NULL;
     }
}

/*
 * Private functions
 *
 */

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   News_Feed_Category *cat;

   cat = cfd->data;

   if (cat) cat->config_dialog = cfd;
   else news->config_dialog_category_new = cfd;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata, cat);
   cfd->cfdata = cfdata;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   free(cfdata->name);
   free(cfdata->icon);

   if (cfdata->cat)
     cfdata->cat->config_dialog = NULL;
   else
     news->config_dialog_category_new = NULL;
   free(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata, News_Feed_Category *cat)
{
   char buf[4096];

   cfdata->cat = cat;

   if (cat)
     {
        if (cat->name) cfdata->name = strdup(cat->name);
        if (cat->icon) cfdata->icon = strdup(cat->icon);
     }
   else
     {
        cfdata->name = strdup("");
        snprintf(buf, sizeof(buf), "%s/%s",
                 e_module_dir_get(news->module), NEWS_FEED_ITEM_CAT_ICON_DEFAULT);
        cfdata->icon = strdup(buf);
     }
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ob;
   
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, D_("Icon"), 0);
   ob = e_widget_button_add(evas, "", NULL,
                            _icon_select, cfdata, NULL);
   cfdata->gui.button_icon = ob;
   if (cfdata->icon)
     _icon_select_changed(cfdata);
   e_widget_size_min_set(ob, 48, 48);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.0);

   of = e_widget_framelist_add(evas, D_("Name"), 0);
   ob = e_widget_entry_add(evas, &(cfdata->name), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.0);
   
   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   News_Feed_Category *cat;

   cat = cfdata->cat;
   if (cat)
     {
        if ( !news_feed_category_edit(cat,
                                      cfdata->name,
                                      cfdata->icon) )
          return 0;
     }
   else
     {
        cat = news_feed_category_new(cfdata->name,
                                     cfdata->icon);
        if (!cat) return 0;
        news->config->feed.categories =
           eina_list_append(news->config->feed.categories, cat);

        cfdata->cat = cat;
        news->config_dialog_category_new = NULL;
        cat->config_dialog = cfd;
     }

   news_feed_lists_refresh(1);
   
   return 1;
}

static void
_icon_select(void *data1, void *data2)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_Data *cfdata;
   E_Dialog *dia;
   Evas_Object *o;
   Evas_Coord mw, mh;
   char *dir = NULL;

   cfdata = data1;

   if (cfdata->gui.icon_sel) return;

   if (cfdata->cat) cfd = cfdata->cat->config_dialog;
   else cfd = news->config_dialog_category_new;

   dia = e_dialog_new(cfd->con, "E", "_news_category_icon_select_dialog");
   if (!dia) return;
   e_dialog_title_set(dia, D_("Select an Icon"));
   dia->data = cfdata;

   if (cfdata->icon)
     dir = ecore_file_dir_get(cfdata->icon);
   
   if (dir)
     {
        o = e_widget_fsel_add(dia->win->evas, dir, "/", NULL, NULL,
		              _icon_select_cb, cfdata,
		              NULL, cfdata, 1);
	free(dir);
     }
   else
     {
        o = e_widget_fsel_add(dia->win->evas, "~/", "/", NULL, NULL,
			      _icon_select_cb, cfdata,
			      NULL, cfdata, 1);
     }
   evas_object_show(o);
   cfdata->gui.icon_sel = o;
   e_widget_size_min_get(o, &mw, &mh);
   e_dialog_content_set(dia, o, mw, mh);

   /* buttons at the bottom */
   e_dialog_button_add(dia, D_("OK"), NULL, _icon_select_ok, cfdata);
   e_dialog_button_add(dia, D_("Cancel"), NULL, _icon_select_cancel, cfdata);
   e_dialog_resizable_set(dia, 1);
   e_win_centered_set(dia->win, 1);
   e_dialog_show(dia);
   e_win_resize(dia->win, 475, 341);
   cfdata->gui.icon_sel_dia = dia;
}

static void
_icon_select_cb(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
}

static void
_icon_select_ok(void *data, E_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;
   const char *file;

   cfdata = data;
   file = e_widget_fsel_selection_path_get(cfdata->gui.icon_sel);
   if (file)
     {
        if(cfdata->icon) free(cfdata->icon);
        cfdata->icon = strdup(file);

        _icon_select_changed(cfdata);
     }

   _icon_select_cancel(data, dia);
}

static void
_icon_select_cancel(void *data, E_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   e_object_del(E_OBJECT(dia));
   cfdata->gui.icon_sel = NULL;
   cfdata->gui.icon_sel_dia = NULL;
}

static void
_icon_select_changed(void *data)
{
   E_Config_Dialog_Data *cfdata;
   Evas_Object *ic;

   cfdata = data;

   if (!cfdata->icon) return;

   ic = e_icon_add(evas_object_evas_get(cfdata->gui.button_icon));
   e_icon_file_set(ic, cfdata->icon);
   e_icon_fill_inside_set(ic, 1);

   if (cfdata->gui.icon) evas_object_del(cfdata->gui.icon);

   e_widget_button_icon_set(cfdata->gui.button_icon, ic);
   cfdata->gui.icon = ic;
}
