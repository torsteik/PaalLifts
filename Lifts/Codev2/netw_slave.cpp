#include <stdio.h>
#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "netw_fsm.h"
#include "netw_slave.h"
#include "NetwMemb.h"

void serve(SharedVars* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;

	if (!netw_join(shared)){
		shared->netw_fsm_state = FSM_FIND_NETWORK;
		pthread_exit(NULL);
	}
	else{
		pthread_t event_thread;
		pthread_create(&event_thread, NULL, &report_events, shared);

		int sock_fd_set[2] = {
			shared->netw_membs[BROADCAST].sock_fd,
			shared->netw_membs[ip_to_id(shared->master_ip)].sock_fd
		};

		while (1){
			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 0.4*SEC_TO_USEC;

			fd_set read_fd_set = file_descriptor_setup(sock_fd_set, 2); //Keep an eye on these
			if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout) > -1){
				read_broadcast(shared, &read_fd_set);
				read_master(shared, &read_fd_set);


				shared->netw_membs[ip_to_id(shared->master_ip)].send_heartbeat();
			}
		}
	}
}

bool netw_join(SharedVars* shared){
	char send_msg[BUFF_SIZE];
	send_msg[0] = CONNECT;
	shared->netw_membs[BROADCAST].send(send_msg);
	//ACK
}

void* report_events_slave(void* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;

	while (1){
		usleep(0.1*SEC_TO_USEC);
		if (shared->orders_new[0]){
			shared->netw_membs[ip_to_id(shared->master_ip)].send(shared->orders_new);
			memset(shared->orders_new, 0, sizeof(shared->orders_new));
		}
		if (shared->orders_complete){
			shared->netw_membs[ip_to_id(shared->master_ip)].send(shared->orders_complete);
			memset(shared->orders_complete, 0, sizeof(shared->orders_complete));
		}
	}
}

void read_broadcast(SharedVars* shared, fd_set* read_fd_set_ptr){
	static int lives = 4;
	static clock_t prev_heartbeat = clock();

	RecvMsg broadcast_msg;
	if (FD_ISSET(shared->netw_membs[BROADCAST].sock_fd, read_fd_set_ptr)){
		broadcast_msg = shared->netw_membs[BROADCAST].recv();
	}
	else{
		broadcast_msg.MSG_ID = NO_RESPONSE;
	}
	switch (broadcast_msg.MSG_ID){
	case HEARTBEAT:
		lives = 4;
		prev_heartbeat = clock();
		break;

	case NO_RESPONSE:
		if ((clock() - prev_heartbeat) / CLOCKS_PER_SEC > 0.5){
			lives--;
			if (!lives){
				if (!shared->backup){
					shared->netw_fsm_state = FSM_FIND_NETWORK;
				}
				else{
					shared->netw_fsm_state = FSM_MASTER;
				}
				return;
			}
		}
		break;
	}
}

void read_master(SharedVars* shared, fd_set* read_fd_set_ptr){
	RecvMsg master_msg;
	if (FD_ISSET(shared->netw_membs[ip_to_id(shared->master_ip)].sock_fd, read_fd_set_ptr)){
		master_msg = shared->netw_membs[ip_to_id(shared->master_ip)].recv();
	}
	else{
		master_msg.MSG_ID = NO_RESPONSE;
	}

	switch (master_msg.MSG_ID){
	case NEW_ORDER:
		shared->local_q[master_msg.content[1]] = 1;
		//Send ack
		break;

	case TOGGLE_BACKUP:
		if (shared->netw_membs[MY_ID].role = SLAVE_ROLE){
			shared->netw_membs[MY_ID].role = BACKUP_ROLE;
			shared->netw_membs[master_msg.content[1]].role = BACKUP_ROLE;
		}
		else{
			shared->netw_membs[MY_ID].role = SLAVE_ROLE;
		}
		break;

	case BACKUP_DATA:
		for (int button = 0; button < N_FLOORS; button++){
			shared->netw_master_q[button] = master_msg.content[button + 1];
		}
		break;
	}
}
