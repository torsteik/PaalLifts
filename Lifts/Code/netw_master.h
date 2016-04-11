#ifndef NETW_MASTER_H
#define NETW_MASTER_H

#include "Socket_UDP.h"

#define NO_BACKUP	-1

struct backup_func_arg_t{ //Change name to master_shared_variables and use this for all threads in master
	Socket_UDP*  netw_members_arg;
	char* master_q_arg;
};

struct shared_variables_t{
	Socket_UDP* netw_members;
	int ip;
	int* master_q
}; // Make sure this becomes a pointer and that all threads point to the same object. OBS, this is must not happen for ip_id!!!!
//(Probably not)Also after passing this to threads it might be unnecessary to typecast it, this could be done only when needed.

void* broadcast_alive_func(void* netw_members_void);
void* manage_connections_func(void* netw_members_void);
int determine_master(); //Bad name
void* manage_backup_func(void* );
int init_backup();
#endif
