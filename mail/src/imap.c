#include <e.h>
#include "e_mod_main.h"
#include "imap.h"

#ifdef PRINT_LOTS_OF_DEBUG
static void
tls_log_func(int level, const char *str)
{
   fprintf(stderr, "|<%d>| %s", level, str);
}
#endif

static ImapServer *_mail_imap_server_find (Ecore_Con_Server *server);
static ImapServer *_mail_imap_server_get (Config_Box *cb);
static ImapClient *_mail_imap_client_get (Config_Box *cb);
static Eina_Bool _mail_imap_server_add (void *data, int type, void *event);
static Eina_Bool _mail_imap_server_del (void *data, int type, void *event);
static Eina_Bool _mail_imap_server_data (void *data, int type, void *event);
static void _mail_imap_server_logout (ImapServer * is);

static Eina_List *iservers = NULL;

void
_mail_imap_check_mail (void *data)
{
   Ecore_Con_Type type;
   Eina_List *l;
   printf("check mail!!!!\n");

  for (l = iservers; l; l = l->next)
    {
      ImapServer *is;
      printf("1\n");

      is = l->data;
      is->data = data;
      if (!is->server)
	{
	   printf("2\n");
	  if (!is->add_handler)
	    is->add_handler =
	      ecore_event_handler_add (ECORE_CON_EVENT_SERVER_ADD,
				       _mail_imap_server_add, NULL);
	  if (!is->del_handler)
	    is->del_handler =
	      ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DEL,
				       _mail_imap_server_del, NULL);
	  if (!is->data_handler)
	    is->data_handler =
	      ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DATA,
				       _mail_imap_server_data, NULL);

	  if (is->local)
	    type = ECORE_CON_LOCAL_SYSTEM;
	  else
	    type = ECORE_CON_REMOTE_NODELAY;

	  if (ecore_con_ssl_available_get () && (is->ssl))
            {
	       type |= ECORE_CON_USE_SSL;
#ifdef PRINT_LOTS_OF_DEBUG
               if (ecore_con_ssl_available_get() == 1)
                 {
                    gnutls_global_set_log_level(9);
                    gnutls_global_set_log_function(tls_log_func);
                 }
            }
#endif
           }
	  is->state = IMAP_STATE_DISCONNECTED;
	  is->server =
	     ecore_con_server_connect (type, is->host,
		   is->port, NULL);
	  is->cmd = 0;
	  is->current = is->clients;
	}
    }
}

void
_mail_imap_add_mailbox (void *data)
{
  ImapClient *ic;
  Config_Box *cb;

  cb = data;
  if (!cb)
    return;
  ic = _mail_imap_client_get (cb);
}

void
_mail_imap_del_mailbox (void *data)
{
  ImapClient *ic;
  Config_Box *cb;

  cb = data;
  if (!cb)
    return;
  ic = _mail_imap_client_get (cb);
  ic->server->clients = eina_list_remove (ic->server->clients, ic);
}

void
_mail_imap_shutdown ()
{
  if (!iservers)
    return;
  while (iservers)
    {
      ImapServer *is;

      is = iservers->data;
      if (is->add_handler)
	ecore_event_handler_del (is->add_handler);
      if (is->del_handler)
	ecore_event_handler_del (is->del_handler);
      if (is->data_handler)
	ecore_event_handler_del (is->data_handler);
      iservers = eina_list_remove_list (iservers, iservers);
       E_FREE(is);
    }
}

/* PRIVATES */
static ImapServer *
_mail_imap_server_find (Ecore_Con_Server * server)
{
  Eina_List *l;

  for (l = iservers; l; l = l->next)
    {
      ImapServer *is;

      is = l->data;
      if (is->server == server)
	return is;
    }
  return NULL;
}

static ImapServer *
_mail_imap_server_get (Config_Box *cb)
{
   ImapServer *is = NULL;
   Eina_List *l = NULL;

   for (l = iservers; l; l = l->next)
     {
	ImapServer *curr;

	curr = l->data;
	if ((curr->local == cb->local) && (curr->port == cb->port) && (curr->ssl == cb->ssl) &&
	    (!strcmp(curr->host, cb->host)) &&
	    (!strcmp(curr->user, cb->user)) &&
	    (!strcmp(curr->pass, cb->pass)))
	  {
	     is = curr;
	     break;
	  }
     }
  if (!is)
    {
      is = E_NEW (ImapServer, 1);
      is->local = cb->local;
      is->port = cb->port;
      is->ssl = cb->ssl;
      is->host = cb->host;
      is->user = cb->user;
      is->pass = cb->pass;

      is->server = NULL;
      is->cmd = 0;
      is->state = IMAP_STATE_DISCONNECTED;
      iservers = eina_list_append (iservers, is);
    }
  return is;
}

static ImapClient *
_mail_imap_client_get (Config_Box *cb)
{
  ImapServer *is;
  ImapClient *ic;
  Eina_List *l;
  int found = 0;

  if (!cb)
    return NULL;

  is = _mail_imap_server_get(cb);

  for (l = is->clients; l; l = l->next)
    {
       ic = l->data;
       if (!ic->config)
	 continue;
       if (!strcmp (ic->config->new_path, cb->new_path))
	 {
	    found = 1;
	    break;
	 }
    }
  if (!found)
    {
      ic = E_NEW (ImapClient, 1);
      ic->config = cb;
      ic->server = is;
      ic->config->num_new = 0;
      ic->config->num_total = 0;
      ic->server->clients = eina_list_append (ic->server->clients, ic);
    }
  return ic;
}

static Eina_Bool
_mail_imap_server_add (void *data, int type, void *event)
{
  Ecore_Con_Event_Server_Add *ev = event;
  ImapServer *is;

  is = _mail_imap_server_find (ev->server);
  if (!is)
    return EINA_TRUE;

  is->state = IMAP_STATE_CONNECTED;
  is->cmd = 0;
  return EINA_FALSE;
}

static Eina_Bool
_mail_imap_server_del (void *data, int type, void *event)
{
  Ecore_Con_Event_Server_Del *ev = event;
  ImapServer *is;

  is = _mail_imap_server_find (ev->server);
  if (!is)
    return EINA_TRUE;

  if (is->state == IMAP_STATE_DISCONNECTED)
    printf ("Imap Server Disconnected\n");
  else
    is->state = IMAP_STATE_DISCONNECTED;

  ecore_con_server_del (is->server);
  is->server = NULL;

  _mail_set_text (is->data);
  return EINA_FALSE;
}

static Eina_Bool
_mail_imap_server_data (void *data, int type, void *event)
{
  Ecore_Con_Event_Server_Data *ev = event;
  ImapServer *is;
  ImapClient *ic;
  int len, num = 0, total = 0;
  char in[1024], out[1024], *spc;
  size_t slen;

  is = _mail_imap_server_find (ev->server);
  if (!is)
    return EINA_TRUE;
  if (is->state == IMAP_STATE_DISCONNECTED)
    return EINA_TRUE;

  len = sizeof (in) - 1;
  len = (len > ev->size) ? (ev->size) : (len);

  memcpy (in, ev->data, len);
  in[len] = 0;

  if ((spc = strchr (in, ' ')))
    {
      slen = strlen (spc);
      if ((slen > 5) && (!strncmp (spc + 1, "NO ", 3)))
	{
	  _mail_imap_server_logout (is);
	  printf ("Imap Failure: %s\n", spc + 4);
	  return EINA_FALSE;
	}
      else if ((slen > 6) && (!strncmp (spc + 1, "BAD ", 4)))
	{
	  _mail_imap_server_logout (is);
	  printf ("Imap Bad Command: %s\n", spc + 5);
	  return EINA_FALSE;
	}
    }

  if (!is->current) return EINA_FALSE;
  ic = is->current->data;
  is->state++;

  switch (is->state)
    {
    case IMAP_STATE_SERVER_READY:
      len =
	snprintf (out, sizeof (out), "A%03i LOGIN %s %s\r\n", ++is->cmd,
		  is->user, is->pass);
      ecore_con_server_send (ev->server, out, len);
      break;
    case IMAP_STATE_STATUS_OK:
      if (sscanf (in, "* STATUS %*s (MESSAGES %i UNSEEN %i)", &total, &num) == 2)
	{
	  ic->config->num_new = num;
	  ic->config->num_total = total;
	  _mail_set_text (is->data);

	  if ((num > 0) && (ic->config->use_exec) && (ic->config->exec))
	    _mail_start_exe (ic->config);
	}

      ic = NULL;
      is->current = is->current->next;
      if (is->current)
	{
	   is->state = IMAP_STATE_LOGGED_IN;
	   ic = is->current->data;
	}
      else
	_mail_imap_server_logout (is);
      /* Fall through if we have another mailbox to check */
      if (!ic)
	break;
    case IMAP_STATE_LOGGED_IN:
      len =
	snprintf (out, sizeof (out), "A%03i STATUS %s (MESSAGES UNSEEN)\r\n",
		  ++is->cmd, ic->config->new_path);
      ecore_con_server_send (ev->server, out, len);
      break;
    default:
      break;
    }
  return EINA_FALSE;
}

static void
_mail_imap_server_logout (ImapServer * is)
{
  int len;
  char out[1024];

  if (!is)
    return;

  if (is->state >= IMAP_STATE_LOGGED_IN)
    {
      len = snprintf (out, sizeof (out), "A%03i LOGOUT", ++is->cmd);
      ecore_con_server_send (is->server, out, len);
    }
  ecore_con_server_del (is->server);
  is->server = NULL;
  is->state = IMAP_STATE_DISCONNECTED;
}
