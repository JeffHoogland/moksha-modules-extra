#ifndef MBOX_H
#define MBOX_H

#include <e.h>
#include "e_mod_main.h"

typedef struct _MboxClient MboxClient;

struct _MboxClient 
{
   void *data;
   Config_Box *config;
   Ecore_File_Monitor *monitor;
};

void _mail_mbox_add_mailbox(void *data, void *data2);
void _mail_mbox_del_mailbox(void *data);
void _mail_mbox_shutdown();
void _mail_mbox_check_mail(void *data);
void _mail_mbox_check_monitors ();
#endif
