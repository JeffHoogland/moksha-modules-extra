#ifndef MDIR_H
#define MDIR_H

#include <e.h>
#include "e_mod_main.h"

typedef struct _MdirClient MdirClient;

struct _MdirClient 
{
   void *data;
   Config_Box *config;
   Ecore_File_Monitor *monitor;
};

void _mail_mdir_add_mailbox(void *data, void *data2);
void _mail_mdir_del_mailbox(void *data);
void _mail_mdir_shutdown();

#endif
