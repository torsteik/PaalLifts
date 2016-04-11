#ifndef NETWORK_FSM_H
#define NETWORK_FSM_H

//Most of these defines can be placed in Socket_UDP, also solving or include loop problem
#define SEC_TO_USEC		1000000

#define N_FLOORS		4

#define BROADCAST		255

#define NETW_INFO_PORT	32123
#define ELEV_INFO_PORT	32124

//These could be moved to Socket_UDP
#define MSG_ID			(int)msg.content[0] //Consider making enum && move to network_fsm.h
//OBS OBS OBS, MSG_ID only works if the "message" is called msg.
#define ERROR			0
#define	M_ALIVE			1 //Consider making this 3 so it sends hearts, could have slave send this as well
#define CONNECT			2
#define ACCEPT_CON		3
#define BECOME_BACKUP	4
#define BACKUP_DATA		5
#define S_ALIVE			6
#define NEW_ORDER		7
#define COMPLETED_ORDER	8

// Type definitions:
enum state_t{ FIND_NETWORK, MASTER, SLAVE };


// Functions:
void network_fsm();
int find_master();
//The functions to be run by threads should probably have their own module or something; maybe there should be a module for each state: master, slave

#endif
