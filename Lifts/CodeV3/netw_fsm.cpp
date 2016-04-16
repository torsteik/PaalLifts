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
#include "netw_slave.h"
#include "NetwMemb.h"
#include "Queue.h"

void* netw_fsm(){													
	unsigned long master_ip;
	shared->netw_membs[BROADCAST] = NetwMemb(INADDR_ANY, id_to_ip(BROADCAST), NETW_INFO_PORT, BROADCAST_ROLE); 
	shared->netw_fsm_state = FSM_FIND_NETWORK;
	while (1){

		switch (shared->netw_fsm_state){
		case FSM_FIND_NETWORK:
			//Test connection
			
			master_ip = find_master();
			if (!master_ip) shared->netw_fsm_state = FSM_MASTER;
			else 			shared->netw_fsm_state = FSM_SLAVE;
			break;

		case FSM_MASTER:
			pthread_t master_threads[2];

			pthread_create(&master_threads[0], NULL, &manage_netw,   NULL);			//Investigate how sockets works as shared variables
			pthread_create(&master_threads[1], NULL, &manage_backup, NULL);

			int prev_role = shared->netw_membs[MY_ID].role;
			if ( prev_role == BACKUP_ROLE) /*Look up own role in NetwMembs*/
				sleep(2);
				follow_up_ext_orders(shared);

			pthread_join(master_threads[0], NULL);
			pthread_join(master_threads[1], NULL);
			break;

		case FSM_SLAVE:
			// Move inside serve?
			shared->netw_membs[ip_to_id(master_ip)] = NetwMemb(master_ip, master_ip, ELEV_INFO_PORT, MASTER_ROLE);
			
			serve(master_ip);
			break;
		}
	}
}

unsigned long find_master(){ //DENY PASSING IF NO CONNECTION?
	struct timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 1000000 - 1;

	clock_t start = clock();
	while (timeout.tv_usec > 0)	{
		
		fd_set read_fd_set;
		file_descriptor_setup(&shared->netw_membs[BROADCAST].sock_fd, &read_fd_set);
		if (select(netw_membs[BROADCAST].sock_fd + 1, &read_fd_set, NULL, NULL, &timeout)){
			
			struct RecvMsg msg = netw_membs[BROADCAST].recv();
			if (msg.MSG_ID == HEARTBEAT){
				return msg.sender_ip;
			}
		}
		timeout.tv_usec = (clock() - start) * SEC_TO_USEC/CLOCKS_PER_SEC;
	}
	return 0;
}

void follow_up_ext_orders(){
	int elev_id;
	for (int button = 0; button < N_FLOORS * 2; ++button){
		elev_id = shared->netw_master_q[button];
		if (elev_id && (shared->netw_membs[elev_id].netw_role == NO_ROLE || shared->netw_membs[elev_id].role == BROADCAST_ROLE)){
			shared->elev_q->recycle_order(button);
			shared_netw_master_q[button] = 0;
		}
	}
}

void file_descriptor_setup(int* sock_fd_set, fd_set* read_fd_set){ 
	FD_ZERO(read_fd_set);
	for (int i = 0; i < sizeof(sock_fd_set); ++i){
		FD_SET(sock_fd_set[i], read_fd_set);
	}
	return read_fd_set;
}