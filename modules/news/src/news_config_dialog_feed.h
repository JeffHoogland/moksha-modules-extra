#ifdef E_MOD_NEWS_TYPEDEFS



#else

#ifndef NEWS_CONFIG_DIALOG_FEED_H_INCLUDED
#define NEWS_CONFIG_DIALOG_FEED_H_INCLUDED

int  news_config_dialog_feed_show(News_Feed *feed);
void news_config_dialog_feed_hide(News_Feed *feed);

void news_config_dialog_feed_refresh_categories(News_Feed *feed);
void news_config_dialog_feed_refresh_langs(News_Feed *feed);

#endif
#endif
