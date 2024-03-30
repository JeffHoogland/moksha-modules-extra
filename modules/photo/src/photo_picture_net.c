#include "Photo.h"

typedef struct _Picture_Net_List Picture_Net_List;

struct _Picture_Net_List
{
   Eina_List *pictures;
   int nb_pictures_waiting;
};

static Picture_Net_List *pictures_net;


/*
 * Public functions
 */

int      photo_picture_net_init(void)
{
  return 0;
}

void     photo_picture_net_shutdown(void)
{
  pictures_net = NULL;
}


/*
 * Private functions
 *
 */
