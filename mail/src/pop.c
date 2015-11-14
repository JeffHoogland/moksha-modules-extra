#include <e.h>
#include "e_mod_main.h"
#include "pop.h"

static Eina_Bool _mail_pop_server_add (void *data, int type, void *event);
static Eina_Bool _mail_pop_server_del (void *data, int type, void *event);
static Eina_Bool _mail_pop_server_data (void *data, int type, void *event);
static PopClient *_mail_pop_client_get (void *data);
static PopClient *_mail_pop_client_get_from_server (void *data);
static void _mail_pop_client_quit (void *data);

static Eina_List *pclients;

void
_mail_pop_check_mail (void *data)
{
  Ecore_Con_Type type = ECORE_CON_REMOTE_NODELAY;
  Eina_List *l;
  Instance *inst;

  inst = data;
  if (!inst)
    return;

  for (l = pclients; l; l = l->next)
    {
      PopClient *pc;

      pc = l->data;
      if (!pc)
	continue;
      pc->data = inst;
      if (!pc->server)
	{
	  if (!pc->add_handler)
	    pc->add_handler =
	      ecore_event_handler_add (ECORE_CON_EVENT_SERVER_ADD,
				       _mail_pop_server_add, NULL);
	  if (!pc->del_handler)
	    pc->del_handler =
	      ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DEL,
				       _mail_pop_server_del, NULL);
	  if (!pc->data_handler)
	    pc->data_handler =
	      ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DATA,
				       _mail_pop_server_data, NULL);

	  if ((ecore_con_ssl_available_get ()) && (pc->config->ssl))
	    type |= ECORE_CON_USE_MIXED;
	  pc->server =
	    ecore_con_server_connect (type, pc->config->host,
				      pc->config->port, pc);
	  pc->state = POP_STATE_DISCONNECTED;
	}
    }
}

void
_mail_pop_add_mailbox (void *data)
{
  PopClient *pc;
  Config_Box *cb;

  cb = data;
  if (!cb)
    return;
  pc = _mail_pop_client_get (cb);
  pc->config->num_new = 0;
  pc->config->num_total = 0;
  pclients = eina_list_append (pclients, pc);
}

void
_mail_pop_del_mailbox (void *data)
{
  PopClient *pc;
  Config_Box *cb;

  cb = data;
  if (!cb)
    return;
  pc = _mail_pop_client_get (cb);
  if (pc->server)
    _mail_pop_client_quit (pc);
  if (pc->add_handler)
    ecore_event_handler_del (pc->add_handler);
  if (pc->del_handler)
    ecore_event_handler_del (pc->del_handler);
  if (pc->data_handler)
    ecore_event_handler_del (pc->data_handler);
  pclients = eina_list_remove (pclients, pc);
}

void
_mail_pop_shutdown ()
{
  if (!pclients)
    return;

  while (pclients)
    {
      PopClient *pc;

      pc = pclients->data;
      if (!pc)
	continue;
      if (pc->server)
	_mail_pop_client_quit (pc);
      if (pc->add_handler)
	ecore_event_handler_del (pc->add_handler);
      if (pc->del_handler)
	ecore_event_handler_del (pc->del_handler);
      if (pc->data_handler)
	ecore_event_handler_del (pc->data_handler);
      pclients = eina_list_remove_list (pclients, pclients);
      free (pc);
      pc = NULL;
    }
}

/* PRIVATES */
static Eina_Bool
_mail_pop_server_add (void *data, int type, void *event)
{
  Ecore_Con_Event_Server_Add *ev = event;
  PopClient *pc;

  pc = _mail_pop_client_get_from_server (ev->server);
  if (!pc) return EINA_TRUE;

  pc->state = POP_STATE_CONNECTED;
  return EINA_FALSE;
}

static Eina_Bool
_mail_pop_server_del (void *data, int type, void *event)
{
  Ecore_Con_Event_Server_Del *ev = event;
  PopClient *pc;

  pc = _mail_pop_client_get_from_server (ev->server);
  if (!pc) return EINA_TRUE;

  if (pc->state == POP_STATE_DISCONNECTED)
    printf ("Pop Server Disconnected\n");

  ecore_con_server_del (ev->server);
  pc->server = NULL;
  pc->state = POP_STATE_DISCONNECTED;

  return EINA_FALSE;
}

static Eina_Bool
_mail_pop_server_data (void *data, int type, void *event)
{
  Ecore_Con_Event_Server_Data *ev = event;
  PopClient *pc;
  char in[2048], out[2048];
  int len, num = 0, total = 0;

  pc = _mail_pop_client_get_from_server (ev->server);
  if (!pc) return EINA_TRUE;
  if (pc->state == POP_STATE_DISCONNECTED) return EINA_TRUE;

  len = sizeof (in) - 1;
  len = (((len) > (ev->size)) ? ev->size : len);
  memcpy (in, ev->data, len);
  in[len] = 0;

  if (!strncmp (in, "-ERR", 4))
    {
      printf ("ERROR: %s\n", in);
      _mail_pop_client_quit (pc);
      return EINA_FALSE;
    }
  else if (strncmp (in, "+OK", 3))
    {
      printf ("Unexpected reply: %s\n", in);
      _mail_pop_client_quit (pc);
      return EINA_FALSE;
    }

  pc->state++;
  switch (pc->state)
    {
    case POP_STATE_SERVER_READY:
      len = snprintf (out, sizeof (out), "USER %s\r\n", pc->config->user);
      ecore_con_server_send (ev->server, out, len);
      break;
    case POP_STATE_USER_OK:
      len = snprintf (out, sizeof (out), "PASS %s\r\n", pc->config->pass);
      ecore_con_server_send (ev->server, out, len);
      break;
    case POP_STATE_PASS_OK:
      len = snprintf (out, sizeof (out), "STAT\r\n");
      ecore_con_server_send (ev->server, out, len);
      break;
    case POP_STATE_STATUS_OK:
      if (sscanf (in, "+OK %i %i", &num, &total) == 2)
	{
	  pc->config->num_new = num;
	  pc->config->num_total = num;
	}
      _mail_pop_client_quit (pc);
      if ((num > 0) && (pc->config->use_exec) && (pc->config->exec))
	_mail_start_exe (pc->config);
      break;
    default:
      break;
    }
  return EINA_FALSE;
}

static PopClient *
_mail_pop_client_get (void *data)
{
  PopClient *pc;
  Eina_List *l;
  Config_Box *cb;
  int found = 0;

  cb = data;
  if (!cb)
    return NULL;

  if ((!pclients) || (eina_list_count (pclients) <= 0))
    {
      pc = E_NEW (PopClient, 1);
      pc->server = NULL;
      pc->state = POP_STATE_DISCONNECTED;
      pc->config = cb;
      if (!pc->add_handler)
	pc->add_handler =
	  ecore_event_handler_add (ECORE_CON_EVENT_SERVER_ADD,
				   _mail_pop_server_add, NULL);
      if (!pc->del_handler)
	pc->del_handler =
	  ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DEL,
				   _mail_pop_server_del, NULL);
      if (!pc->data_handler)
	pc->data_handler =
	  ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DATA,
				   _mail_pop_server_data, NULL);
    }

  for (l = pclients; l; l = l->next)
    {
      pc = l->data;
      if (!pc)
	continue;
      if (!pc->config)
	continue;
      if ((!strcmp (pc->config->host, cb->host)) &&
	  (!strcmp (pc->config->user, cb->user)) &&
	  (!strcmp (pc->config->pass, cb->pass)))
	{
	  found = 1;
	  break;
	}
    }
  if (!found)
    {
      pc = E_NEW (PopClient, 1);
      pc->server = NULL;
      pc->state = POP_STATE_DISCONNECTED;
      pc->config = cb;
      if (!pc->add_handler)
	pc->add_handler =
	  ecore_event_handler_add (ECORE_CON_EVENT_SERVER_ADD,
				   _mail_pop_server_add, NULL);
      if (!pc->del_handler)
	pc->del_handler =
	  ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DEL,
				   _mail_pop_server_del, NULL);
      if (!pc->data_handler)
	pc->data_handler =
	  ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DATA,
				   _mail_pop_server_data, NULL);
    }
  return pc;
}

static PopClient *
_mail_pop_client_get_from_server (void *data)
{
  Ecore_Con_Server *server = data;
  Eina_List *l;

  if (!pclients)
    return NULL;
  for (l = pclients; l; l = l->next)
    {
      PopClient *pc;

      pc = l->data;
      if (!pc)
	continue;
      if (!pc->server)
	continue;
      if (pc->server == server)
	return pc;
    }
  return NULL;
}

static void
_mail_pop_client_quit (void *data)
{
  PopClient *pc;
  int len;
  char out[1024];

  pc = data;
  if (!pc)
    return;
  if (pc->state >= POP_STATE_CONNECTED)
    {
      len = snprintf (out, sizeof (out), "QUIT\r\n");
      ecore_con_server_send (pc->server, out, len);
    }
  ecore_con_server_del (pc->server);
  pc->server = NULL;
  pc->state = POP_STATE_DISCONNECTED;

  _mail_set_text (pc->data);
}
