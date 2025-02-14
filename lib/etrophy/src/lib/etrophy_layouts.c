#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>

#include "Etrophy.h"

#ifdef  __cplusplus
extern "C" {
#endif

static Elm_Genlist_Item_Class itc_group;
static Elm_Genlist_Item_Class itc;

static char *
_group_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   const Etrophy_Level *level = data;
   char buf[128];

   snprintf(buf, sizeof(buf), "Level %s", etrophy_level_name_get(level));
   return strdup(buf);
}

static char *
_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part)
{
   const Etrophy_Score *score = data;
   char buf[128];

   if (!strcmp(part, "elm.text"))
     {
        const char *name = etrophy_score_player_name_get(score);
        if (name)
          return strdup(name);
        return NULL;
     }

   snprintf(buf, sizeof(buf), "%i", etrophy_score_score_get(score));
   return strdup(buf);
}

static void
_score_level_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *genlist = data;
   Elm_Object_Item *item;
   int value, i = 0;

   value = elm_spinner_value_get(obj);
   item = elm_genlist_first_item_get(genlist);
   while (item)
     {
        if (elm_genlist_item_item_class_get(item) == &itc_group)
          if (value == i++)
            elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
        item = elm_genlist_item_next_get(item);
     }
}

EAPI Evas_Object *
etrophy_score_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore)
{
   Evas_Object *layout, *spinner, *genlist;
   const Eina_List *levels, *l;
   Etrophy_Level *level;
   int i = 0;

   layout = elm_layout_add(parent);
   elm_layout_file_set(layout, PACKAGE_DATA_DIR "/etrophy.edj",
                       "etrophy/scores/default");
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

   genlist = elm_genlist_add(layout);
   elm_genlist_homogeneous_set(genlist, EINA_TRUE);
   evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set(layout, "list", genlist);

   spinner = elm_spinner_add(layout);
   elm_spinner_wrap_set(spinner, EINA_TRUE);
   elm_spinner_step_set(spinner, 1);
   elm_spinner_editable_set(spinner, EINA_FALSE);
   evas_object_smart_callback_add(spinner, "delay,changed",
                                  _score_level_changed_cb, genlist);
   evas_object_size_hint_weight_set(spinner, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(spinner, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set(layout, "selector", spinner);

   itc_group.item_style = "group_index";
   itc_group.func.text_get = _group_text_get;
   itc_group.func.content_get = NULL;
   itc_group.func.state_get = NULL;
   itc_group.func.del = NULL;

   itc.item_style = "double_label";
   itc.func.text_get = _text_get;
   itc.func.content_get = NULL;
   itc.func.state_get = NULL;
   itc.func.del = NULL;

   levels = etrophy_gamescore_levels_list_get(gamescore);
   elm_spinner_min_max_set(spinner, 0, eina_list_count(levels) - 1);

   EINA_LIST_FOREACH(levels, l, level)
     {
        Elm_Object_Item *group;
        const Eina_List *scores, *s;
        Etrophy_Score *score;
        char buf[128];

        snprintf(buf, sizeof(buf), "Level %s", etrophy_level_name_get(level));
        elm_spinner_special_value_add(spinner, i++, buf);

        group = elm_genlist_item_append(genlist, &itc_group, level, NULL,
                                        ELM_GENLIST_ITEM_GROUP, NULL, NULL);
        elm_genlist_item_select_mode_set(group,
                                         ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

        scores = etrophy_level_scores_list_get(level);
        EINA_LIST_FOREACH(scores, s, score)
          {
             Elm_Object_Item *item;
             item = elm_genlist_item_append(genlist, &itc, score, NULL,
                                            ELM_GENLIST_ITEM_NONE, NULL, NULL);
             elm_genlist_item_select_mode_set(
               item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
          }
     }

   return layout;
}

EAPI Evas_Object *
etrophy_trophies_layout_add(Evas_Object *parent __UNUSED__, Etrophy_Gamescore *gamescore __UNUSED__)
{
   /* TODO */
   return NULL;
}

EAPI Evas_Object *
etrophy_locks_layout_add(Evas_Object *parent __UNUSED__, Etrophy_Gamescore *gamescore __UNUSED__)
{
   /* TODO */
   return NULL;
}

#ifdef  __cplusplus
}
#endif
