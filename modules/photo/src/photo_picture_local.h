#ifdef E_MOD_PHOTO_TYPEDEFS

typedef struct _Picture_Local_Dir Picture_Local_Dir;

#else

#ifndef PHOTO_PICTURE_LOCAL_H_INCLUDED
#define PHOTO_PICTURE_LOCAL_H_INCLUDED

#define PICTURE_LOCAL_SHOW_LOGO_DEFAULT 1
#define PICTURE_LOCAL_IMPORT_RECURSIVE_DEFAULT 0
#define PICTURE_LOCAL_IMPORT_HIDDEN_DEFAULT 0

#define PICTURE_LOCAL_AUTO_RELOAD_DEFAULT 0

#define PICTURE_LOCAL_POPUP_LOADER_MOD 500
#define PICTURE_LOCAL_POPUP_LOADER_TIME 2
#define PICTURE_LOCAL_POPUP_THUMB_MOD 250
#define PICTURE_LOCAL_POPUP_THUMB_TIME 2

#define PICTURE_LOCAL_POPUP_DEFAULT 2
#define PICTURE_LOCAL_POPUP_NEVER 0
#define PICTURE_LOCAL_POPUP_SUM 1
#define PICTURE_LOCAL_POPUP_ALWAYS 2

#define PICTURE_LOCAL_DIR_NOT_LOADED 0
#define PICTURE_LOCAL_DIR_LOADED 1
#define PICTURE_LOCAL_DIR_LOADING 2

#define PICTURE_LOCAL_DIR_RECURSIVE_DEFAULT 0
#define PICTURE_LOCAL_DIR_READ_HIDDEN_DEFAULT 0

#define PICTURE_LOCAL_GET_RANDOM -1

struct _Picture_Local_Dir
{
   const char *path;
   int recursive;
   int read_hidden;
   int state;

   E_Config_Dialog *config_dialog;
};


int                photo_picture_local_init(void);
void               photo_picture_local_shutdown(void);

void               photo_picture_local_load_start(void);
void               photo_picture_local_load_stop(void);
int                photo_picture_local_load_state_get(void);

Picture           *photo_picture_local_get(int position);

int                photo_picture_local_loaded_nb_get(void);
int                photo_picture_local_tothumb_nb_get(void);

void               photo_picture_local_ev_set(Photo_Item *pi);
void               photo_picture_local_ev_raise(int nb);

Picture_Local_Dir *photo_picture_local_dir_new(char *path, int recursive, int read_hidden);
void               photo_picture_local_dir_free(Picture_Local_Dir *dir, int del_dialog);

void               photo_picture_local_picture_deleteme_nb_update(int how_much);

#endif
#endif
