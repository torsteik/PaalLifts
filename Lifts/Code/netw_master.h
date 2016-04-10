#ifndef NETW_MASTER_H
#define NETW_MASTER_H

#include "Socket_UDP.h"

#define NO_BACKUP	-1

struct backup_func_arg_t{ //Change name to master_shared_variables and use this for all threads in master
	Socket_UDP*  netw_members_arg;
	char* master_q_arg;
};

void* broadcast_alive_func(void* netw_members_void);
void* manage_connections_func(void* netw_members_void);
int determine_master(); //Bad name
void* manage_backup_func(void* );
int init_backup();
#endif
