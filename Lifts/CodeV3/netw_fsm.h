#ifndef NETWORK_FSM_H
#define NETWORK_FSM_H

#include "NetwMemb.h"
#include <stdint.h>



//------NETW FSM STATES------
#define FSM_FIND_NETWORK	0
#define FSM_MASTER			1
#define FSM_SLAVE			2

void* netw_fsm(void* shared_void);
unsigned long find_master();
void follow_up_ext_orders();
void file_descriptor_setup(int* sock_fd_set, fd_set* read_fd_set)
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
/*
fd_set select_setup(int sec, int usec, int sock_fd_set[], int set_size){
	SelectVar slct_var;
	//Set timeout
	slct_var.timeout.tv_sec = sec;
	slct_var.timeout.tv_usec = usec;
	//Place the file descriptor, fd, in a file descriptor set. 
	FD_ZERO(&slct_var.readfds);
	for (int i = 0; i < set_size; i++){
		FD_SET(sock_fd_set[i], &slct_var.readfds);
	}
	return slct_var;
*/
/*
void* broadcast_heartbeat(void* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;
	char heartbeat_msg = HEARTBEAT;
	while (1){
		shared->netw_membs[BROADCAST].send(&heartbeat_msg);
		usleep(0.4*SEC_TO_USEC);
	}
}
*/