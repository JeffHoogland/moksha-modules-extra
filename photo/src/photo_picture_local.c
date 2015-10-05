#include "Photo.h"

#define POPUP_LOADING(list, buf, dtime)                                \
if (list->loader.popup)                                                \
  photo_popup_warn_del(list->loader.popup);                            \
list->loader.popup =                                                   \
  photo_popup_warn_add(POPUP_WARN_TYPE_NEWS, buf,                      \
                       PICTURE_LOCAL_POPUP_LOADER_TIME + dtime,        \
                       _popup_loader_close, _popup_loader_desactivate, \
		       NULL);

#define POPUP_THUMBNAILING(list, buf, dtime)                         \
if (list->thumb.popup)                                               \
  photo_popup_warn_del(list->thumb.popup);                            \
list->thumb.popup =                                                  \
  photo_popup_warn_add(POPUP_WARN_TYPE_NEWS, buf,                    \
                       PICTURE_LOCAL_POPUP_THUMB_TIME + dtime,       \
                       _popup_thumb_close, _popup_thumb_desactivate, \
                       NULL);


typedef struct _Picture_Local_List Picture_Local_List;

struct _Picture_Local_List
{
   Eina_List *pictures;
  int pictures_waiting_delete;

   /* thumb */
  struct
  {
     int nb;
     Popup_Warn *popup;
     int         popup_show;
  } thumb;

   /* ecore idler to load in background */
   struct
   {
      Eina_List *queue;

      Ecore_Idler *idler;
      Ecore_Timer *timer;
      Popup_Warn  *popup;

      Picture_Local_Dir *current_dir;

      Eina_List  *dirs;
      DIR *odir;
   } loader;

   /* event to warn photo items wich are waiting for pictures */
   struct
   {
      int id;
      int nb_clients;
   } loader_ev;
};

static Picture_Local_List *pictures_local;


static void _pictures_old_del(int force, int force_now);

static Eina_Bool  _load_idler(void *data);
static Eina_Bool  _load_timer(void *data);
static void _load_idler_stop(void);
static Eina_Bool  _load_cb_ev_fill(void *data, int type, void *event);

static void _thumb_generate_cb(void *data, Evas_Object *obj, void *event_info);
static void _thumb_generate_stop(void);

static int  _popup_loader_close(Popup_Warn *popw, void *data);
static void _popup_loader_desactivate(Popup_Warn *popw, void *data);
static int  _popup_thumb_close(Popup_Warn *popw, void *data);
static void _popup_thumb_desactivate(Popup_Warn *popw, void *data);


/*
 * Public functions
 */

int photo_picture_local_init(void)
{
   Picture_Local_List *list;

   list = E_NEW(Picture_Local_List, 1);
   list->loader_ev.id = ecore_event_type_new();
   pictures_local = list;

   photo_picture_local_load_start();

   return 1;
}

void photo_picture_local_shutdown(void)
{
   photo_picture_local_load_stop();
   
   _pictures_old_del(1, 1);

   eina_list_free(pictures_local->pictures);
   pictures_local = NULL;
}

void photo_picture_local_load_start(void)
{
   Eina_List *l;

   photo_picture_local_load_stop();

   /* del old pictures */
   _pictures_old_del(1, 0);

   /* set all directories to not loaded */
   for (l=photo->config->local.dirs; l; l=eina_list_next(l))
     {
        Picture_Local_Dir *d;
        d = eina_list_data_get(l);
        d->state = PICTURE_LOCAL_DIR_NOT_LOADED;
     }

   /* initialise thumbnailer */
   pictures_local->thumb.nb = 0;
   pictures_local->thumb.popup_show = 1;
   pictures_local->thumb.popup = NULL;

   /* initialise and launch loader */
   pictures_local->loader.queue = NULL;
   pictures_local->loader.idler = ecore_idler_add(_load_idler, NULL);
   pictures_local->loader.timer = ecore_timer_add(0.2, _load_timer, NULL);

   if (photo->config_dialog)
     photo_config_dialog_refresh_local_load();
}

void photo_picture_local_load_stop(void)
{
   _load_idler_stop();
   _thumb_generate_stop();

   if (photo->config_dialog)
     {
        photo_config_dialog_refresh_local_infos();
        photo_config_dialog_refresh_local_dirs();
        photo_config_dialog_refresh_local_load();
     }
}

int photo_picture_local_load_state_get(void)
{
   if (pictures_local->thumb.nb ||
       pictures_local->loader.idler)
     return 1;

   return 0;
}

Picture *photo_picture_local_get(int position)
{
   Picture *picture;
   Eina_List *l, *was_first;
   Picture_Local_List *pl;

   pl = pictures_local;

   DPICL(("Trying to get a picture (%d), position = %d",
          eina_list_count(pl->pictures), position));

   if (!(eina_list_count(pl->pictures) - pl->pictures_waiting_delete))
     return NULL;

   if (position != PICTURE_LOCAL_GET_RANDOM)
     {
        /* get the given picture */
        l = eina_list_nth_list(pl->pictures, position);
        picture = eina_list_data_get(l);
     }
   else
     {
        /* get a random picture */
        l = eina_list_nth_list(pl->pictures,
                               rand() % eina_list_count(pl->pictures));
        was_first = l;
        do
          {
             DD(("- Search -"));
             picture = eina_list_data_get(l);
             if (!picture->pi && !picture->delete_me &&
		 (picture->thumb != PICTURE_THUMB_WAITING))
               return picture;
             l = eina_list_next(l);
             if (!l) l = pl->pictures;
          } while (l != was_first);
        picture = NULL;
     }
  
   return picture;
}

int photo_picture_local_loaded_nb_get(void)
{
  return ((eina_list_count(pictures_local->pictures) -
	  pictures_local->thumb.nb) -
	  pictures_local->pictures_waiting_delete);
}

int photo_picture_local_tothumb_nb_get(void)
{
   return pictures_local->thumb.nb;
}

void photo_picture_local_ev_set(Photo_Item *pi)
{
   if (pi->local_ev_fill_handler)
     return;

   DPICL(("Set event fill"));

   pictures_local->loader_ev.nb_clients++;
   pi->local_ev_fill_handler = ecore_event_handler_add(pictures_local->loader_ev.id,
                                                       _load_cb_ev_fill,
                                                       pi);
}

void photo_picture_local_ev_raise(int nb)
{
   Picture_Event_Fill *ev;
	 
   if (!pictures_local->loader_ev.nb_clients) return;

   ev = E_NEW(Picture_Event_Fill, 1);
   ev->new = nb;
   ev->type = PICTURE_LOCAL;
   /* raise event to warn clients : a picture is here for you ! */
   ecore_event_add(pictures_local->loader_ev.id, ev, NULL, NULL);
   DPICL(("Loader event RAISED !"));
}

void photo_picture_local_picture_deleteme_nb_update(int how_much)
{
  pictures_local->pictures_waiting_delete += how_much;
}

Picture_Local_Dir *photo_picture_local_dir_new(char *path, int recursive, int read_hidden)
{
   Picture_Local_Dir *dir;

   if (!ecore_file_is_dir(path))
     {
        char buf[4096];
        snprintf(buf, sizeof(buf),
                 D_("<hilight>Directory %s doesnt exists.</hilight><br><br>"
                   "You can change the picture's folders in main configuration panel<br>"
                   "They can be jpeg or png<br><br>"
                   "After import, if you can remove these files and the pictures still can<br>"
                   "be viewed, but you wont be able to set them as wallpaper anymore<br><br>"),
                 path);
        e_module_dialog_show(photo->module, D_("Photo Module Error"), buf);
        return NULL;
     }

   dir = E_NEW(Picture_Local_Dir, 1);
   dir->path = eina_stringshare_add(path);
   dir->recursive = recursive;
   dir->read_hidden = read_hidden;
   dir->state = PICTURE_LOCAL_DIR_NOT_LOADED;

   DPICL(("New directory created : %s", path));

   return dir;
}

void photo_picture_local_dir_free(Picture_Local_Dir *dir, int del_dialog)
{
   if (dir->config_dialog && del_dialog)
     photo_config_dialog_dir_hide(dir);
   eina_stringshare_del(dir->path);
   free(dir);
}


/*
 * Private functions
 */

static void
_pictures_old_del(int force, int force_now)
{
   Picture *p;
   int no = 0;
   
   while ( (p = eina_list_nth(pictures_local->pictures, no)) )
     {
        if (photo_picture_free(p, force, force_now))
          pictures_local->pictures = eina_list_remove(pictures_local->pictures, p);
        else
	  no++;
     }
}

static Eina_Bool
_load_idler(void *data)
{
   Picture_Local_List *pl;
   Picture_Local_Dir *d;
   Eina_List *l;
   char *file_tmp;
   char file[200];
   struct dirent *fs;
   struct stat fs_stat;

   pl = pictures_local;

   DD(("o"));

   /* if the E thumbnailer is too busy, wait */
   if (pl->thumb.nb >= 2) return EINA_TRUE;

   /* no more dirs in the current_dir */
   if (!eina_list_count(pl->loader.dirs))
     {
        /* find a dir to load in user dir list */
        if (pl->loader.current_dir)
          {
             pl->loader.current_dir->state = PICTURE_LOCAL_DIR_LOADED;
             pl->loader.current_dir = NULL;
	     if (photo->config_dialog)
	       photo_config_dialog_refresh_local_dirs();
          }
        DD(("oo"));
        for (l=photo->config->local.dirs; l; l=eina_list_next(l))
          {
             d = eina_list_data_get(l);
             DD(("ooo"));
             if (d->state == PICTURE_LOCAL_DIR_NOT_LOADED)
               {
                  d->state = PICTURE_LOCAL_DIR_LOADING;
                  pl->loader.current_dir = d;
                  pl->loader.dirs = eina_list_append(pl->loader.dirs,
                                                     strdup(d->path));
		  pl->loader.odir = NULL;
                  DPICL(("Going to read %s", d->path));
		  if (photo->config_dialog)
		    photo_config_dialog_refresh_local_dirs();
                  return EINA_TRUE;
               }
          }
        /* no more directories to load */
        if (!pl->loader.current_dir)
          {
             pl->loader.odir = NULL;
             pl->loader.idler = NULL;
             if (photo->config_dialog)
               {
                  photo_config_dialog_refresh_local_infos();
                  photo_config_dialog_refresh_local_load();
               }
	     /* last loading popup message */
             if (photo->config->local.popup >= PICTURE_LOCAL_POPUP_SUM)
               {
		 char buf[50];

		 snprintf(buf, sizeof(buf), "Scan finished : %d pictures found",
			  eina_list_count(pl->pictures) - pl->pictures_waiting_delete);
		 POPUP_LOADING(pl, buf, 3);
               }
             if (pl->loader.timer)
               {
                  ecore_timer_del(pl->loader.timer);
               }
             pl->loader.timer = ecore_timer_add(0.0001, _load_timer, NULL);
             return EINA_FALSE;
          }
     }

   /* first dir list */
   if ( !pl->loader.odir )
     pl->loader.odir = opendir((char *)eina_list_data_get(pl->loader.dirs));

   /* no more files in the current loader.dirs item */
   if ( !pl->loader.odir || !(fs = readdir(pl->loader.odir)) )
     {
        DD(("removing %s", (char *)eina_list_data_get(pl->loader.dirs)));
        /* go to next dir */
        closedir(pl->loader.odir);
	pl->loader.odir = NULL;
        free(eina_list_data_get(pl->loader.dirs));
        pl->loader.dirs = eina_list_remove_list(pl->loader.dirs,
                                                pl->loader.dirs);
        return EINA_TRUE;
     }

   if ( (!strcmp(fs->d_name, ".")) || (!strcmp(fs->d_name, "..")) ||
        (!pl->loader.current_dir->read_hidden && (fs->d_name[0] == '.')) )
     return EINA_TRUE;

   snprintf(file, sizeof(file),
            "%s/%s", (char *)eina_list_data_get(pl->loader.dirs), fs->d_name);

   if (stat(file, &fs_stat) < 0) return EINA_TRUE;

   if ( (S_ISLNK(fs_stat.st_mode)) &&
        (file_tmp = ecore_file_readlink(file)) )
     {
        strncpy(file, file_tmp, sizeof(file));
        if (stat(file, &fs_stat) < 0) return EINA_FALSE;
     }

   if ( pl->loader.current_dir->recursive &&
        (S_ISDIR(fs_stat.st_mode)) )
     {
        pl->loader.dirs = eina_list_append(pl->loader.dirs, strdup(file));
        DPICL(("added %s to loader dirs", file));
        return EINA_TRUE;
     }

   /* enqueue the file */
   pl->loader.queue = eina_list_append(pl->loader.queue, strdup(file));
   
   return EINA_TRUE;
}

static Eina_Bool
_load_timer(void *data)
{
   Picture_Local_List *pl;
   Picture *picture;
   char *file;
   int rounds;

   pl = pictures_local;

   rounds = 0;
   while(pl->loader.queue)// && (rounds < 50)) //FIXME
     {
        file = pl->loader.queue->data;

        /* create the picture */
        picture = photo_picture_new(file, 1, _thumb_generate_cb);
        if (picture)
          {
             pl->thumb.nb++;
             pl->pictures = eina_list_append(pl->pictures, picture);
             
             /* loader popups */
             if (photo->config->local.popup == PICTURE_LOCAL_POPUP_ALWAYS)
               {
                  int nb;
                  
                  /* loading popup message */        
                  nb = eina_list_count(pl->pictures) - pl->pictures_waiting_delete;
                  if (nb && ((nb == 1) || !(nb%PICTURE_LOCAL_POPUP_LOADER_MOD)))
                    {
                       char buf[50];
                       
                       if (nb == 1)
                         snprintf(buf, sizeof(buf), "Scanning for pictures");
                       else
                         snprintf(buf, sizeof(buf), "%d pictures found", nb);
                       POPUP_LOADING(pl, buf, 0);
                    }
               }
          }

        free(file);
        pl->loader.queue = eina_list_remove_list(pl->loader.queue,
                                                 pl->loader.queue);
        rounds++;
     }

   if (!pl->loader.idler)
     {
        pl->loader.timer = NULL;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_load_idler_stop(void)
{
   Picture_Local_List *pl;

   pl = pictures_local;

   DPICL(("Idler stop !"));

   if (pl->loader.idler)
     {
        ecore_idler_del(pl->loader.idler);
        pl->loader.idler = NULL;
     }
   if (pl->loader.timer)
     {
        ecore_timer_del(pl->loader.timer);
        pl->loader.timer = NULL;
     }

   if (pl->loader.popup)
     {
        photo_popup_warn_del(pl->loader.popup);
        pl->loader.popup = NULL;
     }

   if (pl->loader.current_dir)
     pl->loader.current_dir->state = PICTURE_LOCAL_DIR_NOT_LOADED;
   pl->loader.current_dir = NULL;

   if (pl->loader.dirs)
     {
	Eina_List *l;

        for(l=pl->loader.dirs; l; l=eina_list_next(l))
          {
	     char *name;
             name = eina_list_data_get(l);
             free(name);
          }
        eina_list_free(pl->loader.dirs);
        pl->loader.dirs = NULL;
     }

   if (pl->loader.odir)
     {
        pl->loader.odir = NULL;
     }
}

static Eina_Bool
_load_cb_ev_fill(void *data, int type, void *event)
{
   Photo_Item *pi;
   Picture_Event_Fill *ev;

   pi = data;
   ev = event;

   DPICL(("Loader - EVENT -"));

   ecore_event_handler_del(pi->local_ev_fill_handler);
   pi->local_ev_fill_handler = NULL;

   photo_item_action_change(pi, 1);

   pictures_local->loader_ev.nb_clients--;
   DPICL(("Loader event nb_clients : %d", pictures_local->loader_ev.nb_clients));

   ev->new--;
   if (!ev->new)
     return EINA_FALSE;return EINA_TRUE;
}

static void
_thumb_generate_cb(void *data, Evas_Object *obj, void *event_info)
{
   Picture *picture;
   Picture_Local_List *pl;

   if (!photo || !e_module_enabled_get(photo->module))
     return;

   pl = pictures_local;
   picture = data;

   pl->thumb.nb--;

   if (!obj)
     {
        DPICL(("generated object is NULL !!"));
	pl->pictures = eina_list_remove(pl->pictures, picture);
        photo_picture_free(picture, 1, 1);
        return;
     }

   evas_object_geometry_get(obj, NULL, NULL,
			    &picture->original_w, &picture->original_h);
   DPICL(("thumb generated %dx%d", picture->original_w, picture->original_h));
   

   picture->thumb = PICTURE_THUMB_READY;

   /* popups about thumbnailing */

   /* thumbnailing message, only one time */
   if (photo->config->local.thumb_msg)
     {
        photo->config->local.thumb_msg = 0;
	photo_config_save();
        e_module_dialog_show(photo->module, D_("Photo Module Information"),
                             D_("<hilight>Creating thumbs</hilight><br><br>"
                               "Some pictures are being thumbed in a <hilight>background task</hilight>.<br>"
                               "It can take a while, but after, loading will be faster and lighter :)<br><br>"
                               "Each time wou will load pictures that haven't been loaded in Photo module before,<br>"
                               "they will be thumbed.<br><br>"
			       "While creating popups, you will not be able to see any picture in Photo.<br>"
			       "I hope i'll be able to change that :)"));
     }

   /* when still thumbnailing after loading */
   if ((photo->config->local.popup >= PICTURE_LOCAL_POPUP_SUM) &&
       !pl->loader.idler)
     {
        if (!pl->thumb.nb)
          {
             /* Last thumbnailing popup message */
             POPUP_THUMBNAILING(pl, "Thumbnailing finished :)", 2);
          }
        else
          {
             if (photo->config->local.popup == PICTURE_LOCAL_POPUP_ALWAYS)
               {
                  /* thumbnailing popup message */
                  if (!(pl->thumb.nb%PICTURE_LOCAL_POPUP_THUMB_MOD))
                    {
                       char buf[50];
                       
                       snprintf(buf, sizeof(buf), "Still %d pictures to thumbnail",
                                pl->thumb.nb);
                       POPUP_THUMBNAILING(pl, buf, 0);
                    }
               }
          }
     }

   /* refreshes */

   if (photo->config_dialog)
     {
       if (!pl->thumb.nb)
	 {
	   photo_config_dialog_refresh_local_load();
	   photo_config_dialog_refresh_local_infos();
	 }
       if ( !(pl->thumb.nb%100) )
	 photo_config_dialog_refresh_local_infos();
     }

   /* new picture event */

   if (pl->loader_ev.nb_clients)
     photo_picture_local_ev_raise(1);

   /* if the pic is loaded, remove it, we dont want it */

   if (picture->picture)
     {
        evas_object_del(picture->picture);
        picture->picture = NULL;
     }
}

static void
_thumb_generate_stop(void)
{
   Picture_Local_List *pl;
   Picture *p;
   int no = 0;

   pl = pictures_local;

   if (pl->thumb.nb)
     {   
        while ( (p = eina_list_nth(pictures_local->pictures, no)) )
          {
             if (p->thumb == PICTURE_THUMB_WAITING)
               {
                  e_thumb_icon_end(p->picture);
                  photo_picture_free(p, 1, 1);
                  pictures_local->pictures = eina_list_remove(pictures_local->pictures,
                                                              p);
               }
             else
               no++;
          }
        pl->thumb.nb = 0;
     }

   if (pl->thumb.popup)
     {
        photo_popup_warn_del(pl->thumb.popup);
        pl->thumb.popup = NULL;
     }
}

static int
_popup_loader_close(Popup_Warn *popw, void *data)
{
   pictures_local->loader.popup = NULL;
   return 1;
}

static void
_popup_loader_desactivate(Popup_Warn *popw, void *data)
{
   if (photo->config->local.popup != PICTURE_LOCAL_POPUP_NEVER)
     photo->config->local.popup--;
   photo_config_save();
}

static int
_popup_thumb_close(Popup_Warn *popw, void *data)
{
   pictures_local->thumb.popup = NULL;
   return 1;
}

static void
_popup_thumb_desactivate(Popup_Warn *popw, void *data)
{
   if (photo->config->local.popup != PICTURE_LOCAL_POPUP_NEVER)
     photo->config->local.popup--;
   photo_config_save();
}
