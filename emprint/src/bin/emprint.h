#ifndef _EMPRINT_H
#define _EMPRINT_H

typedef struct _Options Options;
typedef struct _Band Band;

struct _Options 
{
   int beep, delay, use_thumb;
   int quality, region, window;
   const char *app, *filename;
   struct 
     {
	int width, height, size;
	const char *filename;
     } thumb;
};

struct _Band 
{
   Ecore_Evas *ee;
   Evas *evas;
   Evas_Object *edj;
};

#endif
