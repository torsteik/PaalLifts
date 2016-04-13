#ifndef NETW_MASTER_H
#define NETW_MASTER_H

#include "NetwMemb.h"

#define NO_BACKUP	0

struct SlaveThreadVars{			//Find better name
	SharedVars* shrd_vars;
	uint8_t slave_id;
};

void* broadcast_heartbeat(void* shrd_vars_void);
void* manage_netw(void* shrd_vars_void);
int determine_master(); //Bad name
void* handle_orders(void* shrd_vars_void);
void* manage_backup(void* shrd_vars_void);
#endif
