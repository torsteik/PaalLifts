#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/select.h>
#include <pthread.h>
#include <iostream>
#include <semaphore.h>

#include "main.h"
#include "netw_fsm.h"	//Because: 1. struct SharedVars
#include "netw_master.h"
#include "NetwMemb.h"
#include "Queue.h"

void* manage_netw(){
	pthread_t elev_managing_threads[255];

	pthread_create(&elev_managing_threads[MY_ID], NULL, &handle_local_events_master, NULL);

	while (1){
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0.4*SEC_TO_USEC;

		fd_set read_fd_set;
		file_descriptor_setup(&shared->netw_membs[BROADCAST].sock_fd, &read_fd_set);
		if (select(shared->netw_membs[BROADCAST].sock_fd + 1, &read_fd_set, NULL, NULL, &timeout) > 0){
			
			struct RecvMsg recv_msg = shared->netw_membs[BROADCAST].recv();
			if(recv_msg.MSG_ID == CONNECT){
				int* slave_id = new int;
				*slave_id = ip_to_id(recv_msg.sender_ip);

				char send_msg[BUFF_SIZE];
				send_msg[0] = ACKNOWLEDGE;

				shared->netw_membs[*slave_id] = NetwMemb(*slave_id, *slave_id, ELEV_INFO_PORT, SLAVE_ROLE);
				shared->netw_membs[*slave_id].send(&send_msg);

				pthread_create(&elev_managing_threads[*slave_id], NULL, &manage_slave, slave_id);
			}
		}
		shared->netw_membs[BROADCAST].send_heartbeat(shared->netw_master_q);
	}
}

void* handle_local_events_master(){
	while (1){
		usleep(0.1*SEC_TO_USEC);
		
		if (shared->elev_q->orders_new[0]){
			for (int button = 0; button < N_FLOORS * 2; ++button){
				if (!shared->netw_master_q[button] && shared->elev_q->orders_new[button + 1]){
					char send_msg[BUFF_SIZE];
					send_msg[0] = NEW_ORDER;			
					send_msg[1] = button;
					
					uint8_t cheapest_slave = cost_fun(shared, button);
					if(shared->netw_membs[cheapest_slave].send_and_get_ack(send_msg)){
						shared->netw_master_q[button] = cheapest_slave;
					}
				}
			}
			memset(shared->elev_q->orders_new, 0, 1 + N_FLOORS * 2);
		}

		if (shared->elev_q->orders_complete){
			for (int button = 0; button < N_FLOORS * 2; button++){			
				if (shared->netw_master_q[button] && shared->orders_complete[button + 1]){
					shared->netw_master_q[button] = 0;
				}
			}
			memset(shared->elev_q->orders_complete, 0, 1 + N_FLOORS * 2);
		}
	}
}

void* manage_slave(void* slave_id_void_ptr){
	int slave_id = (int)(*slave_id_void_ptr);

	int lives = 4;
	clock_t prev_heartbeat = clock();
	while (1){
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0.2*SEC_TO_USEC;
		
		struct RecvMsg recv_msg;
		fd_set read_fd_set;
		file_descriptor_setup(&shared->netw_membs[slave_id].sock_fd, &read_fd_set);
		if (select(shared->netw_membs[slave_id].sock_fd + 1, &read_fd_set, NULL, NULL, &timeout) > 0)
			recv_msg = shared->netw_membs[slave_id].recv();
		else
			recv_msg.MSG_ID = NO_RESPONSE;

		switch (recv_msg.MSG_ID){
		case HEARTBEAT:
			shared->netw_membs[slave_id].floor 			= recv_msg.content[1];
			shared->netw_membs[slave_id].dir 			= recv_msg.content[2];
			shared->netw_membs[slave_id].elev_fsm_state = recv_msg.content[3];
			lives = 4;
			prev_heartbeat = clock();
			break;

		case NEW_ORDER:
			for (int button = 0; button < N_FLOORS * 2; button++){
				if (!shared->netw_master_q[button] && recv_msg.content[1 + button]){
					
					uint8_t cheapest_slave = cost_fun(button);
					if (cheapest_slave == MY_ID){
						shared->elev_q->add_order_from_netw(button);
						shared->netw_master_q[button] = 255;
					}
					else{
						char send_msg[BUFF_SIZE];
						send_msg[0] = NEW_ORDER;
						send_msg[1] = button;
						if(shared->netw_membs[cheapest_slave].send_and_get_ack(send_msg)){
							shared->netw_master_q[button] = cheapest_slave;
						}
					}
				}
			}
			break;

		case COMPLETED_ORDER:
			for (int button = 0; button < N_FLOORS * 2; ++button){
				if (shared->netw_master_q[button] && recv_msg.content[button + 1]){
					shared->netw_master_q[button] = 0;
					
					char send_msg[BUFF_SIZE];
					send_msg[0] = ACKNOWLEDGE;
					shared->netw_membs[slave_id].send(send_msg);
				}
			}
			break;

		case NO_RESPONSE:
			if ((clock() - prev_heartbeat) / CLOCKS_PER_SEC > 0.5){
				lives--;
				if (!lives){
					for (int button = 0; button < N_FLOORS * 2; ++button){
						if (shared->netw_master_q[button] == slave_id){
							shared->elev_q->recycle_order(button);
						}
					}
					if (shared->backup == slave_id){
						shared->backup = NO_BACKUP;
					}
					close(shared->netw_membs[slave_id].sock_fd);
					shared->netw_membs[slave_id] = NetwMemb();
					delete slave_id;
					pthread_exit(NULL);
				}
			}
			break;
		}
	}
}

void* manage_backup(){
	if (!shared->recovered) // Probably needed more places(or not), since only one error occurs at a time this might not be necessary
		shared->backup = NO_BACKUP;
	}

	char backup_q_msg[BUFF_SIZE];
	while (1){		 	
		usleep(0.3*SEC_TO_USEC);
		if (shared->backup == NO_BACKUP){
			// Sleep some more here maybe?
			for (int memb_id = 1; memb_id < 255; ++memb_id){
				if (shared->netw_membs[memb_id].netw_role == SLAVE_ROLE){
					char toggle_msg[BUFF_SIZE];
					send_msg[0] = TOGGLE_BACKUP;
					send_msg[1] = memb_id;
					
					if (shared->netw_membs[memb_id].send_and_get_ack(send_msg)){ //if unlucky, might toggle off(increase time maybe(inside ack))
						shared->netw_membs[memb_id].netw_role = BACKUP_ROLE;
						shared->backup = memb_id;
						break;
					}
				}
			}
		}
		else{
			bool update = 0;
			for (int button = 0; button < N_FLOORS * 2; ++button){
				if (shared->netw_master_q[button] != backup_q_msg[1 + button]){
					backup_q_msg[0] = BACKUP_DATA;
					backup_q_msg[1 + button] = shared->netw_master_q[button];
					update = 1;
				}
			}
			if (update){
				shared->netw_membs[shared->backup].send(send_msg);
			}
		}
	}
}

int cost_fun(uint8_t new_order){
	int order_floor;
	int order_dir;

	if (new_order >= N_FLOORS){
		order_floor = new_order - N_FLOORS;
		order_dir = 1;
	}
	else{
		order_floor = new_order;
		order_dir = 0;
	}
	int elev_floor;
	int elev_dir;
	int desired_dir;
	
	int best_index;
	int	best_cost = INT_MAX;
	int cost = 0;

	for (int i = 0; i < 255; ++i){

		cost = 0;

		if (shared->netw_memb[i].role){
			if (shared->netw_memb[i].elev_fsm_state != INIT){

				elev_dir = shared->netw_membs[i].dir;
				elev_floor = shared->netw_membs[i].floor;
				desired_dir = (order_floor < elev_floor);

				cost = (0.5*shared->netw_memb[i].elev_fsm_state + 1)*N_FLOORS;
				cost += abs(order_floor - elev_floor);
				if ((shared->netw_memb[i].elev_fsm_state) == MOVE && (abs(order_floor - elev_floor) == 0)){
					cost += N_FLOOR;
				}
				if ((elev_dir + desired_dir + order_dir == 0) || (elev_dir + desired_dir + order_dir == 3)){
					cost -= N_FLOORS;
				}
			}
		}
		if (cost < best_cost){
			best_index = i;
		}
	}
	return best_index;
}