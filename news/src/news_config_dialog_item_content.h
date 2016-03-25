#ifdef E_MOD_NEWS_TYPEDEFS



#else

#ifndef NEWS_CONFIG_DIALOG_ITEM_CONTENT_H_INCLUDED
#define NEWS_CONFIG_DIALOG_ITEM_CONTENT_H_INCLUDED

int  news_config_dialog_item_content_show(News_Item *ni);
void news_config_dialog_item_content_hide(News_Item *ni);

void news_config_dialog_item_content_refresh_feeds(News_Item *ni);
void news_config_dialog_item_content_refresh_selected_feeds(News_Item *ni);

#endif
#endif
