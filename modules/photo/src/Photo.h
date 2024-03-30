#ifndef PHOTO_H
#define PHOTO_H

/* Enlightenment includes */
#include <config.h>
#include <e.h>

/* Photo includes */
#define E_MOD_PHOTO_TYPEDEFS 1
#include "Photo_includes.h"
#undef E_MOD_PHOTO_TYPEDEFS
#include "Photo_includes.h"

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
#ifdef DEBUG_PICTURE
# define DPIC(x)  do {printf("PICTURE - %s:%d:  ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DPIC(x)  ((void) 0)
#endif
#ifdef DEBUG_PICTURE_LOCAL
# define DPICL(x)  do {printf("PICTURE LOCAL - %s:%d:  ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DPICL(x)  ((void) 0)
#endif
#ifdef DEBUG_PICTURE_NET
# define DPICN(x)  do {printf("PICTURE NET - %s:%d:  ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DPICN(x)  ((void) 0)
#endif
#ifdef DEBUG_POPUP_WARN
# define DPOPW(x)  do {printf("POPW - %s:%d: ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DPOPW(x)  ((void) 0)
#endif
#ifdef DEBUG_POPUP_INFO
# define DPOPI(x)  do {printf("POPI - %s:%d: ", __FILE__, __LINE__); printf x; printf ("\n"); fflush(stdout);} while (0)
#else
# define DPOPI(x)  ((void) 0)
#endif

#endif
