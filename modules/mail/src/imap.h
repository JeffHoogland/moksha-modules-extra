#ifndef IMAP_H
#define IMAP_H

#include <e.h>
#include "e_mod_main.h"

typedef struct _ImapServer ImapServer;
typedef struct _ImapClient ImapClient;

typedef enum 
{
   IMAP_STATE_DISCONNECTED,
     IMAP_STATE_CONNECTED,
     IMAP_STATE_SERVER_READY,
     IMAP_STATE_LOGGED_IN,
     IMAP_STATE_STATUS_OK
} ImapState;

struct _ImapServer
{
   int port;
   unsigned char local;
   unsigned char ssl;
   const char *host;
   const char *user;
   const char *pass;

   Ecore_Con_Server *server;
   Eina_List *clients;
   Eina_List *current;
   int cmd;
   ImapState state;
   void *data;
   
   Ecore_Event_Handler *add_handler;
   Ecore_Event_Handler *del_handler;
   Ecore_Event_Handler *data_handler;
};

struct _ImapClient 
{
   Config_Box *config;
   ImapServer *server;
};

void _mail_imap_check_mail(void *data);
void _mail_imap_add_mailbox(void *data);
void _mail_imap_del_mailbox(void *data);
void _mail_imap_shutdown();

#endif

//imap.1and1.com
//m39988144-2/3
////testing/2
//
