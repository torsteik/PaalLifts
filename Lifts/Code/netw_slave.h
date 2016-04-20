#ifndef NETW_SLAVE_H
#define NETW_SLAVE_H

void serve(unsigned long master_ip);
void* report_local_events_slave(void* master_id_void);
int read_broadcast(fd_set* read_fd_set_ptr);
void read_master(fd_set* read_fd_set_ptr, int master_id);

#endif