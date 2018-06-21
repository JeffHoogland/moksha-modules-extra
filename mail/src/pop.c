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
  void *list_data;
  
  inst = data;
  if (!inst)
    return;
  
  for (l = pclients; l; l = l->next)
    {
      PopClient *pc;

      pc = l->data;
      if (!pc)
       continue;
      
      EINA_LIST_FREE(pc->config->senders, list_data)
      eina_stringshare_del(list_data);
       
      pc->config->parse = 0;
      pc->config->iterator = 1;
      pc->config->num = 0;
      pc->config->buf = eina_strbuf_new();
      eina_strbuf_reset(pc->config->buf);
      
  
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
  char in[2048], out[2048], parse_from[20]=""; 
  int len, total = 0;
  const char *heystack;
  const char *tmp;

  pc = _mail_pop_client_get_from_server (ev->server);
  if (!pc) return EINA_TRUE;
  if (pc->state == POP_STATE_DISCONNECTED) return EINA_TRUE;

  len = sizeof (in) - 1;
  len = (((len) > (ev->size)) ? ev->size : len);
  memcpy (in, ev->data, len);
  in[len] = 0;
  eina_strbuf_append(pc->config->buf, in);

  if (!strncmp (in, "-ERR", 4))
    {
      printf ("ERROR: %s\n", in);
      _mail_pop_client_quit (pc);
      return EINA_FALSE;
    }   
  else if (strncmp (in, "+OK", 3))
    {
      printf ("Unexpected reply: %s\n", in);
      //~ _mail_pop_client_quit (pc);
      return EINA_FALSE;
    }

  pc->state++;
  
  switch (pc->state)
    {
    case POP_STATE_SERVER_READY: //2
      len = snprintf (out, sizeof (out), "USER %s\r\n", pc->config->user);
      ecore_con_server_send (ev->server, out, len);
      break;
    case POP_STATE_USER_OK: //3
      len = snprintf (out, sizeof (out), "PASS %s\r\n", pc->config->pass);
      ecore_con_server_send (ev->server, out, len);
      break;
    case POP_STATE_PASS_OK: //4 Find out the mail numbers
      len = snprintf (out, sizeof (out), "STAT\r\n");
      ecore_con_server_send (ev->server, out, len);
      break;
    case POP_STATE_STATUS_OK: //5
      if (pc->config->parse == 0) //Finding out the number of mails just once after STAT command
      {
        if (sscanf (in, "+OK %i %i", &pc->config->num, &total) == 2)
        {
          pc->config->num_new = pc->config->num;
          pc->config->num_total = total;
        }
      }    
      
      if (pc->config->iterator <= pc->config->num) //TOP command sends to server for each mail 
      {
        len = snprintf (out, sizeof (out), "TOP %d 0 \r\n", pc->config->iterator);
        ecore_con_server_send (ev->server, out, len);
        pc->config->iterator++;
        pc->state=4;
        pc->config->parse = 1;
      }
      else
      {
        pc->state = 5;
        len = snprintf (out, sizeof (out), "LIST \r\n");
        ecore_con_server_send (ev->server, out, len);
      }
      break;
    case POP_STATE_PARSE_OK: //Parsing the data. I am looking for "From: <name@mail>"
        heystack = eina_strbuf_string_get(pc->config->buf);
        //~ printf("\n------------NEWHeystack %s\n", heystack);
        while ((heystack = strstr(heystack, "From: ")) != NULL)
        {
          tmp = heystack;
          while (((heystack = strstr(heystack, "<")) != NULL) && ((heystack - tmp) < 45))
          {
            sscanf(heystack, "<%[^>\n]", parse_from);
            pc->config->senders = eina_list_prepend(pc->config->senders, 
                                     eina_stringshare_add(parse_from));
            heystack ++;
          }
          heystack = tmp + 1;
        } 
        
         eina_strbuf_free(pc->config->buf);
        _mail_pop_client_quit(pc); 
         
         if ((pc->config->num > 0) && (pc->config->use_exec) && (pc->config->exec))
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
  PopClient *pc=data;
  int len;
  char out[1024];
  Eina_List *l;

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
