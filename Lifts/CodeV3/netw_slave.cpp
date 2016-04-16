#include <stdio.h>
#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "netw_fsm.h"
#include "netw_slave.h"
#include "NetwMemb.h"
#include "Queue.h"

void serve(unsigned long master_ip){
	int master_id = ip_to_id(master_ip); // Do this outside?

	char send_msg[BUFF_SIZE];
	send_msg[0] = CONNECT;
	if (!shared->netw_membs[master_id].send_and_get_ack(send_msg)){}
		shared->netw_fsm_state = FSM_FIND_NETWORK;
		pthread_exit(NULL);
	}
	else{
		pthread_t local_event_thread;
		pthread_create(&local_event_thread, NULL, &report_local_events_slave, &master_id);

		int sock_fd_set[2] = {
			shared->netw_membs[BROADCAST].sock_fd,
			shared->netw_membs[master_id].sock_fd
		};

		while (1){
			struct timeval timeout;
			timeout.tv_sec  = 0;
			timeout.tv_usec = 0.2*SEC_TO_USEC;  // Care timeout

			fd_set read_fd_set;
			file_descriptor_setup(sock_fd_set, &read_fd_set); //Keep an eye on these
			if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout) > -1){
				
				read_broadcast(&read_fd_set);
				read_master(&read_fd_set, master_id);
				shared->netw_membs[master_id].send_heartbeat(shared->netw_master_q); // To include or not to include that is... so include sharedvars in netw_membs o no?
			}
		}
	}
}

void* report_local_events_slave(void* master_id_void){
	int master_id = (int)(*master_id_void); // OBS! Concurrency issues o no?
	
	while (1){
		usleep(0.1*SEC_TO_USEC); // Care sleep, think of timing with ACK
		if (shared->elev_q->orders_new[0]){
			shared->netw_membs[master_id].send(shared->elev_q->orders_new);
			memset(shared->elev_q->orders_new, 0, sizeof(shared->elev_q->orders_new));
		}
		if (shared->elev_q->orders_complete){
			if(shared->netw_membs[master_id].send_and_get_ack(shared->elev_q->orders_complete)){
				memset(shared->elev_q->orders_complete, 0, sizeof(shared->elev_q->orders_complete));
			}
		}
	}
}

void read_broadcast(fd_set* read_fd_set_ptr){
	static int lives = 4;
	static clock_t prev_heartbeat = clock();

	struct RecvMsg broadcast_msg;
	if (FD_ISSET(shared->netw_membs[BROADCAST].sock_fd, read_fd_set_ptr)){
		broadcast_msg = shared->netw_membs[BROADCAST].recv();
	}
	else{
		broadcast_msg.MSG_ID = NO_RESPONSE;
	}

	switch (broadcast_msg.MSG_ID){
	case HEARTBEAT:
		shared->elev_q->sync_q_with_netw(broadcast_msg);
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

void read_master(fd_set* read_fd_set_ptr, int master_id){
	
	struct RecvMsg master_msg;
	if (FD_ISSET(shared->netw_membs[master_id].sock_fd, read_fd_set_ptr)){
		master_msg = shared->netw_membs[master_id].recv();
	}
	else{
		master_msg.MSG_ID = NO_RESPONSE;
	}

	switch (master_msg.MSG_ID){
	case NEW_ORDER:
		shared->elev_q->add_order_from_netw(master_msg.content[1]);
		
		char send_msg[BUFF_SIZE];
		send_msg[0] = ACKNOWLEDGE;
		shared->netw_membs[master_id].send(send_msg);
		break;

	case TOGGLE_BACKUP:
		if (shared->netw_membs[MY_ID].netw_role = SLAVE_ROLE){
			shared->netw_membs[MY_ID].netw_role = BACKUP_ROLE;
			shared->netw_membs[master_msg.content[1]].netw_role = BACKUP_ROLE;

			char send_msg[BUFF_SIZE];
			send_msg[0] = ACKNOWLEDGE;
			shared->netw_membs[master_id].send(send_msg); // Must happen before ack timer runs out
		}
		else{
			shared->netw_membs[MY_ID].netw_role = SLAVE_ROLE;
		}
		break;

	case BACKUP_DATA:
		for (int button = 0; button < N_FLOORS; ++button){
			shared->netw_master_q[button] = master_msg.content[1 + button];
		}
		break;
	}
}
