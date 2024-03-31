#ifdef E_MOD_NEWS_TYPEDEFS

typedef struct _News_Feed News_Feed;
typedef struct _News_Feed_Ref News_Feed_Ref;
typedef struct _News_Feed_Category News_Feed_Category;
typedef struct _News_Feed_Document News_Feed_Document;
typedef struct _News_Feed_Article News_Feed_Article;

typedef struct _News_Feed_Lang News_Feed_Lang;

typedef enum _News_Feed_Type
   {
      NEWS_FEED_TYPE_UNKNOWN,
      NEWS_FEED_TYPE_RSS,
      NEWS_FEED_TYPE_ATOM
   } News_Feed_Type;

typedef enum _News_Feed_Popup_News_Active
   {
      NEWS_FEED_POPUP_NEWS_ACTIVE_NO,
      NEWS_FEED_POPUP_NEWS_ACTIVE_URGENT,
      NEWS_FEED_POPUP_NEWS_ACTIVE_ALL
   } News_Feed_Popup_News_Active;

#else

#ifndef NEWS_FEED_H_INCLUDED
#define NEWS_FEED_H_INCLUDED

#define NEWS_FEED_TIMER_DEFAULT 20
#define NEWS_FEED_TIMER_MIN 1
#define NEWS_FEED_TIMER_MAX 60

#define NEWS_FEED_SORT_NAME_DEFAULT 0

#define NEWS_FEED_LANG_ALL_DEFAULT 0

#define NEWS_FEED_NB_TRIES_MAX 30

#define NEWS_FEED_PROXY_PORT_DEFAULT 8080

#define NEWS_FEED_UNREAD_COUNT_MAX 1000

#define NEWS_FEED_POPUP_NEWS_ACTIVE_DEFAULT NEWS_FEED_POPUP_NEWS_ACTIVE_ALL
#define NEWS_FEED_POPUP_NEWS_TIMER_S_DEFAULT 8

#define NEWS_FEED_POPUP_OTHER_ON_TIMEOUT_DEFAULT 1
#define NEWS_FEED_POPUP_OTHER_TIMER_S_DEFAULT 3

#define NEWS_FEED_NAME_OVRW_DEFAULT 0
#define NEWS_FEED_LANGUAGE_OVRW_DEFAULT 1
#define NEWS_FEED_DESCRIPTION_OVRW_DEFAULT 1
#define NEWS_FEED_URL_HOME_OVRW_DEFAULT 1
#define NEWS_FEED_ICON_OVRW_DEFAULT 1

#define NEWS_FEED_ITEM_CAT_COMICS "Comics"
#define NEWS_FEED_ITEM_CAT_COMPUTERS "Computers"
#define NEWS_FEED_ITEM_CAT_ENLIGHTENMENT "Enlightenment"
#define NEWS_FEED_ITEM_CAT_NEWS "News"
#define NEWS_FEED_ITEM_CAT_SCIENCES "Science"
#define NEWS_FEED_ITEM_CAT_TEST "Test"
#define NEWS_FEED_ITEM_CAT_VARIOUS "Various"
#define NEWS_FEED_ITEM_CAT_ICON_DEFAULT "/images/categories/default.png"
#define NEWS_FEED_ITEM_CAT_ICON_COMICS NEWS_FEED_ITEM_CAT_ICON_DEFAULT
#define NEWS_FEED_ITEM_CAT_ICON_COMPUTERS NEWS_FEED_ITEM_CAT_ICON_DEFAULT
#define NEWS_FEED_ITEM_CAT_ICON_ENLIGHTENMENT NEWS_FEED_ITEM_CAT_ICON_DEFAULT
#define NEWS_FEED_ITEM_CAT_ICON_NEWS NEWS_FEED_ITEM_CAT_ICON_DEFAULT
#define NEWS_FEED_ITEM_CAT_ICON_SCIENCES NEWS_FEED_ITEM_CAT_ICON_DEFAULT
#define NEWS_FEED_ITEM_CAT_ICON_TEST NEWS_FEED_ITEM_CAT_ICON_DEFAULT
#define NEWS_FEED_ITEM_CAT_ICON_VARIOUS NEWS_FEED_ITEM_CAT_ICON_DEFAULT

#define NEWS_FEED_ITEM_FEED_ \
"", 1, "fr", 1, "", 1, \
"", 1, \
""
#define NEWS_FEED_ITEM_FEED_AFP_EN \
"Afp", 1, "en", 1, "The News / wire stories", 1, \
"http://www.afp.com/english/", 1, \
"http://www.afp.com/english/rss/stories.xml"
#define NEWS_FEED_ITEM_FEED_BARRAPUNTO \
"Barrapunto", 1, "es", 1, "La información que te interesa", 1, \
"http://barrapunto.com", 1, \
"http://barrapunto.com/barrapunto.rss"
#define NEWS_FEED_ITEM_FEED_BBC_WORLD \
"BBC world", 1, "en", 1, "Visit BBC News for up-to-the-minute news, breaking news, video, audio and feature stories. BBC News provides trusted World and UK news as well as local and regional perspectives. Also entertainment, business, science, technology and health news.", 1, \
"http://www.bbc.co.uk", 1, \
"http://newsrss.bbc.co.uk/rss/newsonline_uk_edition/world/rss.xml"
#define NEWS_FEED_ITEM_FEED_BOULETCORP \
"bouletcorp - le blog", 0, "fr", 1, "Le blog de boulet", 0, \
"http://www.bouletcorp.com/blog/", 1, \
"http://www.bouletcorp.com/blog/fill_rss.php5"
#define NEWS_FEED_ITEM_FEED_CNN \
"Cnn", 1, "en", 1, "CNN.com delivers up-to-the-minute news and information on the latest top stories, weather, entertainment, politics and more.", 1, \
"http://www.cnn.com", 1, \
"http://rss.cnn.com/rss/cnn_topstories.rss"
#define NEWS_FEED_ITEM_FEED_COURRIERINTERNATIONAL \
"Courrierinternational", 1, "fr", 1, "Courrier international : hebdomadaire français d'information internationale. Sur Internet, les journalistes de Courrier international réalisent une revue de presse quotidienne.", 1, \
"http://www.courrierinternational.com", 1, \
"http://www.courrierinternational.com/rss/rss_a_la_une.xml"
#define NEWS_FEED_ITEM_FEED_DIGG \
"digg", 1, "en", 1, "digg", 1, \
"http://www.digg.com", 1, \
"http://digg.com/rss/index.xml"
#define NEWS_FEED_ITEM_FEED_E_CVS \
"E Cvs", 1, "en", 1, "Enlightenment is a completely themeable, highly configurable Window Manager for the X Window System, traditionally used in Unix environments.", 1, \
"http://www.enlightenment.org", 1, \
"http://cia.navi.cx/stats/project/e/.rss?ver=2&amp;medium=plaintext&amp;limit="
#define NEWS_FEED_ITEM_FEED_E_FR \
"Enlightenment-fr", 1, "fr", 1, "", 1, \
"http://fr.enlightenment.org/", 1, \
"http://fr.enlightenment.org/feed/"
#define NEWS_FEED_ITEM_FEED_FRESHMEAT \
"Freshmeat", 1, "en", 1, "The last 24 hours worth of freshmeat.net releases", 1, \
"http://www.freshmeat.net", 1, \
"http://rss.freshmeat.net/freshmeat/feeds/fm-releases-global"
#define NEWS_FEED_ITEM_FEED_FUTURASCIENCES \
"Futura-Sciences", 0, "fr", 1, "L'actualité quotidienne des sciences et technologies sur Futura-Sciences !", 1, \
"http://www.futura-sciences.com/", 1, \
"http://feeds.feedburner.com/packfs"
#define NEWS_FEED_ITEM_FEED_GOOGLE \
"Google feed", 1, "en", 1, "Google News", 1, \
"http://news.google.com", 1, \
"http://news.google.com/?output=rss"
#define NEWS_FEED_ITEM_FEED_GOOGLE_FR \
"Google Fr feed", 1, "fr", 1, "Google News France", 1, \
"http://news.google.fr", 1, \
"http://news.google.fr/?output=rss"
#define NEWS_FEED_ITEM_FEED_LEMONDE_UNE \
"Le monde (la une)", 1, "fr", 1, "Toute l'actualité au moment de la connexion", 1, \
"http://www.lemonde.fr", 1, \
"http://www.lemonde.fr/rss/sequence/0,2-3208,1-0,0.xml"
#define NEWS_FEED_ITEM_FEED_MENEAME \
"Menéame: publicadas", 1, "es", 1, "Sitio colaborativo de publicación y comunicación entre blogs", 1, \
"http://meneame.net/", 1, \
"http://feeds.feedburner.com/meneame/published"
#define NEWS_FEED_ITEM_FEED_NEWS \
"News module svn", 1, "en", 1, "Enlightenment News module developement feed, to follow subversion commits (svn://oooo.zapto.org/News)", 1, \
"http://oooo.zapto.org?page=projets/news", 1, \
"http://oooo.zapto.org/projets/news/rss/"
#define NEWS_FEED_ITEM_FEED_OSNEWS \
"Osnews", 1, "en", 1, "Exploring the Future of Computing", 1, \
"http://www.osnews.com", 1, \
"http://www.osnews.com/files/recent.rdf"
#define NEWS_FEED_ITEM_FEED_SLASHDOT \
"Slashdot", 1, "en", 1, "News for nerds, stuff that matters", 1, \
"http://www.slashdot.org", 1, \
"http://rss.slashdot.org/Slashdot/slashdot"
#define NEWS_FEED_ITEM_FEED_TEST \
"Test feed", 1, "en", 1, "", 1, \
"http://oooo.zapto.org", 1, \
"http://oooo.zapto.org/test.rss"
#define NEWS_FEED_ITEM_FEED_ICON_DEFAULT "/images/feeds/default.png"
#define NEWS_FEED_ITEM_FEED_ICON_AFP_EN "images/feeds/afp.png"
#define NEWS_FEED_ITEM_FEED_ICON_BARRAPUNTO "images/feeds/barrapunto.png"
#define NEWS_FEED_ITEM_FEED_ICON_BBC_WORLD "images/feeds/bbcworld.png"
#define NEWS_FEED_ITEM_FEED_ICON_BOULETCORP "images/feeds/bouletcorp.png"
#define NEWS_FEED_ITEM_FEED_ICON_CNN "images/feeds/cnn.png"
#define NEWS_FEED_ITEM_FEED_ICON_COURRIERINTERNATIONAL "images/feeds/courrierinternational.png"
#define NEWS_FEED_ITEM_FEED_ICON_DIGG "images/feeds/digg.png"
#define NEWS_FEED_ITEM_FEED_ICON_E_CVS "images/feeds/e_cvs.png"
#define NEWS_FEED_ITEM_FEED_ICON_E_FR "images/feeds/e_fr.png"
#define NEWS_FEED_ITEM_FEED_ICON_FRESHMEAT "images/feeds/freshmeat.png"
#define NEWS_FEED_ITEM_FEED_ICON_FUTURASCIENCES "images/feeds/futurasciences.png"
#define NEWS_FEED_ITEM_FEED_ICON_GOOGLE "images/feeds/google.png"
#define NEWS_FEED_ITEM_FEED_ICON_GOOGLE_FR "images/feeds/google_fr.png"
#define NEWS_FEED_ITEM_FEED_ICON_LEMONDE "images/feeds/lemonde.png"
#define NEWS_FEED_ITEM_FEED_ICON_MENEAME "images/feeds/meneame.png"
#define NEWS_FEED_ITEM_FEED_ICON_NEWS "images/feeds/news.png"
#define NEWS_FEED_ITEM_FEED_ICON_OSNEWS "images/feeds/osnews.png"
#define NEWS_FEED_ITEM_FEED_ICON_SLASHDOT "images/feeds/slashdot.png"
#define NEWS_FEED_ITEM_FEED_ICON_TEST NEWS_FEED_ITEM_FEED_ICON_DEFAULT

#define NEWS_FEED_FOREACH_BEG() \
{ \
Eina_List *_l_feeds, *_l_cats; \
News_Feed_Category *_cat; \
News_Feed *_feed; \
\
for (_l_feeds=news->config->feed.categories; _l_feeds; _l_feeds=eina_list_next(_l_feeds)) \
{ \
   _cat = _l_feeds->data; \
   for (_l_cats=_cat->feeds; _l_cats; _l_cats=eina_list_next(_l_cats)) \
     { \
        _feed = _l_cats->data;
#define NEWS_FEED_FOREACH_END() \
     } \
} \
}

struct _News_Feed
{
   /* not saved */
   News_Item *item;
   News_Feed_Category *category;

   /* saved */
   const char *name;
   int         name_ovrw;
   const char *language;
   int         language_ovrw;
   const char *description;
   int         description_ovrw;
   const char *url_home;
   int         url_home_ovrw;
   const char *url_feed;
   const char *icon;
   int         icon_ovrw;
   int         important;

   /* not saved */
   E_Config_Dialog *config_dialog;
   E_Menu *menu;

   News_Feed_Document *doc;

   Evas_Object *obj;
   Evas_Object *obj_icon;

   char *host;
   char *file;
};

struct _News_Feed_Ref
{
   /* saved */
   const char *category;
   const char *name;

   /* not saved */
   News_Feed *feed;
};

struct _News_Feed_Category
{
   /* saved */
   const char *name;
   const char *icon;

   Eina_List *feeds;

   /* not saved */
   /* temporary list of categories */
   /* this list is sorted and contains only the feeds
      of the languages selected by user
      it regenerated via news_feed_lists_refresh */
   Eina_List *feeds_visible;
   int        feeds_visible_free;
   E_Config_Dialog *config_dialog;
};

struct _News_Feed_Document
{
   News_Feed       *feed;

   News_Popup   *popw;
   Eina_List    *articles;
   int           unread_count;
   unsigned char ui_needrefresh : 1;

   struct
   {
      Ecore_Con_Server *conn;
      Ecore_Event_Handler *handler_add;
      Ecore_Event_Handler *handler_del;
      Ecore_Event_Handler *handler_data;

      unsigned char waiting_reply : 1;
      int           nb_tries;
      char         *buffer;
      int           buffer_size;
   } server;

   struct
   {
      News_Feed_Type type;
      float          version;
      const char    *meta_article;
      const char    *meta_channel;
      const char    *meta_date;
      const char    *charset;

      News_Parse      *parser;
      unsigned char    got_infos : 1;
      time_t           last_time;
      News_Parse_Error error;
   } parse;
};

struct _News_Feed_Article
{
   News_Feed_Document  *doc;

   unsigned char unread : 1;
   unsigned char reused : 1;

   char        *title;
   char        *url;
   char        *description;
   struct tm    date;
   Evas_Object *image;
};

struct _News_Feed_Lang
{
   const char *key;
   const char *name;
};

int                 news_feed_init(void);
void                news_feed_shutdown(void);
void                news_feed_all_delete(void);
void                news_feed_all_restore(void);
void                news_feed_lists_refresh(int sort);
void                news_feed_timer_set(int time);

News_Feed          *news_feed_new(char *name, int name_ovrw,
                                  char *language, int language_ovrw,
                                  char *description, int description_ovrw,
                                  char *url_home, int url_home_ovrw,
                                  char *url_feed,
                                  char *icon, int icon_ovrw,
                                  int important,
                                  News_Feed_Category *category);
int                 news_feed_edit(News_Feed *f,
                                   char *name, int name_ovrw,
                                   char *language, int language_ovrw,
                                   char *description, int description_ovrw,
                                   char *url_home, int url_home_ovrw,
                                   char *url_feed,
                                   char *icon, int icon_ovrw,
                                   int important,
                                   News_Feed_Category *category,
                                   int check_only);
void                news_feed_free(News_Feed *f);

int                 news_feed_attach(News_Feed *f, News_Feed_Ref *ref, News_Item *ni);
void                news_feed_detach(News_Feed *f, int really);
int                 news_feed_update(News_Feed *f);
void                news_feed_obj_refresh(News_Feed *f, int changed_content, int changed_state);
News_Feed_Ref      *news_feed_ref_find(News_Feed *f, News_Item *ni);
void                news_feed_read_set(News_Feed *feed);
void                news_feed_unread_count_change(News_Feed *feed, int nb);
void                news_feed_list_ui_refresh(void);

void                news_feed_article_del(News_Feed_Article *art);
void                news_feed_article_unread_set(News_Feed_Article *art, int unread);

News_Feed_Category *news_feed_category_new(char *name, char *icon);
int                 news_feed_category_edit(News_Feed_Category *cat, char *name, char *icon);
void                news_feed_category_free(News_Feed_Category *cat);
void                news_feed_category_list_ui_refresh(void);

void                news_feed_lang_list_refresh(void);
void                news_feed_lang_list_free(Eina_List *l);
const char         *news_feed_lang_name_get(const char *key);
int                 news_feed_lang_selected_is(const char *key);

#endif
#endif
