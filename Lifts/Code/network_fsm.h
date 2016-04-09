#ifndef NETWORK_FSM_H
#define NETWORK_FSM_H

#define SEC_TO_USEC		1000000

#define BROADCAST		255

#define NETW_INFO_PORT	32123
#define ELEV_INFO_PORT	32124

#define MSG_ID			(int)msg.content[0] //Consider making enum && move to network_fsm.h
//OBS OBS OBS, MSG_ID only works if the "message" is called msg.
#define ERROR			0
#define	M_ALIVE			1 //Consider making this 3 so it sends hearts
#define CONNECT			2

// Type definitions:
enum state_t{ FIND_NETWORK, MASTER, SLAVE };
struct manage_connections_arg_t{

};

// Functions:
void network_fsm();
int find_master();
	//The functions to be run by threads should probably have their own module or something; maybe there should be a module for each state: master, slave
void* broadcast_alive_func(void* netw_info_sock_void_ptr);
void* manage_connections_func(void* netw_info_sock_void_ptr);
int determine_master();
#endif
