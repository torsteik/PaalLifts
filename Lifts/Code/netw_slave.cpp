#include <stdio.h>
#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
 
#include "defines.h"
#include "NetwMemb.h"
#include "Queue.h"
#include "SharedVars.h"
#include "netw_slave.h"

void serve(unsigned long master_ip){
	int master_id = ip_to_id(master_ip);

	char send_msg[BUFF_SIZE] = {0};
	send_msg[0] = CONNECT;
	printf("Asking senpai to notice me.\n");
	if (!shared.netw_membs[BROADCAST].send_and_get_ack(send_msg)){
		printf("Did not join network.\n");
		shared.netw_fsm_state = FSM_FIND_NETWORK;
		return;
	}
	else{
		printf("Im in baby!\n");
		pthread_t local_event_thread;
		pthread_create(&local_event_thread, NULL, &report_local_events_slave, &master_id);

		int sock_fd_set[2] = {
			shared.netw_membs[BROADCAST].sock_fd,
			shared.netw_membs[master_id].sock_fd
		};

		while (1){
			struct timeval timeout;
			timeout.tv_sec  = 0;
			timeout.tv_usec = 0.2*SEC_TO_USEC;  // Care timeout

			fd_set read_fd_set;
			file_descriptor_setup(sock_fd_set, 2, &read_fd_set); //Keep an eye on these
			if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout) > -1){
				
				if(read_broadcast(&read_fd_set)){
					shared.netw_membs[master_id] = NetwMemb();
					return;
				}
				read_master(&read_fd_set, master_id);
				shared.netw_membs[master_id].floor = shared.floor;
				shared.netw_membs[master_id].dir = shared.dir;
				shared.netw_membs[master_id].elev_fsm_state = shared.elev_fsm_state;
				shared.netw_membs[master_id].send_heartbeat(shared.netw_master_q, timeout);

			}
		}
	}
}

void* report_local_events_slave(void* master_id_void){
	int master_id = (*(int*)master_id_void);
	
	while (1){
		usleep(0.15*SEC_TO_USEC);

		if (shared.elev_q.orders_new[0]){
			printf("Status: %i\n", shared.elev_q.orders_new[0]);
			printf("sending new order\n");
			shared.netw_membs[master_id].send(shared.elev_q.orders_new);
			printf("ChckP1\n");
			memset(shared.elev_q.orders_new, 0, sizeof(shared.elev_q.orders_new));
		}
		if (shared.elev_q.orders_complete[0]){
			printf("Notifies master, order_completed\n");
			if(shared.netw_membs[master_id].send_and_get_ack(shared.elev_q.orders_complete)){
				memset(shared.elev_q.orders_complete, 0, sizeof(shared.elev_q.orders_complete));
			}
		}
	}
}

int read_broadcast(fd_set* read_fd_set_ptr){
	static int lives = 4;
	static time_t prev_heartbeat = time(0);

	struct RecvMsg broadcast_msg;
	if (FD_ISSET(shared.netw_membs[BROADCAST].sock_fd, read_fd_set_ptr)){
		broadcast_msg = shared.netw_membs[BROADCAST].recv();
	}
	else{
		broadcast_msg.MSG_ID = NO_RESPONSE;
	}

	switch (broadcast_msg.MSG_ID){
	case HEARTBEAT:
		shared.elev_q.sync_q_with_netw(broadcast_msg.content);
		lives = 4;
		prev_heartbeat = time(0);
		return 0;

	case NO_RESPONSE:
		if ((time(0) - (double)prev_heartbeat) > 1){
			prev_heartbeat = time(0);
			lives--;
			if (lives < 1){
				if (!shared.backup){
					shared.netw_fsm_state = FSM_FIND_NETWORK;
					printf("I'm going on an adventure\n");
				}
				else{
					shared.backup = 0;
					shared.netw_fsm_state = FSM_MASTER;
					printf("I'm backup\n");
				}
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

void read_master(fd_set* read_fd_set_ptr, int master_id){
	
	struct RecvMsg master_msg;
	if (FD_ISSET(shared.netw_membs[master_id].sock_fd, read_fd_set_ptr)){
		master_msg = shared.netw_membs[master_id].recv();
	}
	else{
		master_msg.MSG_ID = NO_RESPONSE;
	}
	char send_msg[BUFF_SIZE] = {};
	switch (master_msg.MSG_ID){
	case NEW_ORDER:
		shared.elev_q.add_order_from_netw(master_msg.content[1]);
		
		send_msg[0] = ACKNOWLEDGE;
		shared.netw_membs[master_id].send(send_msg);
		break;

	case TOGGLE_BACKUP:
		if (shared.netw_membs[MY_ID].netw_role == SLAVE_ROLE){
			shared.netw_membs[MY_ID].netw_role = BACKUP_ROLE;
			shared.backup = 1;
			shared.netw_membs[(unsigned char)master_msg.content[1]].netw_role = BACKUP_ROLE;
			
			send_msg[0] = ACKNOWLEDGE;
			shared.netw_membs[master_id].send(send_msg);
		}
		else{
			shared.netw_membs[MY_ID].netw_role = SLAVE_ROLE;
		}
		break;

	case BACKUP_DATA:
		for (int button = 0; button < N_FLOORS; ++button){
			shared.netw_master_q[button] = master_msg.content[1 + button];
		}
		break;
	}
}
