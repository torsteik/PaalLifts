#ifndef NETW_MASTER_H
#define NETW_MASTER_H

#include "NetwMemb.h"

void* broadcast_heartbeat(void* shrd_vars_void);
void* manage_netw(void* shrd_vars_void);
int determine_master(); //Bad name
void* handle_orders(void* shrd_vars_void);
void* manage_backup(void* shrd_vars_void);
#endif
