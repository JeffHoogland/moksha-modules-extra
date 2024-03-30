#ifdef E_MOD_PHOTO_TYPEDEFS

#else

#ifndef E_MOD_PICTURE_NET_H_INCLUDED
#define E_MOD_PICTURE_NET_H_INCLUDED

#define PICTURE_NET_GET_RANDOM -1

int      photo_picture_net_init(void);
void     photo_picture_net_shutdown(void);

Picture *photo_picture_net_get(int position);

void     photo_picture_net_ev_set(Photo_Item *pi);

#endif
#endif
