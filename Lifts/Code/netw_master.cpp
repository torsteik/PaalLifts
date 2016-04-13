#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/select.h>
#include <pthread.h>

#include "main.h"
#include "netw_fsm.h"	//Because: 1. struct SharedVars
#include "netw_master.h"
#include "NetwMemb.h"

void* broadcast_heartbeat(void* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;
	char heartbeat_msg = HEARTBEAT;
	while (1){
		if (!shared->netw_membs[BROADCAST].send(&heartbeat_msg))
			printf("Failed to broadcast HEARTBEAT");
		usleep(0.4*SEC_TO_USEC);
	}
}

void* manage_netw(void* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;
	pthread_t order_handling_threads[255];

	while (1){
		msg_t recv_msg = shared->netw_membs[BROADCAST].recv(); // Do this everywhere: recv_msg
		switch (recv_msg.MSG_ID){
		case ERROR:
			printf("Error receiving in manage_connecions_func(...).\n");				//Maybe this can be done in .recv()?
			break;

		case HEARTBEAT:
			determine_master();
			break;

		case CONNECT:
			char send_msg = ACCEPT_CON;
			shared->netw_membs[ip_to_id(recv_msg.sender_ip)] = NetwMemb(recv_msg.sender_ip, recv_msg.sender_ip, ELEV_INFO_PORT, SLAVE_ROLE);
			shared->netw_membs[ip_to_id(recv_msg.sender_ip)].send(&send_msg);
			shared->slave_id = ip_to_id(recv_msg.sender_ip); // Need to use semaphore to keep this alive
			pthread_create(&order_handling_threads[ip_to_id(recv_msg.sender_ip)], NULL, &handle_orders, shared);
			break;
		}
	}
}

int determine_master(){

}

void* handle_orders(void* shared_void){										//HELP PLZ!
	SharedVars* shared = (SharedVars*)shared_void;							//Fokin' names!
	int slave_id = shared->slave_id;										//... / Release semaphore here
	msg_t msg;																//Look to rename, but this will affect define
	int cheapest_slave;
	int lives = 3;
	char new_order_msg[2];
	new_order_msg[0] = NEW_ORDER;
	SelectVar slct_var;

	clock_t prev_heartbeat = clock();
	while (1){
		slct_var = select_setup(0, 400000, &shared->netw_membs[slave_id].sock_fd, 1);
		if (select(shared->netw_membs[slave_id].sock_fd + 1, &slct_var.readfds, NULL, NULL, &slct_var.timeout))
			msg = shared->netw_membs[slave_id].recv();
		else
			msg.content[0] = NO_RESPONSE;
		switch (MSG_ID){
		case HEARTBEAT:
			shared->netw_membs[slave_id].floor = msg.content[1];
			shared->netw_membs[slave_id].dir = msg.content[2];
			lives = 3;
			prev_heartbeat = clock();
			break;

		case NEW_ORDER:
			for (int button = 0; button < N_FLOORS * 2; button++){
				if (!shared->netw_master_q[button] && msg.content[button + 1]){
					new_order_msg[1] = button;
					cheapest_slave = cost_fun(shared, button);
					shared->netw_membs[cheapest_slave].send(new_order_msg);
					//get ack
					shared->netw_master_q[button] = slave_id;
				}
			}
			break;

		case COMPLETED_ORDER:
			for (int button = 0; button < N_FLOORS * 2; button++){
				if (shared->netw_master_q[button] && msg.content[button + 1]){
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
						}
						close(shared->netw_membs[slave_id].sock_fd);
						shared->netw_membs[slave_id] = NetwMemb();
						pthread_exit(NULL);										// Might need arg for return value
					}
				}
			}
		}
	}
}

void* manage_backup(void* shared_void){
	SharedVars* shared = (SharedVars*)shared_void;
	int  backup = NO_BACKUP;
	char upgrade_msg = BECOME_BACKUP;
	int  update = 0;
	char update_msg[1 + N_FLOORS * 2];
	update_msg[0] = BACKUP_DATA;
	for (int button = 1; button < N_FLOORS * 2 +1; button++) update_msg[button +1] = 0;

	while (1){																			//Consider using a sleep here so it doesnt use more processing power than necessary
		if (backup == NO_BACKUP){
			for (int memb_id = 0; memb_id < 255; memb_id++){
				if (shared->netw_membs[memb_id].role == SLAVE_ROLE){
					shared->netw_membs[memb_id].send(&upgrade_msg);					// Need ack here and abort if not received
					backup = memb_id;
					shared->netw_membs[memb_id].role = BACKUP_ROLE;
					break;
				}
			}
		}

		else{
			for (int button = 0; button < N_FLOORS * 2; ++button){
				if (shared->netw_master_q[button] != update_msg[button +1]){
					update_msg[button +1] = shared->netw_master_q[button];
					update = 1;
				}
			}
			if (update){
				shared->netw_membs[backup].send(update_msg);
				update = 0;
			}
		}
	}
}

int cost_fun(SharedVars* shared, uint8_t new_order){
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
	for (int i = 1; i < 255; ++i){

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
	int min_index = 0;
	for (int i = 1; i < 255; ++i){
		if (member_utilities[i] < min_utility){
			min_utility = member_utilities[i];
			min_index = i;
		}
	}

	return min_index;
}
