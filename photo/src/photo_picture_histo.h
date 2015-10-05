#ifdef E_MOD_PHOTO_TYPEDEFS



#else

#ifndef PHOTO_PICTURE_HISTO_H_INCLUDED
#define PHOTO_PICTURE_HISTO_H_INCLUDED

#define PICTURE_HISTO_SIZE_MAX 7

#define PICTURE_HISTO_POPUP_TIMER 5

int      photo_picture_histo_init(Photo_Item *pi);
void     photo_picture_histo_shutdown(Photo_Item *pi);

void     photo_picture_histo_attach(Photo_Item *pi, Picture *picture);

Picture *photo_picture_histo_change(Photo_Item *pi, int offset);

void     photo_picture_histo_picture_del(Picture *picture);

#endif
#endif
