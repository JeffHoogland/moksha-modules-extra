#include <e.h>
#include "e_mod_main.h"
#include "mbox.h"

struct _E_Config_Dialog_Data
{
  char *name;
  int type;
  int use_exec;
  char *port;
  int monitor;
  int ssl;
  int ssl_version;
  int local;
  char *host;
  char *user;
  char *pass;
  char *new_path;
  char *cur_path;
  char *exec;

  /* Evas_Object References */
  Evas_Object *exec_label;
  Evas_Object *exec_entry;
  Evas_Object *port_entry;
  Evas_Object *new_path_label;
  Evas_Object *new_path_entry;
  Evas_Object *cur_path_label;
  Evas_Object *cur_path_entry;
  Evas_Object *monitor_check;
  Evas_Object *sslv2;
  Evas_Object *sslv3;
};

static void *_create_data (E_Config_Dialog * cfd);
static void _free_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata);
static Evas_Object *_basic_create_widgets (E_Config_Dialog * cfd, Evas * evas,
					   E_Config_Dialog_Data * cfdata);
static int _basic_apply_data (E_Config_Dialog * cfd,
			      E_Config_Dialog_Data * cfdata);
static void _type_cb_change (void *data, Evas_Object * obj);
static void _use_exec_cb_change (void *data, Evas_Object * obj);
static void _monitor_cb_change (void *data, Evas_Object * obj);

static E_Config_Dialog *prev_dlg;
static Config_Item *mail_ci;

void
_config_box (Config_Item * ci, Config_Box * cb, E_Config_Dialog * mcfd)
{
  E_Config_Dialog *cfd;
  E_Config_Dialog_View *v;
  E_Container *con;

  v = E_NEW (E_Config_Dialog_View, 1);
  if (!v)
    return;

  prev_dlg = mcfd;
  mail_ci = ci;

  v->create_cfdata = _create_data;
  v->free_cfdata = _free_data;
  v->basic.apply_cfdata = _basic_apply_data;
  v->basic.create_widgets = _basic_create_widgets;

  con = e_container_current_get (e_manager_current_get ());
  cfd = e_config_dialog_new (con, D_("Mailbox Configuration"), "Mail", "_e_modules_mail_box_config_dialog", NULL, 0, v, cb);
}

static void
_fill_data (Config_Box * cb, E_Config_Dialog_Data * cfdata)
{
  char buf[1024];

  if (!cb)
    {
      cfdata->type = 0;
      cfdata->monitor = 1;
      cfdata->ssl = 0;
      cfdata->ssl_version = 2;
      cfdata->use_exec = 0;
      cfdata->local = 0;
      snprintf (buf, sizeof (buf), "110");
      cfdata->port = strdup (buf);
      return;
    }

  if (cb->name)
    cfdata->name = strdup (cb->name);

  cfdata->type = cb->type;
  cfdata->monitor = cb->monitor;
  if (cb->ssl)
    {
      cfdata->ssl = 1;
      cfdata->ssl_version = cb->ssl;
    }
  else
    {
      cfdata->ssl = 0;
      cfdata->ssl_version = 2;
    }
  E_CONFIG_LIMIT(cfdata->ssl_version, 2, 3);
  cfdata->use_exec = cb->use_exec;
  cfdata->local = cb->local;
   
  snprintf (buf, sizeof (buf), "%d", cb->port);
  cfdata->port = strdup (buf);

  if (cb->exec)
    cfdata->exec = strdup (cb->exec);
  if (cb->host)
    cfdata->host = strdup (cb->host);
  if (cb->user)
    cfdata->user = strdup (cb->user);
  if (cb->pass)
    cfdata->pass = strdup (cb->pass);

  if (cb->new_path)
    cfdata->new_path = strdup (cb->new_path);
  if (cb->cur_path)
    cfdata->cur_path = strdup (cb->cur_path);
}

static void *
_create_data (E_Config_Dialog * cfd)
{
  E_Config_Dialog_Data *cfdata;
  Config_Box *cb;

  cb = cfd->data;
  cfdata = E_NEW (E_Config_Dialog_Data, 1);
  _fill_data (cb, cfdata);
  return cfdata;
}

static void
_free_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
   E_FREE(cfdata->name);
   E_FREE(cfdata->port);
   E_FREE(cfdata->host);
   E_FREE(cfdata->user);
   E_FREE(cfdata->pass);
   E_FREE(cfdata->new_path);
   E_FREE(cfdata->cur_path);
   E_FREE(cfdata->exec);
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create_widgets (E_Config_Dialog * cfd, Evas * evas,
		       E_Config_Dialog_Data * cfdata)
{
  Evas_Object *o, *of, *ob, *ot;
  E_Radio_Group *rg;

  o = e_widget_list_add (evas, 0, 0);
  of = e_widget_framelist_add (evas, D_("General Settings"), 0);
  ob =
    e_widget_check_add (evas, D_("Start Program When New Mail Arrives"),
			&(cfdata->use_exec));
  e_widget_framelist_object_append (of, ob);
  e_widget_on_change_hook_set (ob, _use_exec_cb_change, cfdata);
  ot = e_widget_table_add (evas, 1);
  cfdata->exec_label = e_widget_label_add (evas, D_("Program:"));
  e_widget_table_object_append (ot, cfdata->exec_label, 0, 0, 1, 1, 0, 0, 1,
				0);
  cfdata->exec_entry = e_widget_entry_add (evas, &cfdata->exec, NULL, NULL, NULL);
  e_widget_table_object_append (ot, cfdata->exec_entry, 1, 0, 1, 1, 0, 0, 1,
				0);
  e_widget_framelist_object_append (of, ot);
  e_widget_list_object_append (o, of, 1, 1, 0.5);

  e_widget_disabled_set (cfdata->exec_label, !cfdata->use_exec);
  e_widget_disabled_set (cfdata->exec_entry, !cfdata->use_exec);

  of = e_widget_framelist_add (evas, D_("Mailbox Type"), 0);
  rg = e_widget_radio_group_new (&(cfdata->type));
  ob = e_widget_radio_add (evas, D_("Pop3"), 0, rg);
  e_widget_on_change_hook_set (ob, _type_cb_change, cfdata);
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_radio_add (evas, D_("Imap"), 1, rg);
  e_widget_on_change_hook_set (ob, _type_cb_change, cfdata);
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_radio_add (evas, D_("Maildir"), 2, rg);
  e_widget_on_change_hook_set (ob, _type_cb_change, cfdata);
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_radio_add (evas, D_("Mbox"), 3, rg);
  e_widget_on_change_hook_set (ob, _type_cb_change, cfdata);
  e_widget_framelist_object_append (of, ob); 
  cfdata->monitor_check = e_widget_check_add (evas, D_("Monitor Mbox file permanently"),
			   &(cfdata->monitor));
  e_widget_on_change_hook_set (cfdata->monitor_check, _monitor_cb_change, cfdata);
  if (cfdata->type == 3)
    {
      e_widget_check_checked_set (cfdata->monitor_check, cfdata->monitor);
      e_widget_disabled_set (cfdata->monitor_check, 0);
    }
  else
    {
      e_widget_check_checked_set (cfdata->monitor_check, 0);
      e_widget_disabled_set (cfdata->monitor_check, 1);
    }
  e_widget_framelist_object_append (of, cfdata->monitor_check);

  e_widget_list_object_append (o, of, 1, 1, 0.5); 

  of = e_widget_frametable_add (evas, D_("Port Settings"), 1);

  ob = e_widget_label_add (evas, D_("Use SSL:"));
  e_widget_frametable_object_append (of, ob, 0, 0, 1, 1, 0, 0, 1, 0);
  ob = e_widget_check_add (evas, "", &(cfdata->ssl));
  e_widget_on_change_hook_set (ob, _type_cb_change, cfdata);
  e_widget_frametable_object_append (of, ob, 1, 0, 1, 1, 0, 0, 1, 0);

  rg = e_widget_radio_group_new (&(cfdata->ssl_version));
  cfdata->sslv2 = e_widget_radio_add (evas, D_("v2"), 2, rg);
  if (!cfdata->ssl) e_widget_disabled_set(cfdata->sslv2, 1);
  e_widget_frametable_object_append (of, cfdata->sslv2, 2, 0, 1, 1, 0, 0, 1, 0);
  cfdata->sslv3 = e_widget_radio_add (evas, D_("v3"), 3, rg);
  if (!cfdata->ssl) e_widget_disabled_set(cfdata->sslv3, 1);
  e_widget_frametable_object_append (of, cfdata->sslv3, 3, 0, 1, 1, 0, 0, 1, 0);

  ob = e_widget_label_add (evas, D_("Port:"));
  e_widget_frametable_object_append (of, ob, 0, 1, 1, 1, 0, 0, 1, 0);
  ob = e_widget_entry_add (evas, &cfdata->port, NULL, NULL, NULL);
  cfdata->port_entry = ob;
  e_widget_frametable_object_append (of, ob, 1, 1, 3, 1, 0, 0, 1, 0);

  ob = e_widget_label_add (evas, D_("Local:"));
  e_widget_frametable_object_append (of, ob, 0, 2, 1, 1, 0, 0, 1, 0);
  ob = e_widget_check_add (evas, "", &(cfdata->local));
  e_widget_frametable_object_append (of, ob, 1, 2, 3, 1, 0, 0, 1, 0);   
   e_widget_list_object_append (o, of, 1, 1, 0.5);
   
  of = e_widget_frametable_add (evas, D_("Mailbox Settings"), 1);
  ob = e_widget_label_add (evas, D_("Name:"));
  e_widget_frametable_object_append (of, ob, 0, 0, 1, 1, 0, 0, 1, 0);
  ob = e_widget_entry_add (evas, &cfdata->name, NULL, NULL, NULL);
  e_widget_frametable_object_append (of, ob, 1, 0, 1, 1, 0, 0, 1, 0);

  ob = e_widget_label_add (evas, D_("Mail Host:"));
  e_widget_frametable_object_append (of, ob, 0, 1, 1, 1, 0, 0, 1, 0);
  ob = e_widget_entry_add (evas, &cfdata->host, NULL, NULL, NULL);
  e_widget_frametable_object_append (of, ob, 1, 1, 1, 1, 0, 0, 1, 0);

  ob = e_widget_label_add (evas, D_("Username:"));
  e_widget_frametable_object_append (of, ob, 0, 2, 1, 1, 0, 0, 1, 0);
  ob = e_widget_entry_add (evas, &cfdata->user, NULL, NULL, NULL);
  e_widget_frametable_object_append (of, ob, 1, 2, 1, 1, 0, 0, 1, 0);

  ob = e_widget_label_add (evas, D_("Password:"));
  e_widget_frametable_object_append (of, ob, 0, 3, 1, 1, 0, 0, 1, 0);
  ob = e_widget_entry_add (evas, &cfdata->pass, NULL, NULL, NULL);
  e_widget_entry_password_set (ob, 1);
  e_widget_frametable_object_append (of, ob, 1, 3, 1, 1, 0, 0, 1, 0);

  cfdata->new_path_label = e_widget_label_add (evas, D_("New Mail Path:"));
  e_widget_frametable_object_append (of, cfdata->new_path_label, 0, 4, 1, 1,
				     0, 0, 1, 0);
  cfdata->new_path_entry = e_widget_entry_add (evas, &cfdata->new_path, NULL, NULL, NULL);
  e_widget_frametable_object_append (of, cfdata->new_path_entry, 1, 4, 1, 1,
				     0, 0, 1, 0);
  if (cfdata->type == 0)
    {
      e_widget_disabled_set (cfdata->new_path_label, 1);
      e_widget_disabled_set (cfdata->new_path_entry, 1);
    }

  cfdata->cur_path_label = e_widget_label_add (evas, D_("Current Mail Path:"));
  e_widget_frametable_object_append (of, cfdata->cur_path_label, 0, 5, 1, 1,
				     0, 0, 1, 0);
  cfdata->cur_path_entry = e_widget_entry_add (evas, &cfdata->cur_path, NULL, NULL, NULL);
  e_widget_frametable_object_append (of, cfdata->cur_path_entry, 1, 5, 1, 1,
				     0, 0, 1, 0);
  if ((cfdata->type == 1) || (cfdata->type == 3))
    {
      e_widget_disabled_set (cfdata->cur_path_label, 1);
      e_widget_disabled_set (cfdata->cur_path_entry, 1);
    }

  e_widget_list_object_append (o, of, 1, 1, 0.5);
  return o;
}

static int
_basic_apply_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
  Config_Box *cb;
  int is_new = 0;

  if (!cfdata->name)
    return 0;

  cb = cfd->data;
  if (!cb)
    {
      cb = E_NEW (Config_Box, 1);
      cb->type = 0;
      cb->port = 110;
      cb->monitor = 1;
      cb->ssl = 0;
      cb->local = 0;
      is_new = 1;
    }

  if (cb->name)
    eina_stringshare_del (cb->name);
  if (cfdata->name)
    cb->name = eina_stringshare_add (cfdata->name);
  else
    cb->name = eina_stringshare_add ("");

  cb->type = cfdata->type;
  cb->port = atoi (cfdata->port);
  cb->monitor = cfdata->monitor;
  if (cfdata->ssl)
    cb->ssl = cfdata->ssl_version;
  else
    cb->ssl = 0;
  cb->local = cfdata->local;
   
  cb->use_exec = cfdata->use_exec;

  if (cb->exec)
    eina_stringshare_del (cb->exec);
  if (cfdata->exec)
    cb->exec = eina_stringshare_add (cfdata->exec);
  else
    cb->exec = eina_stringshare_add ("");

  if (cb->host)
    eina_stringshare_del (cb->host);
  if (cfdata->host)
    cb->host = eina_stringshare_add (cfdata->host);
  else
    cb->host = eina_stringshare_add ("");

  if (cb->user)
    eina_stringshare_del (cb->user);
  if (cfdata->user)
    cb->user = eina_stringshare_add (cfdata->user);
  else
    cb->user = eina_stringshare_add ("");

  if (cb->pass)
    eina_stringshare_del (cb->pass);
  if (cfdata->pass)
    cb->pass = eina_stringshare_add (cfdata->pass);
  else
    cb->pass = eina_stringshare_add ("");

  if (cb->new_path)
    eina_stringshare_del (cb->new_path);
  if (cfdata->new_path)
    cb->new_path = eina_stringshare_add (cfdata->new_path);
  else
    cb->new_path = eina_stringshare_add ("");

  if (cb->cur_path)
    eina_stringshare_del (cb->cur_path);
  if (cfdata->cur_path)
    cb->cur_path = eina_stringshare_add (cfdata->cur_path);
  else
    cb->cur_path = eina_stringshare_add ("");

  if (!is_new)
    {
      e_config_save_queue ();
      _mail_mbox_check_monitors ();
    }

  if (is_new)
    {
/*       cfd->data = cb; */
      mail_ci->boxes = eina_list_append (mail_ci->boxes, cb);
      e_config_save_queue ();
      _mail_box_added (mail_ci, cb->name);
    }

  _mail_box_config_updated (prev_dlg);
  return 1;
}

static void
_monitor_cb_change (void *data, Evas_Object * obj)
{
  E_Config_Dialog_Data *cfdata;

  cfdata = data;
  if (cfdata->type == MAIL_TYPE_MBOX)
    {
    }
}

static void
_type_cb_change (void *data, Evas_Object * obj)
{
  E_Config_Dialog_Data *cfdata;

  cfdata = data;
  if (cfdata->type == 0)
    {
      e_widget_disabled_set (cfdata->new_path_label, 1);
      e_widget_disabled_set (cfdata->new_path_entry, 1);
      e_widget_disabled_set (cfdata->cur_path_label, 1);
      e_widget_disabled_set (cfdata->cur_path_entry, 1);
      e_widget_entry_text_set (cfdata->new_path_entry, "");
      e_widget_entry_text_set (cfdata->cur_path_entry, "");
      if (cfdata->ssl)
	{
	  e_widget_entry_text_set (cfdata->port_entry, "995");
	  cfdata->port = strdup ("995");
	}
      else
	{
	  e_widget_entry_text_set (cfdata->port_entry, "110");
	  cfdata->port = strdup ("110");
	}
    }
  else if ((cfdata->type == 1) || (cfdata->type == 3))
    {
      e_widget_disabled_set (cfdata->new_path_label, 0);
      e_widget_disabled_set (cfdata->new_path_entry, 0);
      e_widget_disabled_set (cfdata->cur_path_label, 1);
      e_widget_disabled_set (cfdata->cur_path_entry, 1);
      e_widget_entry_text_set (cfdata->cur_path_entry, "");
      e_widget_entry_text_set (cfdata->new_path_entry, D_("Inbox"));
      if (cfdata->ssl)
	{
	  e_widget_entry_text_set (cfdata->port_entry, "993");
	  cfdata->port = strdup ("993");
	}
      else
	{
	  e_widget_entry_text_set (cfdata->port_entry, "143");
	  cfdata->port = strdup ("143");
	}
    }
  else if (cfdata->type == 2)
    {
      e_widget_disabled_set (cfdata->new_path_label, 0);
      e_widget_disabled_set (cfdata->new_path_entry, 0);
      e_widget_disabled_set (cfdata->cur_path_label, 0);
      e_widget_disabled_set (cfdata->cur_path_entry, 0);
      e_widget_entry_text_set (cfdata->port_entry, "");
    }

  if (cfdata->type == 3)
    {
      e_widget_check_checked_set (cfdata->monitor_check, 1);
      e_widget_disabled_set (cfdata->monitor_check, 0);
    }
  else
    {
      e_widget_check_checked_set (cfdata->monitor_check, 0);
      e_widget_disabled_set (cfdata->monitor_check, 1);
    }

  if (cfdata->ssl)
    {
      e_widget_disabled_set (cfdata->sslv2, 0);
      e_widget_disabled_set (cfdata->sslv3, 0);
    }
  else
    {
      e_widget_disabled_set (cfdata->sslv2, 1);
      e_widget_disabled_set (cfdata->sslv3, 1);
    }
}

static void
_use_exec_cb_change (void *data, Evas_Object * obj)
{
  E_Config_Dialog_Data *cfdata;

  cfdata = data;
  e_widget_disabled_set (cfdata->exec_label, !cfdata->use_exec);
  e_widget_disabled_set (cfdata->exec_entry, !cfdata->use_exec);
  if (!cfdata->use_exec)
    e_widget_entry_text_set (cfdata->exec_entry, "");
}
