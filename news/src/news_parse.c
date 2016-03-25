#include "News.h"

typedef enum News_Parse_Oc
   {
      NEWS_PARSE_OC_DETECT_METAS,
      NEWS_PARSE_OC_INFOS,
      NEWS_PARSE_OC_ARTICLE_BEG,
      NEWS_PARSE_OC_ITEM_TITLE,
      NEWS_PARSE_OC_ITEM_TITLE_CLEAN,
      NEWS_PARSE_OC_ITEM_DATE,
      NEWS_PARSE_OC_ITEM_IMAGE,
      NEWS_PARSE_OC_ITEM_LINK,
      NEWS_PARSE_OC_ITEM_LINK_CLEAN,
      NEWS_PARSE_OC_ARTICLE_EXISTS,
      NEWS_PARSE_OC_ITEM_DESCRIPTION,
      NEWS_PARSE_OC_ITEM_DESCRIPTION_CLEAN,
      NEWS_PARSE_OC_ARTICLE_END,
      NEWS_PARSE_OC_END
   } News_Parse_Oc;

typedef struct _News_Parse_Oc_Actions News_Parse_Oc_Actions;
typedef struct _News_Parse_Article News_Parse_Article;

struct _News_Parse_Oc_Actions
{
   News_Parse_Oc action;
   News_Parse_Oc after;
};

struct _News_Parse_Article
{
   News_Feed_Article *article;

   char      *pos_end;

   char      *title;
   char      *url;
   char      *image;
   char      *description;
   struct tm  date;
};

struct _News_Parse
{
   News_Feed_Document *doc;
   void (*cb_func) (News_Feed_Document *doc, News_Parse_Error error, int changes);
   char *buffer_pos;

   News_Parse_Oc_Actions oc;
   Eina_List            *articles;
   Ecore_Idler          *idler;
   News_Parse_Article   *art;
   int                   error;
   unsigned char         changes : 1;
};

static Eina_List *_parsers;

static int              _parse_type(News_Feed_Document *doc);

static Eina_Bool              _idler(void *data);
static News_Parse_Error _idler_parse_detect_metas(News_Parse *parser);
static News_Parse_Error _idler_parse_infos(News_Parse *parser);
static News_Parse_Error _idler_parse_article_init(News_Parse *parser);
static News_Parse_Error _idler_parse_article_exists(News_Parse *parser);
static News_Parse_Error _idler_parse_article_end(News_Parse *parser);
static News_Parse_Error _idler_parse_item_title(News_Parse *parser);
static News_Parse_Error _idler_parse_item_date(News_Parse *parser);
static News_Parse_Error _idler_parse_item_image(News_Parse *parser);
static News_Parse_Error _idler_parse_item_link(News_Parse *parser);
static News_Parse_Error _idler_parse_item_description(News_Parse *parser);
static News_Parse_Error _idler_parse_item_clean(News_Parse *parser);

static char            *_meta_block_find(char **buf);
static char            *_meta_block_end(char *buf, int beg_before, int end_before);

static const char      *_parse_infos_item(News_Parse *parser, char *meta, const char *prev, int *edit);

static void             _parse_finished(News_Parse *parser);
static void             _update_doc(News_Parse *parser);
static void             _parse_free(News_Parse *parser);
static void             _parse_article_free(News_Parse_Article *a);

/*
 * Public functions
 */

int
news_parse_init(void)
{
   _parsers = NULL;

   return 1;
}

void
news_parse_shutdown(void)
{
   News_Parse *p;

   while (_parsers)
     {
        p = _parsers->data;
        news_parse_stop(p->doc);
        _parsers = eina_list_remove_list(_parsers, _parsers);
     }
}

void
news_parse_go(News_Feed_Document *doc,
              void (*cb_func) (News_Feed_Document *doc, News_Parse_Error error, int changes))
{
   News_Parse *parser;

   if (doc->parse.parser) return;

   if (!_parse_type(doc))
     {
        if (doc->articles && eina_list_count(doc->articles))
          cb_func(doc, NEWS_PARSE_ERROR_TYPE_UNKNOWN, 0);
        else
          cb_func(doc, NEWS_PARSE_ERROR_TYPE_UNKNOWN, 1);
        return;
     }

   parser = E_NEW(News_Parse, 1);
   parser->doc = doc;
   parser->cb_func = cb_func;
   parser->buffer_pos = parser->doc->server.buffer;

   doc->parse.parser = parser;
   _parsers = eina_list_append(_parsers, parser);

   switch (doc->parse.type)
     {
     case NEWS_FEED_TYPE_RSS:
        parser->oc.action = NEWS_PARSE_OC_DETECT_METAS;
        parser->idler = ecore_idler_add(_idler, parser);
        return;
     case NEWS_FEED_TYPE_ATOM:
        parser->error = NEWS_PARSE_ERROR_NOT_IMPLEMENTED;
        _parse_finished(parser);
        return;
     case NEWS_FEED_TYPE_UNKNOWN:
        return;
     }
}

void
news_parse_stop(News_Feed_Document *doc)
{
   if (!doc->parse.parser) return;

   DPARSE(("Parse STOP"));

   _parse_free(doc->parse.parser);
   _parsers = eina_list_remove(_parsers,
                               doc->parse.parser);
   doc->parse.parser = NULL;
}

/*
 * Private functions
 */

static int
_parse_type(News_Feed_Document *doc)
{
   char *buf, *p;

   buf = doc->server.buffer;

   p = strstr(buf, "<rss");
   if (p)
     {
        p = strstr(p, "version");
        if (!p) return 0;
        doc->parse.type = NEWS_FEED_TYPE_RSS;
        sscanf(p + 9, "%f", &doc->parse.version);
        return 1;
     }

   p = strstr(buf, "<rdf:RDF");
   if (p)
     {
        doc->parse.type = NEWS_FEED_TYPE_RSS;
        doc->parse.version = 1.0;
        return 1;
     }

   p = strstr(buf, "<feed xmlns");
   if (p)
     {
        doc->parse.type = NEWS_FEED_TYPE_ATOM;
        doc->parse.version = 1.0;
        return 1;
     }

   doc->parse.type = NEWS_FEED_TYPE_UNKNOWN;
   doc->parse.version = 0.0;

   return 0;
}

static Eina_Bool
_idler(void *data)
{
   News_Parse *parser;
   News_Parse_Error err;

   parser = data;

   //DD(("%d", parser->oc.action));

   switch(parser->oc.action)
     {
     case NEWS_PARSE_OC_DETECT_METAS:
        err = _idler_parse_detect_metas(parser);
        parser->oc.action = NEWS_PARSE_OC_INFOS;
        break;
     case NEWS_PARSE_OC_INFOS:
        err = _idler_parse_infos(parser);
        parser->oc.action = NEWS_PARSE_OC_ARTICLE_BEG;
        break;
     case NEWS_PARSE_OC_ARTICLE_BEG:
        err = _idler_parse_article_init(parser);
        if (parser->oc.action == NEWS_PARSE_OC_END) break;
        parser->oc.action = NEWS_PARSE_OC_ITEM_TITLE;
        break;
     case NEWS_PARSE_OC_ARTICLE_END:
        err = _idler_parse_article_end(parser);
        parser->oc.action = NEWS_PARSE_OC_ARTICLE_BEG;
        break;
     case NEWS_PARSE_OC_ITEM_TITLE:
        err = _idler_parse_item_title(parser);
        parser->oc.action = NEWS_PARSE_OC_ITEM_TITLE_CLEAN;
        parser->oc.after = NEWS_PARSE_OC_ITEM_DATE;
        break;
     case NEWS_PARSE_OC_ITEM_DATE:
        err = _idler_parse_item_date(parser);
        parser->oc.action = NEWS_PARSE_OC_ITEM_LINK;
        break;
     case NEWS_PARSE_OC_ITEM_LINK:
        err = _idler_parse_item_link(parser);
        parser->oc.action = NEWS_PARSE_OC_ITEM_LINK_CLEAN;
        parser->oc.after = NEWS_PARSE_OC_ARTICLE_EXISTS;
        break;
     case NEWS_PARSE_OC_ARTICLE_EXISTS:
        err = _idler_parse_article_exists(parser);
        if (parser->oc.action == NEWS_PARSE_OC_ARTICLE_END) break;
        parser->oc.action = NEWS_PARSE_OC_ITEM_IMAGE;
        break;
     case NEWS_PARSE_OC_ITEM_IMAGE:
        err = _idler_parse_item_image(parser);
        parser->oc.action = NEWS_PARSE_OC_ITEM_DESCRIPTION;
        break;
     case NEWS_PARSE_OC_ITEM_DESCRIPTION:
        err = _idler_parse_item_description(parser);
        parser->oc.action = NEWS_PARSE_OC_ITEM_DESCRIPTION_CLEAN;
        parser->oc.after = NEWS_PARSE_OC_ARTICLE_END;
        break;
     case NEWS_PARSE_OC_ITEM_TITLE_CLEAN:
     case NEWS_PARSE_OC_ITEM_LINK_CLEAN:
     case NEWS_PARSE_OC_ITEM_DESCRIPTION_CLEAN:
        err = _idler_parse_item_clean(parser);
        parser->oc.action = parser->oc.after;
        break;
     case NEWS_PARSE_OC_END:
        _parse_finished(parser);
        return EINA_FALSE;
     }

   /* error returned */
   if (err != NEWS_PARSE_ERROR_NO)
     {
        parser->error = err;
        parser->oc.action = NEWS_PARSE_OC_END;
     }

   return EINA_TRUE;
}

static News_Parse_Error
_idler_parse_detect_metas(News_Parse *parser)
{
   News_Feed *feed;
   News_Feed_Document *doc;
   float ver;

   doc = parser->doc;
   feed = doc->feed;
   ver = doc->parse.version;

   /* already done in a previous update ? */
   if (doc->parse.meta_channel) return NEWS_PARSE_ERROR_NO;
   
   if (ver == 1.0)
     {
        if (strstr(doc->server.buffer, "<item rdf"))
          doc->parse.meta_article = eina_stringshare_add("<item rdf");
        else
          doc->parse.meta_article = eina_stringshare_add("<item>");

        if (strstr(doc->server.buffer, "<channel rdf"))
          doc->parse.meta_channel = eina_stringshare_add("<channel rdf");
        else
          doc->parse.meta_channel = eina_stringshare_add("<channel>");

	doc->parse.meta_date = eina_stringshare_add("<dc:date");
     }
   else if ((ver > 0.0) && (ver <= 2.0))
     {
        doc->parse.meta_article = eina_stringshare_add("<item>");
        doc->parse.meta_channel = eina_stringshare_add("<channel>");
	doc->parse.meta_date = eina_stringshare_add("<pubDate");
     }

   if (!doc->parse.meta_channel)
     return NEWS_PARSE_ERROR_TYPE_UNKNOWN;

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_infos(News_Parse *parser)
{
   News_Feed *feed;
   const char *name, *language, *description, *url_home, *icon;
   int   edit = 0;

   /* already done in a previous update ? */
   if (parser->doc->parse.got_infos) return NEWS_PARSE_ERROR_NO;

   feed = parser->doc->feed;
   name = feed->name;
   language = feed->language;
   description = feed->description;
   url_home = feed->url_home;
   icon = feed->icon;
  
   if (feed->name_ovrw)
     name = _parse_infos_item(parser, "<title", feed->name, &edit);
   if (feed->language_ovrw)
     language = _parse_infos_item(parser, "<language", feed->language, &edit);
   if (feed->description_ovrw)
     description = _parse_infos_item(parser, "<description", feed->description, &edit);
   if (feed->url_home_ovrw)
     url_home = _parse_infos_item(parser, "<link", feed->url_home, &edit);
   //TODO handle feed image from server
   /*if (feed->icon_ovrw)
     icon = _parse_infos_item(parser, "<image", feed->icon, &edit); */

   if (edit)
     {
        news_feed_edit(feed,
                       (char *)name, feed->name_ovrw,
                       (char *)language, feed->language_ovrw,
                       (char *)description, feed->description_ovrw,
                       (char *)url_home, feed->url_home_ovrw,
                       (char *)feed->url_feed,
                       (char *)icon, feed->icon_ovrw,
                       feed->important,
                       feed->category,
                       0);
        if (feed->item && feed->item->viewer)
          news_viewer_refresh(feed->item->viewer);
        news_feed_lists_refresh(1);
     }

   parser->doc->parse.got_infos = 1;

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_article_init(News_Parse *parser)
{
   News_Parse_Article *art;
   char *pos;
   char *p1, *p2;

   pos = parser->buffer_pos;

   if (pos >= (parser->doc->server.buffer + parser->doc->server.buffer_size))
     {
        parser->oc.action = NEWS_PARSE_OC_END;
        return NEWS_PARSE_ERROR_NO;
     }
     
   p1 = strstr(pos, parser->doc->parse.meta_article);
   if (!p1)
     {
        if (eina_list_count(parser->articles))
          {
             parser->oc.action = NEWS_PARSE_OC_END;
             return NEWS_PARSE_ERROR_NO;
          }
        return NEWS_PARSE_ERROR_BROKEN_FEED;
     }
   p2 = _meta_block_end(p1, 1, 1);
   if (!p2) return NEWS_PARSE_ERROR_BROKEN_FEED;

   art = E_NEW(News_Parse_Article, 1);
   art->pos_end = p2;

   parser->art = art;
   parser->buffer_pos = p1;

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_article_exists(News_Parse *parser)
{
   News_Parse_Article *art;
   News_Feed_Article *a;
   Eina_List *l;

   art = parser->art;

   if (!parser->doc->articles ||
       !eina_list_count(parser->doc->articles))
     return NEWS_PARSE_ERROR_NO;

   DPARSE(("ALREADY EXISTS : Go %s %s",
           art->url, art->title));
   for (l=parser->doc->articles; l; l=eina_list_next(l))
     {
        a = l->data;

        if (
            ( !a->reused )
            &&
            ( !news_util_datecmp(&art->date, &a->date) )
            &&
            ( ( (art->title && a->title) &&
                !strcmp(art->title, a->title) ) ||
              (!art->title && !a->title) )
            )
          {
             art->article = a;
             a->reused = 1;
             parser->oc.action = NEWS_PARSE_OC_ARTICLE_END;
             DPARSE(("ALREADY EXISTS : MATCH !!!"));
             return NEWS_PARSE_ERROR_NO;
          }
        DD(("ALREADY EXISTS : NO Match %s %s",
            a->url, a->title));        
     }

   DPARSE(("ALREADY_EXISTS : DOES NOT exists"));

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_article_end(News_Parse *parser)
{
   parser->articles = eina_list_append(parser->articles,
                                       parser->art);
   parser->buffer_pos = parser->art->pos_end;
   DPARSE(("Parse article %s end", parser->art->title));
   parser->art = NULL;

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_item_title(News_Parse *parser)
{
   News_Parse_Article *art;
   char *pos;
   char *p1, *p2;
   int len;

   art = parser->art;
   pos = parser->buffer_pos;

   p1 = strstr(pos, "<title");
   if (p1 && (p1 < art->pos_end))
     {
        p2 = _meta_block_find(&p1);
        if (!p2 || (p2 > art->pos_end))
          return NEWS_PARSE_ERROR_BROKEN_FEED;
        len = p2 - p1;
        art->title = E_NEW(char, len+1);
        memcpy(art->title, p1, len);
        art->title[len] = '\0';
     }
   else
     art->title = strdup("No title");

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_item_date(News_Parse *parser)
{
   News_Parse_Article *art;
   char *pos, *date;
   char *p1, *p2;
   int len, i;
   struct tm *tm;

   art = parser->art;
   pos = parser->buffer_pos;

   p1 = strstr(pos, parser->doc->parse.meta_date);
   if (p1 && (p1 < art->pos_end))
     {
        p2 = _meta_block_find(&p1);
        if (!p2 || p2 > art->pos_end)
          return NEWS_PARSE_ERROR_BROKEN_FEED;

        len = p2 - p1;
        date = p1;
        tm = &art->date;

        if ((parser->doc->parse.version == 1.0) &&
            (len >= 22))
          {
             sscanf(date, "%4d", &i);
             tm->tm_year = i - 1900;
             sscanf(date+5, "%2d", &tm->tm_mon);
             tm->tm_mon--;   /* tm works with 0-11 */
             sscanf(date+8, "%2d", &tm->tm_mday);
             sscanf(date+11, "%2d", &tm->tm_hour);
             sscanf(date+14, "%2d", &tm->tm_min);
             if (date[16] == ':') /* seconds are precised ? */
               sscanf(date+17, "%2d", &tm->tm_sec);
             else
               tm->tm_sec = 0;
          }
        else if (len >= 25)
          {
             sscanf(date+5, "%2d", &tm->tm_mday);
             if (!strncmp(date+8, "Jan", 3)) tm->tm_mon = 0;
             else if (!strncmp(date+8, "Feb", 3)) tm->tm_mon = 1;
             else if (!strncmp(date+8, "Mar", 3)) tm->tm_mon = 2;
             else if (!strncmp(date+8, "Apr", 3)) tm->tm_mon = 3;
             else if (!strncmp(date+8, "May", 3)) tm->tm_mon = 4;
             else if (!strncmp(date+8, "Jun", 3)) tm->tm_mon = 5;
             else if (!strncmp(date+8, "Jul", 3)) tm->tm_mon = 6;
             else if (!strncmp(date+8, "Aug", 3)) tm->tm_mon = 7;
             else if (!strncmp(date+8, "Sep", 3)) tm->tm_mon = 8;
             else if (!strncmp(date+8, "Oct", 3)) tm->tm_mon = 9;
             else if (!strncmp(date+8, "Nov", 3)) tm->tm_mon = 10;
             else if (!strncmp(date+8, "Dec", 3)) tm->tm_mon = 11;
             else tm->tm_mon = 0;
             sscanf(date+12, "%4d", &i);
             tm->tm_year = i - 1900;
             sscanf(date+17, "%2d", &tm->tm_hour);
             sscanf(date+20, "%2d", &tm->tm_min);
             sscanf(date+23, "%2d", &tm->tm_sec);
          }
     }

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_item_image(News_Parse *parser)
{
   // TODO parse and ecore_download

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_item_link(News_Parse *parser)
{
   News_Parse_Article *art;
   char *pos;
   char *p1, *p2;
   int len;

   art = parser->art;
   pos = parser->buffer_pos;

   p1 = strstr(pos, "<link");
   if (p1 && (p1 < art->pos_end))
     {
        p2 = _meta_block_find(&p1);
        if (!p2 || (p2 > art->pos_end))
          return NEWS_PARSE_ERROR_BROKEN_FEED;
        len = p2 - p1;
        art->url = E_NEW(char, len+1);
        memcpy(art->url, p1, len);
        art->url[len] = '\0';
     }

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_item_description(News_Parse *parser)
{
   News_Parse_Article *art;
   char *pos;
   char *p1, *p2;
   int len;

   art = parser->art;
   pos = parser->buffer_pos;

   p1 = strstr(pos, "<description");
   if (p1 && (p1 < art->pos_end))
     {
        p2 = _meta_block_find(&p1);
        if (!p2 || (p2 > art->pos_end))
          return NEWS_PARSE_ERROR_BROKEN_FEED;
        len = p2 - p1;
        art->description = E_NEW(char, len+1);
        memcpy(art->description, p1, len);
        art->description[len] = '\0';
     }

   return NEWS_PARSE_ERROR_NO;
}

static News_Parse_Error
_idler_parse_item_clean(News_Parse *parser)
{
   char *buf, *p;
   char **buf_org;
   int size;
   int new_size; /* future size, to avoid realloc on each memmove */

   switch (parser->oc.action)
     {
     case NEWS_PARSE_OC_ITEM_TITLE_CLEAN:
        buf_org = &parser->art->title;
        break;
     case NEWS_PARSE_OC_ITEM_LINK_CLEAN:
        buf_org = &parser->art->url;
        break;
     case NEWS_PARSE_OC_ITEM_DESCRIPTION_CLEAN:
        buf_org = &parser->art->description;
        break;
     default:
        return NEWS_PARSE_ERROR_NO;
     }

   buf = *buf_org;
   if (!buf) return NEWS_PARSE_ERROR_NO; /* nothing to clean, ok */
   p = buf;
   size = strlen(buf) + 1;
   new_size = size;

   //FIXME use macros for nicer code
   while (*p)
     {
        //DPARSE(("%d %d ------\n%30.30s", size, new_size, p));
        if (*p == '<') /* meta open, we are going to skip metas inside text */
          {
             char *p2;

             /* skip cfdata */
             if (*(p + 1) == '!')
               {
                  if (!strncmp(p, "<![CDATA[", 9))
                    {
                       memmove(p, p + 9, (buf + new_size) - (p + 9));
                       new_size -= 9;
                    }
                  else
                    p++; /* next char */
               }
             else
               {
                  /* skip normal meta */
                  p2 = p;
                  do
                    {
                       p2++;
                       /* check &gt (>) */
                       if (*p2 == '&')
                         {
                            if (!strncmp(p2 + 1, "gt;", 3))
                              {
                                 p2 += 3;
                                 break;
                              }
                         }
                    }
                  while (*p2 && (*p2 != '>'));
                  if (*p2)
                    {
                       memmove(p, p2 + 1, (buf + new_size) - (p2 + 1));
                       new_size -= ((p2 + 1) - p);
                    }
                  else
                    {
                       *p = '\0';
                       new_size -= (p2 - p);
                    }
               }
          }
        else if (*p == ']') /* skip end cfdata */
          {
             if (!strncmp(p + 1, "]>", 2))
               {
                  memmove(p, p + 3, (buf + new_size) - (p + 3));
                  new_size -= 3;
               }
             else
               p++; /* next char */
          }
        else if (*p == 0xa) /* \n */
          {
             if (parser->oc.action == NEWS_PARSE_OC_ITEM_TITLE_CLEAN)
               {
                  /* remove \n when in title */
                  memmove(p, p + 1, (buf + new_size) - (p + 1));
                  new_size -= 1;
               }
             else
               {
                  new_size += 3;
                  if (new_size > size)
                    {
                       int diff;
                       
                       diff = p - buf;
                       size = new_size;
                       buf = realloc(buf, size);
                       p = buf + diff;
                    }
                  DPARSE(("buf %p p %p new_size %d strlen(buf) %d strlen(p) %d\np :\n%30.30s\n", buf, p, new_size, (int)strlen(buf), (int)strlen(p), p));
                  memmove(p + 3, p, (buf + (new_size - 3)) - (p));
                  *p = '<';
                  *(p + 1) = 'b';
                  *(p + 2) = 'r';
                  *(p + 3) = '>';
                  p = p + 4;
               }
          }
        else if (*p == 0x9) /* \t */
          {
             if (parser->oc.action == NEWS_PARSE_OC_ITEM_TITLE_CLEAN)
               {
                  /* remove \t when in title */
                  memmove(p, p + 1, (buf + new_size) - (p + 1));
                  new_size -= 1;
               }
             else
               {
                  new_size += 4;
                  if (new_size > size)
                    {
                       int diff;
                       
                       diff = p - buf;
                       size = new_size;
                       buf = realloc(buf, size);
                       p = buf + diff;
                    }
                  memmove(p + 4, p, (buf + (new_size - 4)) - (p));
                  *p = '<';
                  *(p + 1) = 't';
                  *(p + 2) = 'a';
                  *(p + 3) = 'b';
                  *(p + 4) = '>';
                  p = p + 5;
               }
          }
        else if (*p == 0xd) /* \<???> (newline) to nothing */
          {
             memmove(p, p + 1, (buf + new_size) - (p + 1));
             new_size -= 1;
          }
        else if (*p == '&') /* html codes */
          {
             char *p_inc;

             p_inc = p + 1;
             if (*p_inc == '#') /* html numbers = ascii hexadecimal char */
               {
                  char b[4];
                  unsigned int i;
                  int len;
                  char *p2 = p + 2;
                  
                  while (*p2 && (*p2 != ';'))
                    p2++;
                  p2++;
                  len = p2 - p;
                  if ((len <= 3) || (len > 7) || !sscanf(p + 2, "%d", &i))
                    p = p_inc; /* next char (invalid html char number) */
                  else if ((i == 8216) || (i == 8217)) /* html num ' */
                    {
                       memmove(p_inc, p + 7, (buf + new_size) - (p + 7));
                       new_size -= 6;
                       *p = '\'';
                       p = p_inc; /* next char */
                    }
                  else if ((i == 171) || (i == 187)) /* html num " */
                    {
                       memmove(p_inc, p + 6, (buf + new_size) - (p + 6));
                       new_size -= 5;
                       *p = '"';
                       p = p_inc; /* next char */
                    }
                  else /* html code convertion dec->hex */
                    {  
                       snprintf(b, sizeof(b), "%c", i);
                       memmove(p_inc, p2, (buf + new_size) - (p2));
                       new_size -= (p2 - p - 1);
                       *p = b[0];
                       /* stay on the same char, could be the start of meta/html char */
                    }
               }
             else if (!strncmp(p_inc, "lt;", 3)) /* html < */
               {
                  memmove(p_inc, p + 4, (buf + new_size) - (p + 4));
                  new_size -= 3;
                  *p = '<';
                  /* stay on the same char, could be start of a meta */
               } /* &gt is checked when skipping meta */
             else if ( !strncmp(p_inc, "amp;", 4) && /* html & */
                       ( (parser->oc.action != NEWS_PARSE_OC_ITEM_DESCRIPTION_CLEAN) ||
                         (*(p_inc+4) == '#') ) ) /* but if we are going to write on a tb, we must not convert, except if the next char is an html num char (cause we need to parse it next) */
               //FIXME thats a quick fix, must fix evas_object_textblock so it supports & alone
               {
                  memmove(p_inc, p + 5, (buf + new_size) - (p + 5));
                  new_size -= 4;
                  *p = '&';
                  /* stay on the same char, could be start of an html char */
               }
             else if (!strncmp(p_inc, "quot;", 5)) /* html " */
               {
                  memmove(p_inc, p + 6, (buf + new_size) - (p + 6));
                  new_size -= 5;
                  *p = '"';
                  p = p_inc; /* next char */
               }
             else if (!strncmp(p_inc, "apos;", 5)) /* html ' */
               {
                  memmove(p_inc, p + 6, (buf + new_size) - (p + 6));
                  new_size -= 5;
                  *p = '\'';
                  p = p_inc; /* next char */
               }
             else if (!strncmp(p_inc, "nbsp;", 5)) /* html space */
               {
                  memmove(p_inc, p + 6, (buf + new_size) - (p + 6));
                  new_size -= 5;
                  *p = ' ';
                  p = p_inc; /* next char */
               }
             else
               p = p_inc; /* next char */
          }
        else
          p++; /* nothing interesting :) next char */
     }


   DPARSE(("Item clean: size: %d new_size: %d", size, new_size));

   buf = realloc(buf, new_size);
   *buf_org = buf;

   return NEWS_PARSE_ERROR_NO;
}


static char *
_meta_block_find(char **buf)
{
   char *p1, *p2;

   if (!buf || !(*buf) || !(**buf)) return NULL;

   p1 = *buf;
   while (p1 && (*p1 != '>')) p1++;
   if ( !p1 || !(*p1) || (*(p1-1) == '/') || !(*(p1+1)) ) return NULL;
   p1++;
   *buf = p1;

   p2 = _meta_block_end(*buf, 0, 1);

   return p2;
}

static char *
_meta_block_end(char *buf, int beg_before, int end_before)
{
   char *p;
   int layer;

   if (!buf || !(*buf)) return NULL;

   p = buf;
   layer = 0;

   DPARSE(("Looking for end meta block (%20.20s)", p));

   if (beg_before)
     {
        /* Looking for autoclosing meta */
        while (*p && (*p != '>')) p++;
        if (*(p-1) == '/')
          {
             DPARSE(("Searching end of an autoclosing meta !"));
             return buf;
          }
	p++;
     }

   while (*p)
     {
        /* look for meta */
        while (*p && (*p != '<'))
          p++;
        if (*(p + 1))
          {
             /* closing meta */
             if (*(p + 1) == '/')
               {
                  if (!layer) goto done;
                  layer--;
               }
             else
               {
                  /* cdata meta, direct skip
                   * because it cant include another one */
                  if (*(p + 1) == '!')
                    {
                       if (!strncmp(p + 1, "![CDATA", 7))
                         {
                            p = strstr(p + 1, "]>");
                            if (!p)
                              return NULL;
                            p++;
                         }
                    }
                  else
                    {
                       /* looking for autoclosing meta */
                       while (*p && (*p != '>'))
                         p++;
                       if (*(p-1) == '/')
                         ;   /* skip meta */
                       else
                         layer++;   /* opening meta */
                    }
               }
          }
        /* next char */
        p++;
     }

   DPARSE(("End meta NOT found, prematurate end of doc !"));
   return NULL;

 done:

   DPARSE(("End meta found here %15.15s", p));

   if (end_before)
     {
        while ( (*p != '<') ) p--;
     }
   else
     {
        while ( *p && (*p != '>') ) p++;
        if ( !(*p) || !(*(p+1)) ) return NULL;
        p++;
     }

   return p;
}

static const char *
_parse_infos_item(News_Parse *parser, char *meta, const char *prev, int *edit)
{
   const char *item;
   char *ibeg, *iend;
   int   ilen;
   char  buf[8192];

   item = prev;
   ibeg = strstr(parser->buffer_pos, meta);
   if ( ibeg &&
        (ibeg < (parser->doc->server.buffer + parser->doc->server.buffer_size)) &&
        (iend = _meta_block_find(&ibeg)) )
     {
        ilen = iend - ibeg;
        if ( !prev || (prev && strncmp(ibeg, prev, ilen)) )
          {
             memcpy(buf, ibeg, ilen);
             buf[ilen] = '\0';
             item = eina_stringshare_add(buf);
             *edit = 1;
          }
     }

   return item;
}


static void
_parse_finished(News_Parse *parser)
{
   News_Feed_Document *doc;

   doc = parser->doc;

   DPARSE(("Parse finished ! %d articles", eina_list_count(parser->articles)));
  
   if ( (parser->error != NEWS_PARSE_ERROR_TYPE_UNKNOWN) &&
        (parser->error != NEWS_PARSE_ERROR_NOT_IMPLEMENTED))
     {
        _update_doc(parser);
     }

   parser->cb_func(doc, parser->error, parser->changes);
   parser->idler = NULL;
   parser->doc->parse.parser = NULL;

   _parsers = eina_list_remove(_parsers, parser);
   _parse_free(parser);
}

static void
_update_doc(News_Parse *parser)
{
   News_Parse_Article *pa;
   News_Feed_Article *fa;
   Eina_List *list, *l;
   int pos;

   list = NULL;

   /* create a list of new articles */

   for (l=parser->articles; l; l=eina_list_next(l))
     {
        pa = eina_list_data_get(l);
        if (!pa->article)
          {
             /* create a brand new article */
             parser->changes = 1;
             DPARSE(("** New articles list, create fa %s", pa->title));
             fa = E_NEW(News_Feed_Article, 1);
             fa->doc = parser->doc;
             fa->title = pa->title;
             pa->title = NULL;
             fa->url = pa->url;
             pa->url = NULL;
             fa->description = pa->description;
             pa->description = NULL;

             //TODO : call news_util_image_get (get image)
             
             memcpy(&fa->date, &pa->date, sizeof(fa->date));

             /* set as unread if its not first parse */
             if (parser->doc->parse.last_time)
               news_feed_article_unread_set(fa, 1);
          }
        else
          {
             /* remove the article reused from old list */
             DPARSE(("** New articles list, reused fa %s", pa->article->title));
             fa = pa->article;
             fa->reused = 0;
	     parser->doc->articles = eina_list_remove(parser->doc->articles, fa);
          }
        _parse_article_free(pa);

	list = eina_list_append(list, fa);
     }
   eina_list_free(parser->articles);
   parser->articles = NULL;

   /* remove old articles
    * except unread articles, we keep them */
   if ((parser->error != NEWS_PARSE_ERROR_BROKEN_FEED) ||
       (parser->doc->unread_count > NEWS_FEED_UNREAD_COUNT_MAX))
     {
        pos = eina_list_count(parser->doc->articles) - 1;
        while ((fa = eina_list_nth(parser->doc->articles, pos)))
          {
             /* if unread count max reached, remove even if unread */
             if (!fa->unread || (parser->doc->unread_count > NEWS_FEED_UNREAD_COUNT_MAX))
               {
                  news_feed_article_del(fa);
                  parser->changes = 1;
               }
             pos--;
          }
     }

   /* append the old articles list to the new one */
   for (l=parser->doc->articles; l; l=eina_list_next(l))
     {
        fa = l->data;
        list = eina_list_append(list, fa);
     }
   /* and replace the old list by the new one */
   eina_list_free(parser->doc->articles);
   parser->doc->articles = list;

   /* DEBUG : list the articles */
#ifdef DPARSE
   DPARSE(("-- New articles list for feed %s", parser->doc->feed->name));
   for (l=parser->doc->articles; l; l=eina_list_next(l))
     {
        fa = eina_list_data_get(l);
        DPARSE(("- %s", fa->title));
     }
   DPARSE(("---------"));
#endif
}

static void
_parse_free(News_Parse *parser)
{
   if (parser->doc) parser->doc->parse.parser = NULL;
  
   while (parser->articles)
     {
        News_Parse_Article *a;
      
        a = parser->articles->data;
        _parse_article_free(a);
        parser->articles = eina_list_remove_list(parser->articles,
                                                 parser->articles);
     }
   if (parser->idler) ecore_idler_del(parser->idler);
   if (parser->art) _parse_article_free(parser->art);

   free(parser);
}

static void
_parse_article_free(News_Parse_Article *a)
{
   E_FREE(a->title);
   E_FREE(a->url);
   //E_FREE(a->image); //TODO
   E_FREE(a->description);

   free(a);
}
