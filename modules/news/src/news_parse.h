#ifdef E_MOD_NEWS_TYPEDEFS

typedef struct _News_Parse News_Parse;

typedef enum _News_Parse_Error
   {
      NEWS_PARSE_ERROR_NO,
      NEWS_PARSE_ERROR_BROKEN_FEED,
      NEWS_PARSE_ERROR_TYPE_UNKNOWN,
      NEWS_PARSE_ERROR_NOT_IMPLEMENTED
   } News_Parse_Error;

#else

#ifndef NEWS_PARSE_H_INCLUDED
#define NEWS_PARSE_H_INCLUDED

int  news_parse_init(void);
void news_parse_shutdown(void);

void news_parse_go(News_Feed_Document *doc,
                   void (cb_func) (News_Feed_Document *doc, News_Parse_Error error, int changes));
void news_parse_stop(News_Feed_Document *doc);

#endif
#endif
