#include <netinet/ip.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "main.h"
#include "netw_fsm.h"
#include "netw_master.h"
#include "NetwMemb.h"

void* netw_fsm(void* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;														
	shared->netw_membs[BROADCAST] = NetwMemb(INADDR_ANY, id_to_ip(BROADCAST), NETW_INFO_PORT, BROADCAST_ROLE); 

	shared->netw_fsm_state = FSM_FIND_NETWORK;
	while (1){
		switch (shared->netw_fsm_state){
		case FSM_FIND_NETWORK:
			unsigned long master_ip = find_master(shared->netw_membs);
			if (!master_ip) shared->netw_fsm_state = MASTER; 
			else 			shared->netw_fsm_state = SLAVE;
			break;

		case FSM_MASTER:
			pthread_t master_threads[3];

			pthread_create(&master_threads[0], NULL, &broadcast_heartbeat, shared);
			pthread_create(&master_threads[1], NULL, &manage_netw, shared);			//Investigate how sockets works as shared variables
			pthread_create(&master_threads[2], NULL, &manage_backup, shared);

			sleep(2);
			if (shared->netw_membs[MY_ID].role == BACKUP_ROLE){ /*Look up own role in NetwMembs*/
				follow_up_ext_orders(shared);
			}
			break;

		case FSM_SLAVE:

			break;
		}
	}
}

unsigned long find_master(NetwMemb netw_membs[256]){
	struct timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 1000000 - 1;

	clock_t start = clock();
	while (timeout.tv_usec > 0)	{
		
		fd_set read_fd_set = file_descriptor_setup(&netw_membs[BROADCAST].sock_fd, 1);
		if (select(netw_membs[BROADCAST].sock_fd + 1, &read_fd_set, NULL, NULL, &timeout)){
			
			RecvMsg msg = netw_membs[BROADCAST].recv();
			if (msg.MSG_ID == HEARTBEAT){
				return msg.sender_ip;
			}
		}
		timeout.tv_usec = (clock()-start) * SEC_TO_USEC/CLOCKS_PER_SEC;
	}
	return 0;
}

// Maybe taking in a pointer and updating it is necessary
fd_set file_descriptor_setup(int* sock_fd_set, int set_size){ 
	fd_set read_fd_set;
	FD_ZERO(&read_fd_set);
	for (int i = 0; i < set_size; i++){
		FD_SET(sock_fd_set[i], &read_fd_set);
	}
	return read_fd_set;
}

void follow_up_ext_orders(SharedVars* shared){
	for (int button = 0; button < N_FLOORS * 2; button++){
		if (shared->netw_master_q[button] && shared->netw_membs[shared->netw_master_q[button]].role == EMPTY_SOCK){
			shared->local_q[button] = shared->netw_master_q[button];
		}
	}
}
