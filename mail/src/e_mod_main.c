/*
 * Copyright (C) 2006 Christopher Michael
 *
 * Portions of this code Copyright (C) 2004 Embrace project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it cwill be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <e.h>
#include "e_mod_main.h"
#ifdef IMAP2
#include "imap2.h"
#else
#include "imap.h"
#endif
#include "pop.h"
#include "mdir.h"
#include "mbox.h"

/* Func Protos for Gadcon */
static E_Gadcon_Client *_gc_init (E_Gadcon * gc, const char *name,
				  const char *id, const char *style);
static void _gc_shutdown (E_Gadcon_Client * gcc);
static void _gc_orient (E_Gadcon_Client * gcc, E_Gadcon_Orient orient);
static const char *_gc_label (const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon (const E_Gadcon_Client_Class *client_class, Evas * evas);
static const char *_gc_id_new (const E_Gadcon_Client_Class *client_class);

/* Func Protos for Module */
static void _mail_cb_mouse_down (void *data, Evas * e, Evas_Object * obj,
				 void *event_info);
static void _mail_cb_mouse_in (void *data, Evas * e, Evas_Object * obj,
			       void *event_info);
static void _mail_cb_mouse_out (void *data, Evas * e, Evas_Object * obj,
				void *event_info);
static void _mail_menu_cb_configure (void *data, E_Menu * m,
				     E_Menu_Item * mi);
static void _mail_menu_cb_post (void *data, E_Menu * m);
static void _mail_menu_cb_exec (void *data, E_Menu * m, E_Menu_Item * mi);
static Config_Item *_mail_config_item_get (const char *id);
static Mail *_mail_new (Evas * evas);
static void _mail_free (Mail * mail);
static Eina_Bool _mail_cb_check (void *data);
static Eina_Bool _mail_cb_exe_exit (void *data, int type, void *event);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;
static E_Config_DD *conf_box_edd = NULL;

Config *mail_config = NULL;

static Ecore_Event_Handler *exit_handler;

static const E_Gadcon_Client_Class _gc_class = {
  GADCON_CLIENT_CLASS_VERSION, "mail", 
  {
     _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, NULL
  },
  E_GADCON_CLIENT_STYLE_PLAIN
};

static E_Gadcon_Client *
_gc_init (E_Gadcon * gc, const char *name, const char *id, const char *style)
{
  E_Gadcon_Client *gcc;
  Instance *inst;
  Mail *mail;
  Eina_List *l;
  int have_pop = 0, have_imap = 0, have_mbox = 0;

  inst = E_NEW (Instance, 1);
  inst->ci = _mail_config_item_get (id);

  mail = _mail_new (gc->evas);
  mail->inst = inst;
  inst->mail = mail;

  gcc = e_gadcon_client_new (gc, name, id, style, mail->mail_obj);
  gcc->data = inst;
  inst->gcc = gcc;
  inst->mail_obj = mail->mail_obj;

  evas_object_event_callback_add (inst->mail_obj, EVAS_CALLBACK_MOUSE_DOWN,
				  _mail_cb_mouse_down, inst);
  evas_object_event_callback_add (inst->mail_obj, EVAS_CALLBACK_MOUSE_IN,
				  _mail_cb_mouse_in, inst);
  evas_object_event_callback_add (inst->mail_obj, EVAS_CALLBACK_MOUSE_OUT,
				  _mail_cb_mouse_out, inst);

  if (inst->ci->show_label)
    edje_object_signal_emit (inst->mail_obj, "label_active", "");
  else
    edje_object_signal_emit (inst->mail_obj, "label_passive", "");

  mail_config->instances = eina_list_append (mail_config->instances, inst);
  for (l = inst->ci->boxes; l; l = l->next)
    {
       Config_Box *cb;

       cb = l->data;
       switch (cb->type)
	 {
	  case MAIL_TYPE_IMAP:
	     have_imap = 1;
	     _mail_imap_add_mailbox (cb);
	     if (!inst->check_timer)
	       inst->check_timer =
		  ecore_timer_add ((inst->ci->check_time * 60.0), _mail_cb_check,
			inst);
	     break;
	  case MAIL_TYPE_POP:
	     have_pop = 1;
	     _mail_pop_add_mailbox (cb);
	     if (!inst->check_timer)
	       inst->check_timer =
		  ecore_timer_add ((inst->ci->check_time * 60.0), _mail_cb_check,
			inst);
	     break;
	  case MAIL_TYPE_MDIR:
	     _mail_mdir_add_mailbox (inst, cb);
	     break;
	  case MAIL_TYPE_MBOX:
	     have_mbox = 1;
	     _mail_mbox_add_mailbox (inst, cb);
	     if (!inst->check_timer)
	       inst->check_timer =
		  ecore_timer_add ((inst->ci->check_time * 60.0), _mail_cb_check,
			inst);
	     break;
	 }
    }
  if (have_pop)
    _mail_pop_check_mail (inst);
  if (have_imap)
    _mail_imap_check_mail (inst);
  if (have_mbox)
    _mail_mbox_check_mail (inst);
  return gcc;
}

static void
_gc_shutdown (E_Gadcon_Client * gcc)
{
  Instance *inst;

  inst = gcc->data;

  if (inst->check_timer)
    ecore_timer_del (inst->check_timer);
   
  evas_object_event_callback_del (inst->mail_obj, EVAS_CALLBACK_MOUSE_DOWN,
				  _mail_cb_mouse_down);
  evas_object_event_callback_del (inst->mail_obj, EVAS_CALLBACK_MOUSE_IN,
				  _mail_cb_mouse_in);
  evas_object_event_callback_del (inst->mail_obj, EVAS_CALLBACK_MOUSE_OUT,
				  _mail_cb_mouse_out);

  if (inst->popup) e_object_del (E_OBJECT (inst->popup));
  mail_config->instances = eina_list_remove (mail_config->instances, inst);
  _mail_free (inst->mail);
  free (inst);
  inst = NULL;
}

static void
_gc_orient (E_Gadcon_Client * gcc, E_Gadcon_Orient orient)
{
  e_gadcon_client_aspect_set (gcc, 16, 16);
  e_gadcon_client_min_size_set (gcc, 16, 16);
}

static const char *
_gc_label (const E_Gadcon_Client_Class *client_class)
{
  return D_("Mail");
}

static Evas_Object *
_gc_icon (const E_Gadcon_Client_Class *client_class, Evas * evas)
{
  Evas_Object *o;
  char buf[4096];

  o = edje_object_add (evas);
  snprintf (buf, sizeof (buf), "%s/e-module-mail.edj",
	    e_module_dir_get (mail_config->module));
  edje_object_file_set (o, buf, "icon");
  return o;
}

static const char *
_gc_id_new (const E_Gadcon_Client_Class *client_class)
{
   Config_Item *ci;

   ci = _mail_config_item_get (NULL);
   return ci->id;
}

static void
_mail_cb_mouse_down (void *data, Evas * e, Evas_Object * obj,
		     void *event_info)
{
  Instance *inst = data;
  Evas_Event_Mouse_Down *ev = event_info;
  Eina_List *l;

  if (!inst)
    return;
  if ((ev->button == 3) && (!mail_config->menu))
    {
      E_Menu *m;
      E_Menu_Item *mi;
      int x, y, w, h;
      char buf[1024];

      m = e_menu_new ();
      mi = e_menu_item_new (m);
      e_menu_item_label_set (mi, D_("Settings"));
      e_util_menu_item_theme_icon_set(mi, "preferences-system");
      e_menu_item_callback_set (mi, _mail_menu_cb_configure, inst);

      m = e_gadcon_client_util_menu_items_append (inst->gcc, m, 0);
      e_menu_post_deactivate_callback_set (m, _mail_menu_cb_post, inst);
      mail_config->menu = m;

      if ((inst->ci->boxes) && (eina_list_count (inst->ci->boxes) > 0))
	{
	   mi = NULL;
	   snprintf (buf, sizeof (buf), "%s/module.edj",
		     e_module_dir_get (mail_config->module));

	   for (l = inst->ci->boxes; l; l = l->next)
	     {
		Config_Box *cb;

		cb = l->data;
		if (!cb)
		  continue;
		mi = e_menu_item_new_relative (m, mi);
		snprintf (buf, sizeof (buf), "%s: %d/%d", cb->name, cb->num_new,
			  cb->num_total);
		e_menu_item_label_set (mi, buf);
		if ((cb->exec) && (cb->use_exec))
		  e_menu_item_callback_set (mi, _mail_menu_cb_exec, cb);
	     }
	   if (mi)
	     {
		mi = e_menu_item_new_relative(m, mi);
		e_menu_item_separator_set(mi, 1); 
	     }
	}

      e_gadcon_canvas_zone_geometry_get (inst->gcc->gadcon, &x, &y, &w, &h);
      e_menu_activate_mouse (m,
			     e_util_zone_current_get (e_manager_current_get
						      ()), x + ev->output.x,
			     y + ev->output.y, 1, 1,
			     E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
      evas_event_feed_mouse_up (inst->gcc->gadcon->evas, ev->button,
				EVAS_BUTTON_NONE, ev->timestamp, NULL);
    }
  else if (ev->button == 1)
    _mail_cb_check (inst);
}

static void
_mail_cb_mouse_in (void *data, Evas * e, Evas_Object * obj, void *event_info)
{
  Instance    *inst = data;
  Evas_Object *list;
  Eina_List   *l;
  char         buf[256];
  char path[PATH_MAX];

  if (!inst)
    return;
  edje_object_signal_emit (inst->mail_obj, "label_active", "");

  if (inst->popup) return;
  if ((!inst->ci->show_popup) || (!inst->ci->boxes)) return;

  inst->popup = e_gadcon_popup_new (inst->gcc);
  snprintf (path, sizeof (path), "%s/mail.edj",
	    e_module_dir_get (mail_config->module));
  list = e_ilist_add (inst->popup->win->evas);
  for (l = inst->ci->boxes; l; l = l->next)
    {
       Config_Box *cb;

       cb = l->data;
       if (!cb) continue;
       if ((!inst->ci->show_popup_empty) && (!cb->num_new)) continue;
       snprintf (buf, sizeof (buf), "%s: %d/%d", cb->name, cb->num_new,
	         cb->num_total);
       e_ilist_append (list, NULL, NULL, buf, 0, NULL, NULL, NULL, NULL);
    }
  if (e_ilist_count (list))
    {
       Evas_Coord mw, mh;

       e_ilist_size_min_get (list, &mw, &mh);
       evas_object_size_hint_min_set(list, mw, mh);
       e_gadcon_popup_content_set (inst->popup, list);
       e_gadcon_popup_show (inst->popup);
    }
  else
    {
       e_object_del (E_OBJECT (inst->popup));
       inst->popup = NULL;
    }
}

static void
_mail_cb_mouse_out (void *data, Evas * e, Evas_Object * obj, void *event_info)
{
  Instance *inst = data;

  if (!inst)
    return;
  if (!inst->ci->show_label)
    edje_object_signal_emit (inst->mail_obj, "label_passive", "");

  if (inst->popup)
    {
       e_object_del (E_OBJECT (inst->popup));
       inst->popup = NULL;
    }
}

static void
_mail_menu_cb_post (void *data, E_Menu * m)
{
  if (!mail_config->menu)
    return;
  e_object_del (E_OBJECT (mail_config->menu));
  mail_config->menu = NULL;
}

static void
_mail_menu_cb_configure (void *data, E_Menu * m, E_Menu_Item * mi)
{
  Instance *inst = data;

  if (!inst)
    return;
  _config_mail_module (inst->ci);
}

static Config_Item *
_mail_config_item_get (const char *id)
{
   Eina_List *l;
   Config_Item *ci;
   char buf[128];

   if (!id)
     {
	int  num = 0;

	/* Create id */
	if (mail_config->items)
	  {
	     const char *p;
	     ci = eina_list_last (mail_config->items)->data;
	     p = strrchr (ci->id, '.');
	     if (p) num = atoi (p + 1) + 1;
	  }
	snprintf (buf, sizeof (buf), "%s.%d", _gc_class.name, num);
	id = buf;
     }
   else
     {
	for (l = mail_config->items; l; l = l->next)
	  {
	     ci = l->data;
	     if (!ci->id)
	       continue;
	     if (!strcmp (ci->id, id))
	       return ci;
	  }
     }

   ci = E_NEW (Config_Item, 1);
   ci->id = eina_stringshare_add (id);
   ci->show_label = 1;
   ci->check_time = 15.0;
   ci->show_popup = 1;
   ci->show_popup_empty = 0;
   ci->boxes = NULL;

   mail_config->items = eina_list_append (mail_config->items, ci);
   return ci;
}

EAPI E_Module_Api e_modapi = {
  E_MODULE_API_VERSION,
  "Mail"
};

EAPI void *
e_modapi_init (E_Module * m)
{
  char buf[4096];

  snprintf (buf, sizeof (buf), "%s/locale", e_module_dir_get (m));
  bindtextdomain (PACKAGE, buf);
  bind_textdomain_codeset (PACKAGE, "UTF-8");

  conf_box_edd = E_CONFIG_DD_NEW ("Mail_Box_Config", Config_Box);
#undef T
#undef D
#define T Config_Box
#define D conf_box_edd
  E_CONFIG_VAL (D, T, name, STR);
  E_CONFIG_VAL (D, T, type, INT);
  E_CONFIG_VAL (D, T, port, INT);
  E_CONFIG_VAL (D, T, monitor, UCHAR);
  E_CONFIG_VAL (D, T, ssl, UCHAR);
  E_CONFIG_VAL (D, T, local, UCHAR);   
  E_CONFIG_VAL (D, T, host, STR);
  E_CONFIG_VAL (D, T, user, STR);
  E_CONFIG_VAL (D, T, pass, STR);
  E_CONFIG_VAL (D, T, new_path, STR);
  E_CONFIG_VAL (D, T, cur_path, STR);
  E_CONFIG_VAL (D, T, use_exec, UCHAR);
  E_CONFIG_VAL (D, T, exec, STR);

  conf_item_edd = E_CONFIG_DD_NEW ("Mail_Config_Item", Config_Item);
#undef T
#undef D
#define T Config_Item
#define D conf_item_edd
  E_CONFIG_VAL (D, T, id, STR);
  E_CONFIG_VAL (D, T, show_label, UCHAR);
  E_CONFIG_VAL (D, T, check_time, DOUBLE);
  E_CONFIG_VAL (D, T, show_popup, UCHAR);
  E_CONFIG_VAL (D, T, show_popup_empty, UCHAR);
  E_CONFIG_LIST (D, T, boxes, conf_box_edd);

  conf_edd = E_CONFIG_DD_NEW ("Mail_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
  E_CONFIG_LIST (D, T, items, conf_item_edd);

  mail_config = e_config_domain_load ("module.mail", conf_edd);
  if (!mail_config)
    {
      Config_Item *ci;

      mail_config = E_NEW (Config, 1);
      ci = E_NEW (Config_Item, 1);

      ci->id = eina_stringshare_add ("0");
      ci->show_label = 1;
      ci->check_time = 15.0;
      ci->show_popup = 1;
      ci->show_popup_empty = 0;
      ci->boxes = NULL;

      mail_config->items = eina_list_append (mail_config->items, ci);
    }

  mail_config->module = m;
  e_gadcon_provider_register (&_gc_class);
  return m;
}

EAPI int
e_modapi_shutdown (E_Module * m)
{
  mail_config->module = NULL;
  e_gadcon_provider_unregister (&_gc_class);

  if (exit_handler)
    ecore_event_handler_del (exit_handler);

  if (mail_config->config_dialog)
    e_object_del (E_OBJECT (mail_config->config_dialog));
  if (mail_config->menu)
    {
      e_menu_post_deactivate_callback_set (mail_config->menu, NULL, NULL);
      e_object_del (E_OBJECT (mail_config->menu));
      mail_config->menu = NULL;
    }
  while (mail_config->items)
    {
      Config_Item *ci;

      ci = mail_config->items->data;
      while (ci->boxes)
	{
	  Config_Box *cb;

	  cb = ci->boxes->data;
	  switch (cb->type)
	    {
	    case MAIL_TYPE_IMAP:
	      _mail_imap_del_mailbox (cb);
	      break;
	    case MAIL_TYPE_POP:
	      _mail_pop_del_mailbox (cb);
	      break;
	    case MAIL_TYPE_MDIR:
	      _mail_mdir_del_mailbox (cb);
	      break;
	    case MAIL_TYPE_MBOX:
	      _mail_mbox_del_mailbox (cb);
	      break;
	    }
	  if (cb->name)
	    eina_stringshare_del (cb->name);
	  if (cb->host)
	    eina_stringshare_del (cb->host);
	  if (cb->user)
	    eina_stringshare_del (cb->user);
	  if (cb->pass)
	    eina_stringshare_del (cb->pass);
	  if (cb->new_path)
	    eina_stringshare_del (cb->new_path);
	  if (cb->cur_path)
	    eina_stringshare_del (cb->cur_path);
	  if (cb->exec)
	    eina_stringshare_del (cb->exec);
	  ci->boxes = eina_list_remove_list (ci->boxes, ci->boxes);
	  free (cb);
	  cb = NULL;
	}
      if (ci->id)
	eina_stringshare_del (ci->id);
      mail_config->items =
	eina_list_remove_list (mail_config->items, mail_config->items);
      free (ci);
      ci = NULL;
    }
  _mail_imap_shutdown ();
  _mail_pop_shutdown ();
  _mail_mdir_shutdown ();
  _mail_mbox_shutdown ();
  free (mail_config);
  mail_config = NULL;
  E_CONFIG_DD_FREE (conf_box_edd);
  E_CONFIG_DD_FREE (conf_item_edd);
  E_CONFIG_DD_FREE (conf_edd);
  return 1;
}

EAPI int
e_modapi_save (E_Module * m)
{
  e_config_domain_save ("module.mail", conf_edd, mail_config);
  return 1;
}

static Mail *
_mail_new (Evas * evas)
{
  Mail *mail;
  char buf[4096];

  mail = E_NEW (Mail, 1);
  mail->mail_obj = edje_object_add (evas);

  snprintf (buf, sizeof (buf), "%s/mail.edj",
	    e_module_dir_get (mail_config->module));
  if (!e_theme_edje_object_set
      (mail->mail_obj, "base/theme/modules/mail", "modules/mail/main"))
    edje_object_file_set (mail->mail_obj, buf, "modules/mail/main");
  evas_object_show (mail->mail_obj);

  return mail;
}

static void
_mail_free (Mail * mail)
{
  evas_object_del (mail->mail_obj);
  free (mail);
  mail = NULL;
}

static Eina_Bool
_mail_cb_check (void *data)
{
  Instance *inst = data;
  Eina_List *l;
  int have_imap = 0, have_pop = 0, have_mbox = 0;

  if (!inst) return EINA_TRUE;

  for (l = inst->ci->boxes; l; l = l->next)
    {
      Config_Box *cb;

      cb = l->data;
      if (!cb)
	continue;
      switch (cb->type)
	{
	case MAIL_TYPE_MDIR:
	  break;
	case MAIL_TYPE_MBOX:
	  have_mbox = 1;
	  break;
	case MAIL_TYPE_POP:
	  have_pop = 1;
	  break;
	case MAIL_TYPE_IMAP:
	  have_imap = 1;
	  break;
	}
    }

  if ((have_imap) || (have_pop) || (have_mbox))
     edje_object_signal_emit (inst->mail->mail_obj, "check_mail", "");
     
   if (have_imap)
     _mail_imap_check_mail (inst);
   if (have_pop)
     _mail_pop_check_mail (inst);     
   if (have_mbox)
     _mail_mbox_check_mail (inst);
   return EINA_TRUE;
}

void
_mail_set_text (void *data)
{
  Instance *inst = data;
  Eina_List *l;
  char buf[1024];
  int count = 0;

  if (!inst)
    return;

  for (l = inst->ci->boxes; l; l = l->next)
    {
       Config_Box *cb;

       cb = l->data;
       if (!cb)
	 continue;
       count += cb->num_new;
    }
 
  if (count > 0)
    {
      snprintf (buf, sizeof (buf), "%d", count);
      edje_object_part_text_set (inst->mail->mail_obj, "new_label", buf);
      edje_object_signal_emit (inst->mail->mail_obj, "new_mail", "");
    }
  else
    {
       edje_object_part_text_set (inst->mail->mail_obj, "new_label", "");
       edje_object_signal_emit (inst->mail->mail_obj, "no_mail", "");
    }
}

void
_mail_start_exe (void *data)
{
  Config_Box *cb;

  cb = data;
  if (!cb)
    return;

  exit_handler =
    ecore_event_handler_add (ECORE_EXE_EVENT_DEL, _mail_cb_exe_exit, cb);
  cb->exe = ecore_exe_run (cb->exec, cb);
}

static Eina_Bool
_mail_cb_exe_exit (void *data, int type, void *event)
{
  Config_Box *cb;

  cb = data;
  if (!cb) return EINA_FALSE;
  cb->exe = NULL;
  ecore_event_handler_del (exit_handler);
  return EINA_FALSE;
}

void
_mail_box_added (Config_Item *ci, const char *box_name)
{
  Eina_List *l, *b;

  for (l = mail_config->instances; l; l = l->next)
    {
      Instance *inst;

      inst = l->data;
      if (inst->ci != ci) continue;
      for (b = ci->boxes; b; b = b->next)
	{
	   Config_Box *cb;

	   cb = b->data;
	   if ((cb->name) && (!strcmp (cb->name, box_name)))
	     {
		switch (cb->type)
		  {
		   case MAIL_TYPE_IMAP:
		      _mail_imap_add_mailbox (cb);
		      break;
		   case MAIL_TYPE_POP:
		      _mail_pop_add_mailbox (cb);
		      break;
		   case MAIL_TYPE_MDIR:
		      _mail_mdir_add_mailbox (inst, cb);
		      break;
		   case MAIL_TYPE_MBOX:
		      _mail_mbox_add_mailbox (inst, cb);
		      break;
		  }
		break;
	     }
	}
    }
}

void
_mail_box_deleted (Config_Item *ci, const char *box_name)
{
  Eina_List *d, *i;
  Config_Box *cb;

  for (i = mail_config->instances; i; i = i->next)
    {
       Instance *inst;
       int found = 0;

       inst = i->data;
       if (inst->ci != ci) continue;
       for (d = ci->boxes; d; d = d->next)
	 {
	    cb = d->data;
	    if ((cb->name) && (box_name))
	      {
		 if (!strcmp (cb->name, box_name))
		   {
		      found = 1;
		      break;
		   }
	      }
	 }
       if (found)
	 {
	    switch (cb->type)
	      {
	       case MAIL_TYPE_IMAP:
		  _mail_imap_del_mailbox (cb);
		  break;
	       case MAIL_TYPE_POP:
		  _mail_pop_del_mailbox (cb);
		  break;
	       case MAIL_TYPE_MDIR:
		  _mail_mdir_del_mailbox (cb);
		  break;
	       case MAIL_TYPE_MBOX:
		  _mail_mbox_del_mailbox (cb);
		  break;
	      }
	    ci->boxes = eina_list_remove (ci->boxes, cb);
	    e_config_save_queue ();
	    break;
	 }
    }
}

void
_mail_config_updated (Config_Item *ci)
{
  Eina_List *l;

  if (!mail_config)
    return;

  for (l = mail_config->instances; l; l = l->next)
    {
      Instance *inst;

      inst = l->data;
      if (inst->ci != ci) continue;
      if (inst->check_timer)
	{
	   ecore_timer_del (inst->check_timer);
	   inst->check_timer =
	      ecore_timer_add ((ci->check_time * 60.0), _mail_cb_check,
		    inst);
	}

      if (ci->show_label)
	edje_object_signal_emit (inst->mail_obj, "label_active", "");
      else
	edje_object_signal_emit (inst->mail_obj, "label_passive", "");
      break;
    }
}

static void
_mail_menu_cb_exec (void *data, E_Menu * m, E_Menu_Item * mi)
{
  Config_Box *cb;

  cb = data;
  if (!cb)
    return;

  exit_handler =
    ecore_event_handler_add (ECORE_EXE_EVENT_DEL, _mail_cb_exe_exit, cb);
  cb->exe = ecore_exe_run (cb->exec, cb);
}
