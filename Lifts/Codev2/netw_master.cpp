#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/select.h>
#include <pthread.h>
#include <iostream>

#include "main.h"
#include "netw_fsm.h"	//Because: 1. struct SharedVars
#include "netw_master.h"
#include "NetwMemb.h"

void* manage_netw(void* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;
	pthread_t order_handling_threads[255];

	pthread_create(&order_handling_threads[MY_ID], NULL, &report_events_master, shared);

	while (1){
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0.4*SEC_TO_USEC;

		RecvMsg recv_msg;
		fd_set read_fd_set = file_descriptor_setup(&shared->netw_membs[BROADCAST].sock_fd, 1);
		if (select(shared->netw_membs[BROADCAST].sock_fd + 1, &read_fd_set, NULL, NULL, &timeout) > 0){
			recv_msg = shared->netw_membs[BROADCAST].recv(); // Do this everywhere: recv_msg
		}

		switch (recv_msg.MSG_ID){
		case HEARTBEAT:
			determine_master();
			break;

		case CONNECT:
			char send_msg = ACCEPT_CON; //Ack
			shared->netw_membs[ip_to_id(recv_msg.sender_ip)] = NetwMemb(recv_msg.sender_ip, recv_msg.sender_ip, ELEV_INFO_PORT, SLAVE_ROLE);
			shared->netw_membs[ip_to_id(recv_msg.sender_ip)].send(&send_msg);
			shared->slave_id = ip_to_id(recv_msg.sender_ip); // Need to use semaphore to keep this alive
			pthread_create(&order_handling_threads[ip_to_id(recv_msg.sender_ip)], NULL, &handle_orders, shared);
			break;
		}

		shared->netw_membs[BROADCAST].send_heartbeat();
	}
}

void* report_events_master(void* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;

	while (1){
		usleep(0.1*SEC_TO_USEC);
		if (shared->orders_new[0]){
			char send_msg[BUFF_SIZE];
			send_msg[0] = NEW_ORDER;
			for (int button = 0; button < N_FLOORS * 2; button++){
				if (!shared->netw_master_q[button] && shared->orders_new[button + 1]){
					send_msg[1] = button;
					uint8_t cheapest_slave = cost_fun(shared, button);
					shared->netw_membs[cheapest_slave].send(send_msg);
					//get ack
					shared->netw_master_q[button] = cheapest_slave;
				}
			}
			memset(shared->orders_new, 0, 1 + N_FLOORS * 2);
		}

		if (shared->orders_complete){
			for (int button = 0; button < N_FLOORS * 2; button++){
				if (shared->netw_master_q[button] && shared->orders_complete[button + 1]){
					shared->netw_master_q[button] = 0;
					//ack
				}
			}
			memset(shared->orders_new, 0, 1 + N_FLOORS * 2);
		}
	}
}

int determine_master(){

}

void* handle_orders(void* shared_void){										//HELP PLZ!
	SharedVars* shared = (SharedVars*)shared_void;							//Fokin' names!
	int slave_id = shared->slave_id;										//... / Release semaphore here

	int lives = 4;
	clock_t prev_heartbeat = clock();
	while (1){
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0.4*SEC_TO_USEC;
		fd_set read_fd_set = file_descriptor_setup(&shared->netw_membs[slave_id].sock_fd, 1);

		RecvMsg recv_msg;
		if (select(shared->netw_membs[slave_id].sock_fd + 1, &read_fd_set, NULL, NULL, &timeout) > 0)
			recv_msg = shared->netw_membs[slave_id].recv();
		else
			recv_msg.MSG_ID = NO_RESPONSE;

		switch (recv_msg.MSG_ID){
		case HEARTBEAT:
			shared->netw_membs[slave_id].floor = recv_msg.content[1];
			shared->netw_membs[slave_id].dir = recv_msg.content[2];
			shared->netw_membs[slave_id].elev_fsm_state = recv_msg.content[3];
			lives = 4;
			prev_heartbeat = clock();
			break;

		case NEW_ORDER:
			char send_msg[BUFF_SIZE];
			send_msg[0] = NEW_ORDER;
			for (int button = 0; button < N_FLOORS * 2; button++){
				if (!shared->netw_master_q[button] && recv_msg.content[button + 1]){
					send_msg[1] = button;
					uint8_t cheapest_slave = cost_fun(shared, button);
					if (cheapest_slave == MY_ID){
						shared->local_q[button] = 1;
						shared->netw_master_q[button] = 255;
					}
					else{
						shared->netw_membs[cheapest_slave].send(send_msg);
						//get ack
						shared->netw_master_q[button] = cheapest_slave;
					}
				}
			}
			break;

		case COMPLETED_ORDER:
			for (int button = 0; button < N_FLOORS * 2; button++){
				if (shared->netw_master_q[button] && recv_msg.content[button + 1]){
					shared->netw_master_q[button] = 0;
					//ack
				}
			}
			break;

		case NO_RESPONSE:
			if ((clock() - prev_heartbeat) / CLOCKS_PER_SEC > 0.5){
				lives--;
				if (!lives){
					for (int button = 0; button < N_FLOORS * 2; button++){
						if (shared->netw_master_q[button] == slave_id){
							//Place order in local_q / make it re-enter the network as a new order
							shared->orders_new[button + 1] = 1;
							shared->orders_new[0] = NEW_ORDER;
						}
					}
					if (shared->backup == slave_id){
						shared->backup = NO_BACKUP;
					}

					close(shared->netw_membs[slave_id].sock_fd);
					shared->netw_membs[slave_id] = NetwMemb();
					pthread_exit(NULL);										// Might need arg for return value
				}
			}
			break;
		}
	}
}

void* manage_backup(void* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;
	if (!shared->recovered){										// Probably needed more places
		shared->backup = NO_BACKUP;
	}
	char send_msg[BUFF_SIZE] = {};
	bool update = 0;

	while (1){																			//Consider using a sleep here so it doesnt use more processing power than necessary
		usleep(0.1*SEC_TO_USEC);
		if (shared->backup == NO_BACKUP){
			send_msg[0] = TOGGLE_BACKUP;
			for (int memb_id = 1; memb_id < 255; memb_id++){
				if (shared->netw_membs[memb_id].role == SLAVE_ROLE){
					send_msg[1] = memb_id;
					shared->netw_membs[memb_id].send(send_msg);					// Need ack here and abort if not received
					shared->backup = memb_id;
					shared->netw_membs[memb_id].role = BACKUP_ROLE;
					break;
				}
			}
		}

		else{
			send_msg[0] = BACKUP_DATA;
			for (int button = 0; button < N_FLOORS * 2; ++button){
				if (shared->netw_master_q[button] != send_msg[button + 1]){
					send_msg[button + 1] = shared->netw_master_q[button];
					update = 1;
				}
			}
			if (update){
				shared->netw_membs[shared->backup].send(send_msg);
				update = 0;
			}
		}
	}
}

uint8_t cost_fun(SharedVars* shared, uint8_t new_order){
	/*
	Preferable if dir is a bool
	And state will be available so include that in calculations

	new_order = [ETG,DIR], DIR =-1(down), 0(idle), 1(up)
	netw_membs[i] = [ETG,DIR]
	*/
	// DEFINING THINGS THAT MAKES SENSE FOR A UTILITY FUNCTION
	int order_floor;
	int order_dir;
	if (new_order >= N_FLOORS){
		order_floor = new_order - N_FLOORS;
		order_dir = -1;
	}
	else{
		order_floor = new_order;
		order_dir = 1;
	}
	int elev_floor;
	int elev_dir;

	int member_utilities[256];
	for (int i = 0; i < 255; ++i){

		elev_dir = shared->netw_membs[i].dir;
		elev_floor = shared->netw_membs[i].floor + elev_dir;
		unsigned int cost = 1000;
		int desired_direction = order_floor - elev_floor;

		if (desired_direction < 0 && elev_dir <= 0){
			cost = (unsigned)desired_direction;
		}
		else if (desired_direction < 0 && elev_dir > 0){
			cost = N_FLOORS - elev_floor + N_FLOORS - order_floor;
		}
		else if (desired_direction > 0 && elev_dir >= 0){
			cost = desired_direction;
		}
		else if (desired_direction > 0 && elev_dir < 0){
			cost = elev_floor + order_floor;
		}
		if (!shared->netw_membs[i].dir){
			cost--;
		}
		if (shared->netw_membs[i].state == EMERGENCY){
			cost = 1000;
		}
		member_utilities[i] = cost;

	}
	int min_utility = 2 * N_FLOORS;
	uint8_t min_index = 0;
	for (int i = 1; i < 255; ++i){
		if (member_utilities[i] < min_utility){
			min_utility = member_utilities[i];
			min_index = i;
		}
	}
	return min_index;
}
