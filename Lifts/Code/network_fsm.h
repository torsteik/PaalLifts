#ifndef NETWORK_FSM_H
#define NETWORK_FSM_H

#include "NetwMemb.h"
#include <stdint.h>
//Most of these defines can be placed in NetwMemb, also solving or include loop problem
#define SEC_TO_USEC		1000000
#define N_FLOORS		4

enum State{ FIND_NETWORK, MASTER, SLAVE };

struct SharedVars{
	NetwMemb netw_membs[256];
	int master_q[N_FLOORS * 2];
	uint8_t* slave_id;
}; //OBS, this is must not happen for ip_id!!!!

struct SelectVar{
	struct timeval timeout;
	fd_set readfds;
};

void network_fsm();
int find_master(NetwMemb netw_membs[256]);
SelectVar select_setup(int sec, int usec, int fd);
#endif


/*
struct timeval timeout; // Consider making the select() setup a func or try to simplify somehow
timeout.tv_sec = 0;
timeout.tv_usec = 400000;
//Set processes to watch
fd_set readfds;
FD_ZERO(&readfds);
FD_SET((shared_vars->netw_members[shared_vars->ip]).sock_fd, &readfds);  //Macro shared vars or make a new pointer or some shit
*/
