#ifndef NEWS_H
#define NEWS_H

/* Enlightenment includes */
#include <config.h>
#include <e.h>

/* News includes */
#define E_MOD_NEWS_TYPEDEFS 1
#include "News_includes.h"
#undef E_MOD_NEWS_TYPEDEFS
#include "News_includes.h"

/* Debugging things */
#ifdef DEBUG
# define DD(x)  do {printf("DD - %s:%d: ", __FILE__, __LINE__); printf x; printf("\n"); fflush(stdout);} while (0)
#else
# define DD(x)   ((void) 0)
#endif
#ifdef DEBUG_MAIN
# define DMAIN(x)  do {printf("MAIN - %s:%d:  ", __FILE__, __LINE__); printf x; printf("\n"); fflush(stdout);} while (0)
#else
# define DMAIN(x)  ((void) 0)
#endif
#ifdef DEBUG_CONFIG
# define DCONF(x)  do {printf("CONFIG - %s:%d:  ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DCONF(x)  ((void) 0)
#endif
#ifdef DEBUG_ITEM
# define DITEM(x)  do {printf("ITEM - %s:%d:  ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DITEM(x)  ((void) 0)
#endif
#ifdef DEBUG_FEED
# define DFEED(x)  do {printf("FEED - %s:%d:  ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DFEED(x)  ((void) 0)
#endif
#ifdef DEBUG_PARSE
# define DPARSE(x)  do {printf("PARSE - %s:%d:  ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DPARSE(x)  ((void) 0)
#endif
#ifdef DEBUG_BROWSER
# define DBROWSER(x)  do {printf("BROWSER - %s:%d:  ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DBROWSER(x)  ((void) 0)
#endif
#ifdef DEBUG_POPUP
# define DPOP(x)  do {printf("POPUP - %s:%d: ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DPOP(x)  ((void) 0)
#endif
#ifdef DEBUG_VIEWER
# define DVIEWER(x)  do {printf("VIEWER - %s:%d: ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DVIEWER(x)  ((void) 0)
#endif

#endif
