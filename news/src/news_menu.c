#include "News.h"

static void _menu_append(E_Menu *mn, News_Item *ni);

static void _cb_configure_feed(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_configure_item(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_configure_item_content(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_configure_main(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_configure_feeds(void *data, E_Menu *m, E_Menu_Item *mi);

static void _cb_item_deactivate_post(void *data, E_Menu *m);
static void _cb_item_setasread(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cb_item_update(void *data, E_Menu *m, E_Menu_Item *mi);

static void _cb_feed_deactivate_post(void *data, E_Menu *m);

static void _cb_browser_deactivate_post(void *data, E_Menu *m);
static void _cb_browser_feed(void *data, E_Menu *m, E_Menu_Item *mi);

/*
 * Public functions
 */

int
news_menu_item_show(News_Item *ni)
{
   E_Menu *mn;

   mn = e_menu_new();
   mn = e_gadcon_client_util_menu_items_append(ni->gcc, mn, 0);
   e_menu_post_deactivate_callback_set(mn, _cb_item_deactivate_post, ni);
   _menu_append(mn, ni);

   ni->menu = mn;

   return 1;
}

void
news_menu_item_hide(News_Item *ni)
{
   e_menu_post_deactivate_callback_set(ni->menu, NULL, NULL);
   _cb_item_deactivate_post(ni, ni->menu);
}

int
news_menu_feed_show(News_Feed *f)
{
   E_Menu *mn;
   E_Menu_Item *mi;
   char buf[256];

   mn = e_menu_new();

   mn = e_gadcon_client_util_menu_items_append(f->item->gcc, mn, 0);
   e_menu_post_deactivate_callback_set(mn, _cb_feed_deactivate_post, f);

   _menu_append(mn, f->item);

   mi = e_menu_item_new_relative(mn, NULL);
   snprintf(buf, sizeof(buf), "Feed (%s) Properties", f->name);
   e_menu_item_label_set(mi, D_(buf));
   if (f->icon && f->icon[0])
     e_menu_item_icon_file_set(mi, f->icon);
   e_menu_item_callback_set(mi, _cb_configure_feed, f);

   f->menu = mn;

   return 1;
}

void
news_menu_feed_hide(News_Feed *f)
{
   e_menu_post_deactivate_callback_set(f->menu, NULL, NULL);
   _cb_feed_deactivate_post(f, f->menu);
}

int
news_menu_browser_show(News_Item *ni)
{
   E_Menu *mn;
   E_Menu_Item *mi;
   char buf[4096];

   mn = e_menu_new();
   e_menu_post_deactivate_callback_set(mn, _cb_browser_deactivate_post, ni);

   NEWS_ITEM_FEEDS_FOREACH_BEG(ni);
   if (!_feed->doc) continue;
   mi = e_menu_item_new(mn);
   if (_feed->doc->unread_count)
     snprintf(buf, sizeof(buf), "[UNREAD] %s", _feed->name);
   else
     snprintf(buf, sizeof(buf), "%s", _feed->name);
   e_menu_item_label_set(mi, buf);
   if (_feed->icon && _feed->icon[0])
     e_menu_item_icon_file_set(mi, _feed->icon);
   e_menu_item_callback_set(mi, _cb_browser_feed, _feed);
   NEWS_ITEM_FEEDS_FOREACH_END();

   ni->menu_browser = mn;

   return 1;
}

void
news_menu_browser_hide(News_Item *ni)
{
   e_menu_post_deactivate_callback_set(ni->menu_browser, NULL, NULL);
   _cb_browser_deactivate_post(ni, ni->menu_browser);
}


/*
 * Private functions
 *
 */

static void
_menu_append(E_Menu *ma, News_Item *ni)
{
   E_Menu *mn;
   E_Menu_Item *mi;

   if (ni->config->feed_refs)
     {
        if (ni->unread_count)
          {
             mi = e_menu_item_new(ma);
             e_menu_item_label_set(mi, D_("Set all as read"));
             news_theme_menu_icon_set(mi, NEWS_THEME_ICON_SETASREAD);
             e_menu_item_callback_set(mi, _cb_item_setasread, ni);
          }

        mi = e_menu_item_new(ma);
        e_menu_item_label_set(mi, D_("Update"));
        news_theme_menu_icon_set(mi, NEWS_THEME_ICON_UPDATE);
        e_menu_item_callback_set(mi, _cb_item_update, ni);
     }

   mn = e_menu_new();

   mi = e_menu_item_new_relative(ma, NULL);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, D_("Gadget"));
   e_util_menu_item_theme_icon_set(mi, "preferences-system");
   e_menu_item_callback_set(mi, _cb_configure_item, ni);

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, D_("Feeds"));
   news_theme_menu_icon_set(mi, "icon");
   e_menu_item_callback_set(mi, _cb_configure_feeds, NULL);

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, D_("Main"));
   e_util_menu_item_theme_icon_set(mi, "preferences-system");
   e_menu_item_callback_set(mi, _cb_configure_main, NULL);

   mi = e_menu_item_new_relative(ma, NULL);
   e_menu_item_label_set(mi, D_("Settings"));
   e_util_menu_item_theme_icon_set(mi, "preferences-system");
   e_menu_item_submenu_set(mi, mn);

   mi = e_menu_item_new_relative(ma, NULL);
   e_menu_item_label_set(mi, D_("Gadget Contents"));
   news_theme_menu_icon_set(mi, "icon");
   e_menu_item_callback_set(mi, _cb_configure_item_content, ni);

}

static void
_cb_configure_feed(void *data, E_Menu *m, E_Menu_Item *mi)
{
   News_Feed *f;

   f = data;
   if (!f) return;
   if (f->config_dialog) return;

   news_config_dialog_feed_show(f);
}

static void
_cb_configure_item(void *data, E_Menu *m, E_Menu_Item *mi)
{
   News_Item *ni;

   ni = data;
   if (!ni) return;
   if (ni->config_dialog) return;

   news_config_dialog_item_show(ni);
}

static void
_cb_configure_item_content(void *data, E_Menu *m, E_Menu_Item *mi)
{
   News_Item *ni;

   ni = data;
   if (!ni) return;
   if (ni->config_dialog_content) return;

   news_config_dialog_item_content_show(ni);
}

static void
_cb_configure_main(void *data, E_Menu *m, E_Menu_Item *mi)
{
   if (!news) return;

   news_config_dialog_show();
}

static void
_cb_configure_feeds(void *data, E_Menu *m, E_Menu_Item *mi)
{
   if (!news) return;

   news_config_dialog_feeds_show();
}

static void
_cb_item_deactivate_post(void *data, E_Menu *m)
{
   News_Item *ni;

   ni = data;
   if (!ni) return;
   if (!ni->menu) return;

   e_object_del(E_OBJECT(ni->menu));
   ni->menu = NULL;
}

static void
_cb_item_setasread(void *data, E_Menu *m, E_Menu_Item *mi)
{
   News_Item *ni;

   ni = data;

   NEWS_ITEM_FEEDS_FOREACH_BEG(ni);
   news_feed_read_set(_feed);
   NEWS_ITEM_FEEDS_FOREACH_END();
}

static void
_cb_item_update(void *data, E_Menu *m, E_Menu_Item *mi)
{
   News_Item *ni;

   ni = data;
   NEWS_ITEM_FEEDS_FOREACH_BEG(ni);
   news_feed_update(_feed);
   NEWS_ITEM_FEEDS_FOREACH_END();
}

static void
_cb_feed_deactivate_post(void *data, E_Menu *m)
{
   News_Feed *f;

   f = data;
   if (!f) return;
   if (!f->menu) return;

   e_object_del(E_OBJECT(f->menu));
   f->menu = NULL;
}

static void
_cb_browser_deactivate_post(void *data, E_Menu *m)
{
   News_Item *ni;

   ni = data;
   if (!ni) return;
   if (!ni->menu_browser) return;

   e_object_del(E_OBJECT(ni->menu_browser));
   ni->menu_browser = NULL;
}

static void
_cb_browser_feed(void *data, E_Menu *m, E_Menu_Item *mi)
{
   News_Feed *feed;
   News_Item *ni;

   feed = data;
   ni = feed->item;
   if (!ni) return;

   if (ni->config->browser_open_home)
     news_util_browser_open(feed->url_home);
   else
     news_util_browser_open(feed->url_feed);
}

