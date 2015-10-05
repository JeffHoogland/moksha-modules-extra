#include "Photo.h"

/*
 * Public functions
 */

int photo_picture_init(void)
{
   E_Container *cont;

   cont = e_container_current_get(e_manager_current_get());
   photo->e_evas = cont->bg_evas;

   if (!photo_picture_local_init())
     return 0;
   //if (!photo_picture_net_init())
   // return 0;

   return 1;
}

void photo_picture_shutdown(void)
{
   photo_picture_setbg_purge(1);

   photo_picture_local_shutdown();
   //photo_picture_net_shutdown();
}

Picture *photo_picture_new(char *path, int thumb_it, void (*func_done) (void *data, Evas_Object *obj, void *event_info))
{
   Picture *picture;
   int th_w, th_h;
   char *ext;

   th_w = photo->config->pictures_thumb_size;
   th_h = photo->config->pictures_thumb_size;
      
   ext = strrchr(path, '.');
   if (!ext)
     return NULL;
   if (strcasecmp(ext, ".jpg") && strcasecmp(ext, ".JPG") &&
       strcasecmp(ext, ".jpeg") && strcasecmp(ext, ".JPEG") &&
       strcasecmp(ext, ".png") && strcasecmp(ext, ".PNG"))
     return NULL;
      
   DPICL(("New picture :  file %s", path));
      
   picture = E_NEW(Picture, 1);
   picture->path = eina_stringshare_add(path);
   picture->infos.name = photo_picture_name_get(path);
   picture->from = PICTURE_LOCAL;

   if (thumb_it)
     {
        Evas_Object *im;

        picture->thumb = PICTURE_THUMB_WAITING;
        im = e_thumb_icon_add(photo->e_evas);
        DPICL(("THUMB of %s wanted at %dx%d", picture->path, th_w, th_h));
        e_thumb_icon_file_set(im, (char *)picture->path, NULL);
        e_thumb_icon_size_set(im, 128, 128);
        evas_object_smart_callback_add(im, "e_thumb_gen", func_done, picture);
        picture->picture = im;
        e_thumb_icon_begin(im);
     }

   return picture;
}

int photo_picture_free(Picture *p, int force, int force_now)
{
   if (p->pi)
     {
        if (!force) return 0;
        if (!force_now)
          {
	    if (!p->delete_me)
	      {
		if (p->from == PICTURE_LOCAL)
		  photo_picture_local_picture_deleteme_nb_update(+1);
		p->delete_me = 1;
	      }
             return 0;
          }
     }

   if (p->delete_me)
     {
       if (p->from == PICTURE_LOCAL)
	 photo_picture_local_picture_deleteme_nb_update(-1);
     }

   DD(("Picture Free : %s", p->path));
   if (p->path) eina_stringshare_del(p->path);
   if (p->picture) evas_object_del(p->picture);

   if (p->infos.name) eina_stringshare_del(p->infos.name);
   if (p->infos.author) eina_stringshare_del(p->infos.author);
   if (p->infos.where_from) eina_stringshare_del(p->infos.where_from);
   if (p->infos.date) eina_stringshare_del(p->infos.date);
   if (p->infos.comments) eina_stringshare_del(p->infos.comments);

   photo_picture_histo_picture_del(p);

   free(p);
   return 1;
}

int photo_picture_load(Picture *pic, Evas *evas)
{
   Evas_Object *im = NULL;

   if (pic->picture)
     return 1;

   im = photo_picture_object_get(pic, evas);
   if (!im)
     return 0;
   
   pic->picture = im;

   return 1;
}

void photo_picture_unload(Picture *pic)
{
   if (!pic->picture)
     return;

   evas_object_del(pic->picture);
   pic->picture = NULL;
}

Evas_Object *photo_picture_object_get(Picture *pic, Evas *evas)
{
   Evas_Object *im = NULL;
   int th_w, th_h;

   th_w = photo->config->pictures_thumb_size;
   th_h = photo->config->pictures_thumb_size;

   if (!pic) return NULL;

   if (pic->thumb)
     {
        /* load picture thumb */
        im = e_thumb_icon_add(evas);
        e_thumb_icon_file_set(im, (char *)pic->path, NULL);
        e_thumb_icon_size_set(im, th_w, th_h);
        e_thumb_icon_begin(im);
     }
   else
     {
       int sw, sh;

        /* load picture */
        im = e_icon_add(evas);
        e_icon_file_set(im, pic->path);

        e_icon_size_get(im, &sw, &sh);
        evas_object_resize(im, sw, sh);
        e_icon_fill_inside_set(im, 1);
     }

   return im;
}

const char *photo_picture_name_get(char *url)
{
   char buf[4096];
   char *name, *ext;
   int name_l;

   name = strrchr(url, '/');
   if (name)
     name++;
   else
     name = url;

   ext = strrchr(name, '.');
   if (ext)
     name_l = strlen(name) - strlen(ext);
   else
     name_l = strlen(name);
   
   strncpy(buf, name, name_l);
   name[name_l] = '\0';

   return eina_stringshare_add(name);
}

char *photo_picture_infos_get(Picture *p)
{
   char buf[4096];
   char buf_ext[4096];

   if (ecore_file_exists(p->path))
     {
        time_t date;
        char *date_ascii;

        date = ecore_file_mod_time(p->path);
        date_ascii = (char *)ctime(&date);
        date_ascii[strlen(date_ascii) - 1] = ' ';
        snprintf(buf_ext, sizeof(buf_ext),
                 "<underline=on underline_color=#000>Date :</> %s<br>"
                 "<underline=on underline_color=#000>Size :</> %.2fMo",
                 date_ascii, (float)ecore_file_size(p->path) / 1000000.0);
     }
   else
     {
        snprintf(buf_ext, sizeof(buf_ext), "This file does not exist anymore on the disk !");
     }

   snprintf(buf, sizeof(buf),
            "<underline=on underline_color=#000>Picture path :</> %s<br>"
	    "<br>"
            "%s",
            p->path,
	    buf_ext);

   return strdup(buf);
}

void photo_picture_setbg_add(const char *name)
{
   char buf[4096];
   const char *home;

   home = e_user_homedir_get();
   snprintf(buf, sizeof(buf), "%s/.e/e/backgrounds/%s.edj", home, name);

   photo->setbg_topurge = eina_list_append(photo->setbg_topurge, strdup(buf));
}

void photo_picture_setbg_purge(int shutdown)
{
   char *file;
   int n = 0;

   while( (file = eina_list_nth(photo->setbg_topurge, n)) )
     {
        n++;
        DMAIN(("set_bg check remove %s", file));
        if (!e_config->desktop_default_background ||
            strcmp(e_config->desktop_default_background, file))
          {
             DMAIN(("set_bg remove %s !", file));
             if (ecore_file_exists(file))
               ecore_file_unlink(file);
             photo->setbg_topurge = eina_list_remove(photo->setbg_topurge, file);
             free(file);
             n--;
          }
        else
          {
             if (shutdown)
               free(file);
          }
     }

   if (shutdown)
     eina_list_free(photo->setbg_topurge);

   return;
}

/*
 * Private functions
 *
 */
