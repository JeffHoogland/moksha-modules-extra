#include "e_mod_main.h"


static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata EINA_UNUSED)
{
   mod->cfd = NULL;
   mod->label = mod->cfd_list[0] = mod->cfd_list[1] = NULL;
}
/*
static void *
_create_data(E_Config_Dialog *cfd EINA_UNUSED)
{
   return NULL;
}

static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata EINA_UNUSED)
{
   return 0;
}

static int
_basic_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata EINA_UNUSED)
{
   return 0;
}
*/

static void
_obj_del(Echievement *ec, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (obj == ec->dialog.icon)
     ec->dialog.icon = NULL;
   else
     ec->dialog.label = NULL;
}

static Evas_Object *
_item_create(Evas *evas, Echievement *ec, Eina_Bool save)
{
   Evas_Object *table, *icon, *label;
   char progress[128];
   unsigned int goal, counter;

   etrophy_trophy_goal_get(ec->trophy, &goal, &counter);
   table = e_widget_frametable_add(evas, D_(etrophy_trophy_name_get(ec->trophy)), 1);

   icon = e_widget_image_add_from_file(evas, PACKAGE_DATA_DIR "/trophy.png",
                                       64, 64);
   if (save) ec->dialog.icon = icon;
   evas_object_event_callback_add(icon, EVAS_CALLBACK_DEL, (Evas_Object_Event_Cb)_obj_del, ec);
   e_widget_frametable_object_append(table, icon, 0, 0, 1, 2, 1, 1, 1, 0);
   label = e_widget_label_add(evas, D_(etrophy_trophy_description_get(ec->trophy)));
   e_widget_frametable_object_append(table, label, 1, 0, 3, 1, 1, 1, 1, 0);

   if (etrophy_trophy_earned_get(ec->trophy))
     {
        char date[256];
        char buf[512];
        time_t t0;

        t0 = etrophy_trophy_date_get(ec->trophy);
        strftime(date, sizeof(date), D_("%d %B %Y - %r"), localtime(&t0));
        snprintf(buf, sizeof(buf), D_("Achieved: %s"), date);
        label = e_widget_label_add(evas, buf);
     }
   else if (goal < 2)
     label = e_widget_label_add(evas, D_("Not achieved"));
   else
     {
        snprintf(progress, sizeof(progress), D_("Progress: %u/%u"), counter, goal);
        label = e_widget_label_add(evas, progress);
     }
   if (save) ec->dialog.label = label;
   evas_object_event_callback_add(label, EVAS_CALLBACK_DEL, (Evas_Object_Event_Cb)_obj_del, ec);
   e_widget_frametable_object_append(table, label, 1, 1, 3, 1, 1, 1, 1, 0);

   return table;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata EINA_UNUSED)
{
   Evas_Object *toolbook, *list, *item, *sf;
   Echievement *ec;
   int mw, mh, mww;

   toolbook = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);

   /* FIXME it should be scrollable and all initial items should
      be visible */
   mod->cfd_list[0] = list = e_widget_list_add(evas, 1, 0);
   EINA_INLIST_FOREACH(EINA_INLIST_GET(mod->trophies_list), ec)
     {
        if (!etrophy_trophy_earned_get(ec->trophy)) continue;
        item = _item_create(evas, ec, EINA_FALSE);
        e_widget_list_object_append(list, item, 1, 1, 0.5);
     }
   e_widget_size_min_get(list, &mw, &mh);

   if (mw < 320) mw = 320;
   if (mh < 220) mh = 220;

   evas_object_resize(list, mw, mh);
   sf = e_widget_scrollframe_simple_add(evas, list);
   e_widget_size_min_set(sf, 320, 220);
   e_widget_toolbook_page_append(toolbook, NULL, D_("My Trophies"),
                                 sf, 1, 1, 1, 1, 0.5, 0.0);

   mod->cfd_list[1] = list = e_widget_list_add(evas, 1, 0);

   EINA_INLIST_FOREACH(EINA_INLIST_GET(mod->trophies_list), ec)
     {
        if ((!etrophy_trophy_earned_get(ec->trophy)) && (!etrophy_trophy_visibility_get(ec->trophy))) continue;
        item = _item_create(evas, ec, EINA_TRUE);
        e_widget_list_object_append(list, item, 1, 1, 0.5);
     }
   e_widget_size_min_get(list, &mww, &mh);

   if (mww < mw) mww = mw;
   if (mh < 220) mh = 220;

   evas_object_resize(list, mww, mh);
   sf = e_widget_scrollframe_simple_add(evas, list);
   e_widget_size_min_set(sf, 320, 220);
   e_widget_toolbook_page_append(toolbook, NULL, D_("All Trophies"),
                                 sf, 1, 1, 1, 1, 0.5, 0.0);

   e_widget_toolbook_page_show(toolbook, 0);
   list = e_widget_list_add(evas, 0, 0);
   e_widget_list_object_append(list, toolbook, 1, 1, 0.5);
   {
      char buf[1024];
      unsigned score = etrophy_gamescore_trophies_points_get(ech_config->gs);

      /* FIXME: this can look better */
      snprintf(buf, sizeof(buf), DP_("%u Echievement Point", "%u Echievement Points", score), score);
      mod->label = item = e_widget_label_add(evas, buf);
      e_widget_list_object_append(list, item, 0, 0, 0.5);
   }
   return list;
}

static void
_totals_update(void)
{
   char buf[1024];
   unsigned score = etrophy_gamescore_trophies_points_get(ech_config->gs);

   snprintf(buf, sizeof(buf), DP_("%u Echievement Point", "%u Echievement Points", score), score);
   e_widget_label_text_set(mod->label, buf);
}

E_Config_Dialog *
e_int_config_echievements(E_Container *con, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;
   char buf[4096];

   if (e_config_dialog_find("Echievements", "extensions/echievements"))
     return NULL;

   mod->obj_del_cb = (Evas_Object_Event_Cb)_obj_del;
   v = E_NEW(E_Config_Dialog_View, 1);
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
/*
   v->create_cfdata = _create_data;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.check_changed = _basic_check_changed;
*/
   snprintf(buf, sizeof(buf), "%s/e-module-echievements.edj",
            mod->module->dir);

   cfd = e_config_dialog_new(con, D_("Echievements"), "Echievements",
                             "extensions/echievements", buf, 0, v, NULL);

   e_dialog_resizable_set(cfd->dia, 1);
   mod->cfd = cfd;
   return cfd;
}

EINTERN void
ech_cfg_ech_update(Echievement *ec)
{
   char progress[128];
   unsigned int goal, counter;

   if (!mod->cfd) return;
   if ((!ec->dialog.icon) || (!ec->dialog.label)) return;
   etrophy_trophy_goal_get(ec->trophy, &goal, &counter);
   if (etrophy_trophy_earned_get(ec->trophy))
     {
        char date[256];
        char buf[512];
        time_t t0;

        t0 = etrophy_trophy_date_get(ec->trophy);
        strftime(date, sizeof(date), D_("%d %B %Y - %r"), localtime(&t0));
        snprintf(buf, sizeof(buf), D_("Achieved: %s"), date);
        e_widget_label_text_set(ec->dialog.label, buf);
     }
   else if (goal < 2)
     e_widget_label_text_set(ec->dialog.label, D_("Not achieved"));
   else
     {
        snprintf(progress, sizeof(progress), D_("Progress: %u/%u"), counter, goal);
        e_widget_label_text_set(ec->dialog.label, progress);
     }
   e_widget_frametable_object_repack(e_widget_parent_get(ec->dialog.label), ec->dialog.label, 1, 1, 3, 1, 1, 1, 1, 0);
   _totals_update();
}

EINTERN void
ech_cfg_ech_add(Echievement *ec)
{
   Evas *e;
   Evas_Object *item;
   int mh;
   int w;

   if (!mod->cfd) return;
   /* use "all trophies" list current width
    * since it's guaranteed to be correct for the moment
    */
   evas_object_geometry_get(mod->cfd_list[1], NULL, NULL, &w, NULL);
   e = evas_object_evas_get(mod->cfd_list[0]);
   item = _item_create(e, ec, EINA_FALSE);
   e_widget_list_object_append(mod->cfd_list[0], item, 1, 1, 0.5);
   e_widget_size_min_get(mod->cfd_list[0], NULL, &mh);

   evas_object_resize(mod->cfd_list[0], w, mh);

   if (ec->dialog.icon || ec->dialog.label) return;

   item = _item_create(e, ec, EINA_TRUE);
   e_widget_list_object_append(mod->cfd_list[1], item, 1, 1, 0.5);
   e_widget_size_min_get(mod->cfd_list[1], NULL, &mh);
   evas_object_resize(mod->cfd_list[1], w, mh);
   _totals_update();
}
