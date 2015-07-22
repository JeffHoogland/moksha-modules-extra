#include "e_mod_main.h"

static void      _ng_border_cb_border_menu_end(void *data, E_Menu *m);
static void      _ng_border_menu_cb_close(void *data, E_Menu *m, E_Menu_Item *mi);
static void      _ng_border_menu_cb_iconify(void *data, E_Menu *m, E_Menu_Item *mi);
static void      _ng_border_menu_cb_maximize(void *data, E_Menu *m, E_Menu_Item *mi);
static void      _ng_border_menu_cb_unmaximize(void *data, E_Menu *m, E_Menu_Item *mi);
static void      _ng_border_menu_cb_fullscreen(void *data, E_Menu *m, E_Menu_Item *mi);

void
ngi_border_menu_show(Ngi_Box *box, E_Border *bd, Evas_Coord x, Evas_Coord y, int dir, Ecore_X_Time timestamp)
{
   E_Menu *m;
   E_Menu_Item *mi;
   Ng *ng = box->ng;

   if (bd->border_menu)
      return;

   m = e_menu_new();
   e_menu_category_set(m,"border");
   e_menu_category_data_set("border",bd);
   e_object_data_set(E_OBJECT(m), bd);
   bd->border_menu = m;
   e_menu_post_deactivate_callback_set(m, _ng_border_cb_border_menu_end, ng); /* FIXME */

   if ((!bd->lock_user_fullscreen) && (!bd->shaded))
     {
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Fullscreen"));
        e_menu_item_check_set(mi, 1);
        e_menu_item_toggle_set(mi, bd->fullscreen);
        e_menu_item_callback_set(mi, _ng_border_menu_cb_fullscreen, bd);
        e_menu_item_icon_edje_set(mi,
                                  e_theme_edje_file_get("base/theme/borders",
                                                        "e/widgets/border/default/fullscreen"),
                                  "e/widgets/border/default/fullscreen");
     }

   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);
   if (!bd->lock_close)
     {
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Close"));
        e_menu_item_callback_set(mi, _ng_border_menu_cb_close, bd);
        e_menu_item_icon_edje_set(mi,
                                  e_theme_edje_file_get("base/theme/borders",
                                                        "e/widgets/border/default/close"),
                                  "e/widgets/border/default/close");
     }

   if (!bd->internal)
     {
        mi = e_menu_item_new(m);
        e_menu_item_separator_set(mi, 1);

        if (!(((bd->client.icccm.min_w == bd->client.icccm.max_w) &&
               (bd->client.icccm.min_h == bd->client.icccm.max_h)) ||
              (bd->lock_user_maximize)))
          {
             if ((!bd->lock_user_maximize) && (!bd->shaded) &&
                 ((bd->layer == 50) || (bd->layer == 100) || (bd->layer == 150)))
               {
                  mi = e_menu_item_new(m);
                  e_menu_item_label_set(mi, D_("Maximized"));
                  e_menu_item_check_set(mi, 1);
                  e_menu_item_toggle_set(mi, (bd->maximized & E_MAXIMIZE_DIRECTION) == E_MAXIMIZE_BOTH);
                  if((bd->maximized & E_MAXIMIZE_DIRECTION) == E_MAXIMIZE_BOTH)
                     e_menu_item_callback_set(mi, _ng_border_menu_cb_unmaximize, bd);
                  else
                     e_menu_item_callback_set(mi, _ng_border_menu_cb_maximize, bd);

                  e_menu_item_icon_edje_set(mi,
                                            e_theme_edje_file_get("base/theme/borders",
                                                                  "e/widgets/border/default/maximize"),
                                            "e/widgets/border/default/maximize");
               }
          }
     }

   if (!bd->lock_user_iconify)
     {
        mi = e_menu_item_new(m);
        if(bd->iconic)
           e_menu_item_label_set(mi, D_("Uniconify"));
        else
           e_menu_item_label_set(mi, D_("Iconify"));

        e_menu_item_callback_set(mi, _ng_border_menu_cb_iconify, bd);
        e_menu_item_icon_edje_set(mi,
                                  e_theme_edje_file_get("base/theme/borders",
                                                        "e/widgets/border/default/minimize"),
                                  "e/widgets/border/default/minimize");
     }

   e_menu_activate_mouse(m, ng->zone, ng->zone->x + x, ng->zone->y + y, 1, 1,
                         dir, timestamp);
}

static void
_ng_border_cb_border_menu_end(void *data, E_Menu *m)
{
   E_Border *bd;

   bd = (E_Border *)e_object_data_get(E_OBJECT(m));
   if (bd)
     {
        /* If the border exists, delete all associated menus */
        e_int_border_menu_del(bd);
     }
   else
     {
        /* Just delete this menu */
        e_object_del(E_OBJECT(m));
     }

   //  if (ng) ngi_thaw(ng);
}

static void
_ng_border_menu_cb_close(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd = (E_Border *)data;
   if (!bd->lock_close)
      e_border_act_close_begin(bd);
}

static void
_ng_border_menu_cb_iconify(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd = (E_Border *)data;
   if (!bd->lock_user_iconify)
     {
        if (bd->iconic)
           e_border_uniconify(bd);
        else
           e_border_iconify(bd);
     }
}

static void
_ng_border_menu_cb_maximize(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd = (E_Border *)data;
   if (!bd->lock_user_maximize)
      e_border_maximize(bd, (e_config->maximize_policy & E_MAXIMIZE_TYPE) |
                        E_MAXIMIZE_BOTH);
}

static void
_ng_border_menu_cb_unmaximize(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd = (E_Border *)data;
   e_border_unmaximize(bd, E_MAXIMIZE_BOTH);
}

static void
_ng_border_menu_cb_fullscreen(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd = (E_Border *)data;
   int toggle;

   if (!bd)
      return;

   if (!bd->lock_user_fullscreen)
     {
        toggle = e_menu_item_toggle_get(mi);
        if (toggle)
           e_border_fullscreen(bd, (E_Fullscreen)e_config->fullscreen_policy);
        else
           e_border_unfullscreen(bd);
     }
}

