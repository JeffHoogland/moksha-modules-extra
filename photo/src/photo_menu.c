#include "Photo.h"

static void _cb_deactivate_post(void *data, E_Menu *m);
static void _cb_picture_info(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_picture_next(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_picture_prev(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_picture_setbg(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_picture_viewer(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_pause_toggle(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_configure_item(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_configure_general(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_hist_menu_populate(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_hist_menu_activate(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_hist_menu_populate(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_hist_menu_activate(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_hist_menu_select(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_hist_menu_pre_select(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _cb_hist_menu_post_select(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _cb_hist_menu_deactivate_post(void *data, E_Menu *m);
static void _cb_popi_close(void *data);

void photo_picture_histo_menu_append(Photo_Item *pi, E_Menu *mn_main)
{
   E_Menu_Item *mi;

}

/*
 * Public functions
 */

int photo_menu_show(Photo_Item *pi)
{
   E_Menu *m;
   E_Menu_Item *mi;
   char buf[4096];

   m = e_menu_new();

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, D_("Module Settings"));
   e_util_menu_item_theme_icon_set(mi, "preferences-system");
   e_menu_item_callback_set(mi, _cb_configure_general, NULL);
   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, D_("Settings"));
   e_util_menu_item_theme_icon_set(mi, "preferences-system");
   e_menu_item_callback_set(mi, _cb_configure_item, pi);

   m = e_gadcon_client_util_menu_items_append(pi->gcc, m, 0);
   e_menu_post_deactivate_callback_set(m, _cb_deactivate_post, pi);
   pi->menu = m;

   mi = e_menu_item_new_relative(m, NULL);
   if (pi->config->timer_active)
     {
        e_menu_item_label_set(mi, D_("Pause slideshow"));
	photo_util_menu_icon_set(mi, PHOTO_THEME_ICON_PAUSE);
     }
   else
     {
        e_menu_item_label_set(mi, D_("Resume slideshow"));
	photo_util_menu_icon_set(mi, PHOTO_THEME_ICON_RESUME);
     }
   e_menu_item_callback_set(mi, _cb_pause_toggle, pi);
   mi = e_menu_item_new_relative(m, mi);
   e_menu_item_label_set(mi, D_("Next picture"));
   photo_util_menu_icon_set(mi, PHOTO_THEME_ICON_NEXT);
   e_menu_item_callback_set(mi, _cb_picture_next, pi);
   mi = e_menu_item_new_relative(m, mi);
   e_menu_item_label_set(mi, D_("Previous picture"));
   photo_util_menu_icon_set(mi, PHOTO_THEME_ICON_PREVIOUS);
   e_menu_item_callback_set(mi, _cb_picture_prev, pi);


   mi = e_menu_item_new_relative(m, mi);
   e_menu_item_label_set(mi, D_("Historic"));
   e_menu_item_submenu_pre_callback_set(mi,
					_cb_hist_menu_populate, pi);
   e_menu_item_submenu_post_callback_set(mi,
					 _cb_hist_menu_activate, pi);

   mi = e_menu_item_new_relative(m, mi);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new_relative(m, mi);
   e_menu_item_label_set(mi, D_("Picture informations"));
   photo_util_menu_icon_set(mi, PHOTO_THEME_ICON_INFOS);
   e_menu_item_callback_set(mi, _cb_picture_info, pi);
   mi = e_menu_item_new_relative(m, mi);
   e_menu_item_label_set(mi, D_("Set as background"));
   photo_util_menu_icon_set(mi, PHOTO_THEME_ICON_SETBG);
   e_menu_item_callback_set(mi, _cb_picture_setbg, pi);
   mi = e_menu_item_new_relative(m, mi);
   snprintf(buf, sizeof(buf), "%s %s", D_("Open in"), photo->config->pictures_viewer);
   e_menu_item_label_set(mi, buf);
   photo_util_menu_icon_set(mi, PHOTO_THEME_ICON_VIEWER);
   e_menu_item_callback_set(mi, _cb_picture_viewer, pi);

   return 1;
}

void photo_menu_hide(Photo_Item *pi)
{
   e_menu_post_deactivate_callback_set(pi->menu, NULL, NULL);
   _cb_deactivate_post(pi, pi->menu);
   pi->menu = NULL;
}


/*
 * Private functions
 *
 */

static void
_cb_deactivate_post(void *data, E_Menu *m)
{
   Photo_Item *pi;

   pi = data;
   if (!pi) return;
   if (!pi->menu) return;

   e_object_del(E_OBJECT(pi->menu));
   pi->menu = NULL;
}

static void
_cb_picture_info(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;

   pi = data;
   if (!pi) return;

   photo_item_action_infos(pi);
}

static void
_cb_picture_next(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;

   pi = data;
   if (!pi) return;

   photo_item_action_change(pi, 1);
   photo_item_timer_set(pi, pi->config->timer_active, 0);
}

static void
_cb_picture_prev(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;

   pi = data;
   if (!pi) return;

   photo_item_action_change(pi, -1);
   photo_item_timer_set(pi, pi->config->timer_active, 0);
}

static void
_cb_picture_setbg(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;

   pi = data;
   if (!pi) return;

   photo_item_action_setbg(pi);
}

static void
_cb_picture_viewer(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;

   pi = data;
   if (!pi) return;

   photo_item_action_viewer(pi);
}

static void _cb_pause_toggle(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;

   pi = data;
   if (!pi) return;

   photo_item_action_pause_toggle(pi);
}

static void
_cb_configure_item(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;

   pi = data;
   if (!pi) return;
   if (pi->config_dialog) return;

   photo_config_dialog_item_show(pi);
}

static void
_cb_configure_general(void *data, E_Menu *m, E_Menu_Item *mi)
{
   if (!photo) return;
   if (photo->config_dialog) return;

   photo_config_dialog_show();
}

static void
_cb_hist_menu_select(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;
   int no;

   pi = data;

   no = e_menu_item_num_get(mi);
   photo_item_action_change(pi,
                            pi->histo.pos -
                            (eina_list_count(pi->histo.list) - (no+1)));
   photo_item_timer_set(pi, pi->config->timer_active, 0);
}

static void
_cb_hist_menu_pre_select(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   E_Menu_Item *mi;
   Photo_Item *pi;
   Picture *p;
   char *text;
   int number;

   mi = data;
   if (!mi) return;
   pi = mi->cb.data;
   if (!pi) return;

   number = (eina_list_count(pi->histo.list) - (e_menu_item_num_get(mi)+1));

   DPIC(("Histo menu : Select %d in histo list", number));

   p = eina_list_nth(pi->histo.list, number);
   if (!p) return;

   text = photo_picture_infos_get(p);

   if (pi->histo.popi)
     photo_popup_info_del(pi->histo.popi);

   pi->histo.popi = photo_popup_info_add(pi, p->infos.name, text, p,
                                         PICTURE_HISTO_POPUP_TIMER,
                                         POPUP_INFO_PLACEMENT_CENTERED,
                                         _cb_popi_close, NULL);

   free(text);
}

static void
_cb_hist_menu_post_select(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   E_Menu_Item *mi;
   Photo_Item *pi;

   mi = data;
   if (!mi) return;
   pi = mi->cb.data;
   if (!pi) return;
   
   DPIC(("Histo menu : Post select callback"));

   if (pi->histo.popi)
     photo_popup_info_del(pi->histo.popi);
   pi->histo.popi = NULL;
}

static void
_cb_hist_menu_deactivate_post(void *data, E_Menu *m)
{
   Photo_Item *pi;

   pi = data;
   pi->menu_histo = NULL;
  
   DD(("deactivate cb post"));

   e_object_del(E_OBJECT(m));
}

static void
_cb_hist_menu_populate(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;
   E_Menu *mn;
   Picture *p;
   int pos;

   pi = data;

   mn = e_menu_new();
   pi->menu_histo = mn;
   e_menu_post_deactivate_callback_set(mn, _cb_hist_menu_deactivate_post, pi);
   e_menu_item_submenu_set(mi, mn);

   pos = eina_list_count(pi->histo.list) - 1;
   while ( (p=eina_list_nth(pi->histo.list, pos)) )
     {
        mi = e_menu_item_new(mn);
        e_menu_item_label_set(mi, p->infos.name);
        e_menu_item_radio_group_set(mi, 1);
        e_menu_item_radio_set(mi, 1);
        if (pi->histo.pos == pos)
          e_menu_item_toggle_set(mi, 1);

        e_menu_item_callback_set(mi, _cb_hist_menu_select, pi);
        evas_object_event_callback_add(mi->event_object, EVAS_CALLBACK_MOUSE_IN,
                                       _cb_hist_menu_pre_select, mi);
        evas_object_event_callback_add(mi->event_object, EVAS_CALLBACK_MOUSE_OUT,
                                       _cb_hist_menu_post_select, mi);

        pos--;
     }
}

static void
_cb_hist_menu_activate(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Photo_Item *pi;
   E_Menu *mn;
   Eina_List *l;

   pi = data;
   mn = pi->menu_histo;

   if (!mn) return;

   for (l=mn->items; l; l=eina_list_next(l))
     {
	E_Menu_Item *mi;

	mi = eina_list_data_get(l);
	if (!mi || mi->separator) continue;
	evas_object_event_callback_add(mi->event_object, EVAS_CALLBACK_MOUSE_IN,
				       _cb_hist_menu_pre_select, mi);
	evas_object_event_callback_add(mi->event_object, EVAS_CALLBACK_MOUSE_OUT,
				       _cb_hist_menu_post_select, mi);
     }
}

static void
_cb_popi_close(void *data)
{
   Photo_Item *pi;

   pi = data;
   pi->histo.popi = NULL;
}
