#ifndef MAIN_H
#define MAIN_H

#include <sys/select.h>
#include <atomic>

//------NETWORK ADDRESS----------------------
#define MY_ID					0
#define BROADCAST				255		//0x40BC576FF equals 129.241.187.255
#define NETW_INFO_PORT			32123
#define ELEV_INFO_PORT			32124
//------MSG FORMAT---------------------------
#define BUFF_SIZE				1024
#define MSG_ID					content[0]
#define ERROR					1
#define NO_RESPONSE				2
#define	HEARTBEAT				3
#define CONNECT					4
#define ACKNOWLEDGE				5
#define TOGGLE_BACKUP			4
#define BACKUP_DATA				5
#define NEW_ORDER				7
#define COMPLETED_ORDER			8
//------NETW MEMB ROLES----------------------
#define NO_ROLE					0
#define SLAVE_ROLE				1
#define BACKUP_ROLE				2
#define MASTER_ROLE				3
#define BROADCAST_ROLE			4
//------NETW FSM STATES----------------------
#define FSM_FIND_NETWORK		0
#define FSM_MASTER				1
#define FSM_SLAVE				2
//------ELEV_STATES--------------------------
#define INIT 					0
#define IDLE 					1
#define MOVE					2
#define TEMP_STOP 				3
//------TEMP_STOP_SUBSTATEs------------------
#define ARRIVAL					0
#define WAIT_FOR_DOOR 			1
#define DETERMINE_NEXT_ACTION	2
//------UTILITY AND PARAMETERES--------------
#define NO_BACKUP				0
#define N_FLOORS				4
#define SEC_TO_USEC				1000000

class Queue;
class NetwMemb;

struct SharedVars {
	SharedVars();
	//---------MAIN--------------------------
	std::atomic<int> recovered;
	std::atomic<char> netw_master_q[N_FLOORS * 2]; //init
	Queue* elev_q;

	//---------ELEV_FSM----------------------
	std::atomic<char> floor;
	std::atomic<char> dir;
	std::atomic<char> elev_fsm_state;

	//---------NETW_FSM----------------------
	NetwMemb* netw_membs[256];
	std::atomic<int> netw_fsm_state;
	std::atomic<int> netw_role;
	std::atomic<int> backup;
	
	//---------NETW_MASTER-------------------

};

extern SharedVars* shared;

void file_descriptor_setup(int* sock_fd_set, fd_set* read_fd_set); //Maybe move to defines

#endif