#ifdef E_MOD_NEWS_TYPEDEFS



#else

#ifndef NEWS_MENU_H_INCLUDED
#define NEWS_MENU_H_INCLUDED

int  news_menu_item_show(News_Item *ni);
void news_menu_item_hide(News_Item *ni);

int  news_menu_feed_show(News_Feed *f);
void news_menu_feed_hide(News_Feed *f);

int  news_menu_browser_show(News_Item *ni);
void news_menu_browser_hide(News_Item *ni);

#endif
#endif
