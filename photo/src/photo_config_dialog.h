#ifdef E_MOD_PHOTO_TYPEDEFS



#else

#ifndef PHOTO_CONFIG_DIALOG_H_INCLUDED
#define PHOTO_CONFIG_DIALOG_H_INCLUDED



#define CONFIG_DIALOG_ACTION_CHOICE(of, col, line, action_var, first)     \
rg = e_widget_radio_group_new(&(action_var));                             \
ob = e_widget_radio_add(evas, "", first, rg);                             \
e_widget_frametable_object_append(of, ob, col, line, 1, 1, 1, 1, 1, 1);   \
ob = e_widget_radio_add(evas, "", ITEM_ACTION_LABEL, rg);                 \
e_widget_frametable_object_append(of, ob, col, line+1, 1, 1, 1, 1, 1, 1); \
ob = e_widget_radio_add(evas, "", ITEM_ACTION_INFOS, rg);                 \
e_widget_frametable_object_append(of, ob, col, line+2, 1, 1, 1, 1, 1, 1); \
ob = e_widget_radio_add(evas, "", ITEM_ACTION_PREV, rg);                  \
e_widget_frametable_object_append(of, ob, col, line+3, 1, 1, 1, 1, 1, 1); \
ob = e_widget_radio_add(evas, "", ITEM_ACTION_NEXT, rg);                  \
e_widget_frametable_object_append(of, ob, col, line+4, 1, 1, 1, 1, 1, 1); \
ob = e_widget_radio_add(evas, "", ITEM_ACTION_PAUSE, rg);                 \
e_widget_frametable_object_append(of, ob, col, line+5, 1, 1, 1, 1, 1, 1); \
ob = e_widget_radio_add(evas, "", ITEM_ACTION_SETBG, rg);                 \
e_widget_frametable_object_append(of, ob, col, line+6, 1, 1, 1, 1, 1, 1); \
ob = e_widget_radio_add(evas, "", ITEM_ACTION_VIEWER, rg);                \
e_widget_frametable_object_append(of, ob, col, line+7, 1, 1, 1, 1, 1, 1); \
ob = e_widget_radio_add(evas, "", ITEM_ACTION_MENU, rg);                  \
e_widget_frametable_object_append(of, ob, col, line+8, 1, 1, 1, 1, 1, 1);

#define CONFIG_DIALOG_ACTION_CHOICE_LABELS(of, col, line, first)          \
ob = e_widget_label_add(evas, first);                                     \
e_widget_frametable_object_append(of, ob, col, line, 1, 1, 1, 1, 1, 1);   \
ob = e_widget_label_add(evas, D_("Show label"));                           \
e_widget_frametable_object_append(of, ob, col, line+1, 1, 1, 1, 1, 1, 1); \
ob = e_widget_label_add(evas, D_("Show informations"));                    \
e_widget_frametable_object_append(of, ob, col, line+2, 1, 1, 1, 1, 1, 1); \
ob = e_widget_label_add(evas, D_("Previous picture"));                     \
e_widget_frametable_object_append(of, ob, col, line+3, 1, 1, 1, 1, 1, 1); \
ob = e_widget_label_add(evas, D_("Next picture"));                         \
e_widget_frametable_object_append(of, ob, col, line+4, 1, 1, 1, 1, 1, 1); \
ob = e_widget_label_add(evas, D_("Pause"));                                \
e_widget_frametable_object_append(of, ob, col, line+5, 1, 1, 1, 1, 1, 1); \
ob = e_widget_label_add(evas, D_("Set as background"));                    \
e_widget_frametable_object_append(of, ob, col, line+6, 1, 1, 1, 1, 1, 1); \
ob = e_widget_label_add(evas, D_("Open viewer"));                          \
e_widget_frametable_object_append(of, ob, col, line+7, 1, 1, 1, 1, 1, 1); \
ob = e_widget_label_add(evas, D_("Show menu"));                            \
e_widget_frametable_object_append(of, ob, col, line+8, 1, 1, 1, 1, 1, 1);


int  photo_config_dialog_show(void);
void photo_config_dialog_hide(void);

void photo_config_dialog_refresh_local_infos(void);
void photo_config_dialog_refresh_local_dirs(void);
void photo_config_dialog_refresh_local_load(void);

#endif
#endif
