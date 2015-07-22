#include "e_mod_main.h"

Ngi_Box *
ngi_box_new(Ng *ng)
{
   Ngi_Box *box;

   box = E_NEW(Ngi_Box, 1);
   box->ng = ng;
   box->pos = 0;
   box->apps = NULL;
   box->drop_handler = NULL;
   box->item_drop = NULL;
   box->dnd_timer = NULL;
   box->items = NULL;

   box->separator = edje_object_add(ng->evas);

   switch(box->ng->cfg->orient)
     {
      case E_GADCON_ORIENT_BOTTOM:
	 ngi_object_theme_set(box->separator, "e/modules/engage/separator_bottom");
	 break;
      case E_GADCON_ORIENT_TOP:
	 ngi_object_theme_set(box->separator, "e/modules/engage/separator_top");
         break;
      case E_GADCON_ORIENT_LEFT:
	 ngi_object_theme_set(box->separator, "e/modules/engage/separator_left");
         break;
      case E_GADCON_ORIENT_RIGHT:
	 ngi_object_theme_set(box->separator, "e/modules/engage/separator_right");
         break;
     }

   const char *clip_to = edje_object_data_get(box->separator, "clip_to_background");

   if (clip_to && atoi(clip_to))
      evas_object_clip_set(box->separator, ng->bg_clip);

   evas_object_clip_set(box->separator, ng->bg_clip);

   ng->boxes = eina_list_append(ng->boxes, box);

   return box;
}

void
ngi_box_free(Ngi_Box *box)
{
   box->ng->boxes = eina_list_remove(box->ng->boxes, box);

   while(box->items)
     ngi_item_free(box->items->data);

   if (box->separator)
      evas_object_del(box->separator);

   E_FREE(box);
}
