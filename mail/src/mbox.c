#include <e.h>
#include "e_mod_main.h"
#include "mbox.h"

static Eina_List *mboxes;

static void _mail_mbox_check_mail_parser (Config_Box *cb);
static void _mail_mbox_check_mail_monitor (void *data, Ecore_File_Monitor * monitor,
					   Ecore_File_Event event, const char *path);

void
_mail_mbox_add_mailbox (void *data, void *data2)
{
  MboxClient *mb;
  Config_Box *cb;

  cb = data2;
  if (!cb)
    return;

  mb = E_NEW (MboxClient, 1);
  mb->config = cb;
  mb->data = data;
  mb->config->num_new = 0;
  mb->config->num_total = 0;

  if (mb->config->monitor)
    mb->monitor =
      ecore_file_monitor_add (mb->config->new_path, _mail_mbox_check_mail_monitor, mb);

  mboxes = eina_list_append (mboxes, mb);
}

void
_mail_mbox_check_monitors ()
{
  Eina_List *l;

  for (l = mboxes; l; l = l->next)
    {
      MboxClient *mb;

      mb = l->data;
      if (!mb)
	continue;

      if (mb->config->monitor)
	{      
	  if (!mb->monitor)
	    mb->monitor =
	      ecore_file_monitor_add (mb->config->new_path, _mail_mbox_check_mail_monitor, mb);
	}
      else
	{
	  if (mb->monitor)
	    {
	      ecore_file_monitor_del (mb->monitor);
	    }
	  mb->monitor = NULL;
	}
      break;
    }
}

void
_mail_mbox_del_mailbox (void *data)
{
  Config_Box *cb;
  Eina_List *l;

  cb = data;
  if (!cb)
    return;
  for (l = mboxes; l; l = l->next)
    {
      MboxClient *mb;

      mb = l->data;
      if (!mb)
	continue;
      if (mb->config != cb)
	continue;
      if (mb->monitor)
	ecore_file_monitor_del (mb->monitor);
      mboxes = eina_list_remove (mboxes, mb);
      free (mb);
      mb = NULL;
      break;
    }
}

void
_mail_mbox_shutdown ()
{
  if (!mboxes)
    return;
  while (mboxes)
    {
      MboxClient *mb;

      mb = mboxes->data;
      if (mb->monitor)
	ecore_file_monitor_del (mb->monitor);
      mboxes = eina_list_remove_list (mboxes, mboxes);
      free (mb);
      mb = NULL;
    }
}

void _mail_mbox_check_mail (void *data)
{
  Eina_List *l;
  Instance *inst;
  int num_new_prev;

  inst = data;
  if (!inst)
    return;

  for (l = mboxes; l; l = l->next)
    {
      MboxClient *mb;
      Config_Box *cb;    
  
      mb = l->data;
      if (!mb)
	continue;
      mb->data = inst;

      cb = mb->config;
      if (!cb)
	continue;

      num_new_prev = cb->num_new;
      _mail_mbox_check_mail_parser(cb);
      
      /* Only launch the program if the number of new mails increased.
	 This is hacky but better than launching it every time there's 
	 unread/new mail imho */

      _mail_set_text (mb->data);
      if ((mb->config->num_new > 0) && (mb->config->num_new > num_new_prev) 
	  && (mb->config->use_exec) && (mb->config->exec))
	_mail_start_exe (mb->config);
    }
}

/* PRIVATES */
static void
_mail_mbox_check_mail_monitor (void *data, Ecore_File_Monitor * monitor,
			       Ecore_File_Event event, const char *path)
{
  MboxClient *mb;
  Config_Box *cb;

  mb = data;
  if (!mb)
    return;

  cb = mb->config;
  if (!cb)
    return;

  _mail_mbox_check_mail_parser(cb);

  _mail_set_text (mb->data);
  if ((mb->config->num_new > 0) && (mb->config->use_exec)
      && (mb->config->exec))
    _mail_start_exe (mb->config);
}

static void
_mail_mbox_check_mail_parser (Config_Box *cb)
{
  FILE *f;
  int header = 0;
  char buf[1024];

  if (!cb->new_path)
    return;

  if (!(f = fopen (cb->new_path, "r")))
    return;  

  cb->num_new = 0;
  cb->num_total = 0;

  while (fgets (buf, sizeof (buf), f))
    {
      if (buf[0] == '\n')
        header = 0;
      else if (!strncmp (buf, "From ", 5))
	{
	  header = 1;
	  cb->num_total++;
	  cb->num_new++;
	}
      else if (header)
	{
	  if ((!strncmp (buf, "Status: ", 8)) && (strchr (buf, 'R')))
	    cb->num_new--;
	  /* Support for Mozilla/Thunderbird mbox format */
	  else if (!strncmp (buf, "X-Mozilla-Status: ", 18))
	    {
	      if (!strstr (buf, "0000"))
		{
		  cb->num_new--;
		  if (strstr (buf, "0009"))
		    cb->num_total--;
		}
	    }
	}
    }
  fclose (f);
}
