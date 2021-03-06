#ifndef MAIN_H
#define MAIN_H
#include <stdint.h>

#define NO_BACKUP	0
#define N_FLOORS	4

struct SharedVars {
	//---------MAIN--------------------------
	bool recovered;
	uint8_t netw_master_q[N_FLOORS * 2];
	uint8_t local_q[N_FLOORS * 2]; //include _ext_ in name

	//---------NETW_FSM----------------------
	NetwMemb netw_membs[256];
	int netw_fsm_state;
	int backup;
	
	//---------NETW_MASTER-------------------
	uint8_t slave_id;						 
};

/*
This needs to be done in main:

for (int i = 0; i < N_FLOORS * 2; i++){
shared->netw_master_q[i] = 0;
shared->local_q[i] = 0;
}
*/
#endif
