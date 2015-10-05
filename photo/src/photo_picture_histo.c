#include "Photo.h"



/*
 * Public functions
 */

int photo_picture_histo_init(Photo_Item *pi)
{
   pi->histo.list = NULL;
   pi->histo.pos = 0;

   return 1;
}

void photo_picture_histo_shutdown(Photo_Item *pi)
{
   Eina_List *l;
   Picture *p;
   Photo_Item *pi2;
   int cur;

   if (pi->histo.list)
     {
        for (l=pi->histo.list; l; l=eina_list_next(l))
          {
             p = eina_list_data_get(l);
             cur = 0;
             while ( (pi2 = eina_list_nth(p->items_histo, cur)) )
               {
                  if (pi == pi2)
                    p->items_histo = eina_list_remove(p->items_histo, pi);
                  else
                    cur++;
               }
          }
        eina_list_free(pi->histo.list);
        pi->histo.list = NULL;
     }

   pi->histo.pos = 0;

   if (pi->histo.popi)
     {
       photo_popup_info_del(pi->histo.popi);
       pi->histo.popi = NULL;
     }
}

void photo_picture_histo_attach(Photo_Item *pi, Picture *picture)
{
   pi->histo.list = eina_list_prepend(pi->histo.list, picture);

   /* if full and we are not on the last histo item, remove last */
   if (eina_list_count(pi->histo.list) > PICTURE_HISTO_SIZE_MAX)
     {
        Eina_List *l, *l2;
        l = eina_list_last(pi->histo.list);
        l2 = eina_list_nth_list(pi->histo.list, pi->histo.pos);
        if (l != l2)
          pi->histo.list = eina_list_remove_list(pi->histo.list, l);
     }

   /* add the pi to the picture's items histo list if not already here */
   if (!eina_list_data_find(picture->items_histo, pi))
     picture->items_histo = eina_list_append(picture->items_histo, pi);
}

Picture *photo_picture_histo_change(Photo_Item *pi, int offset)
{
   Picture *picture;
   int new_pos;

   new_pos = pi->histo.pos + offset;

   DPIC(("HISTO change from %d to %d (off %d)", pi->histo.pos, new_pos, offset));

   picture = eina_list_nth(pi->histo.list, new_pos);
   if (picture)
     {
       if (!picture->pi && !picture->delete_me)
	 pi->histo.pos = new_pos;
       else
	 picture = NULL;
     }

   if (!picture)
     DPIC(("HISTO change FAILED !!"));

   return picture;
}

void photo_picture_histo_picture_del(Picture *picture)
{
   Photo_Item *pi;
   Eina_List *l;

   if (!picture) return;
   for (l=picture->items_histo; l; l=eina_list_next(l))
     {
        pi = eina_list_data_get(l);
        while(eina_list_data_find(pi->histo.list, picture))
          pi->histo.list = eina_list_remove(pi->histo.list, picture);
     }

   eina_list_free(picture->items_histo);
}
