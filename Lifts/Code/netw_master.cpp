#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/select.h>
#include <pthread.h>

#include "netw_master.h"
#include "netw_fsm.h"	//Because: 1. struct SharedVars
#include "NetwMemb.h"

void* broadcast_heartbeat(void* shrd_vars_void){
	SharedVars* shrd_vars = (SharedVars*)shrd_vars_void;
	char heartbeat_msg = HEARTBEAT;
	while (1){
		if (!shrd_vars->netw_membs[BROADCAST].send(&heartbeat_msg))
			printf("Failed to broadcast HEARTBEAT");
		usleep(0.4*SEC_TO_USEC);
	}
}

void* manage_netw(void* shrd_vars_void){
	SharedVars* shrd_vars = (SharedVars*)shrd_vars_void;
	pthread slave_com_threads[255];
	msg_t msg;
	char accept_msg = ACCEPT_CON;

	while (1){
		msg = shrd_vars->netw_membs[BROADCAST].recv();
		switch (MSG_ID){
		case ERROR:
			printf("Error receiving in manage_connecions_func(...).\n");				//Maybe this can be done in .recv()?
			break;

		case HEARTBEAT:
			determine_master();
			break;

		case CONNECT:
			SlaveThreadVars slave_thread_vars; // Make sure this won't overwrite the previous
			shrd_vars->netw_membs[ip_to_id(msg.sender_ip)] = NetwMemb(msg.sender_ip, msg.sender_ip, ELEV_INFO_PORT, SLAVE_SOCK);
			shrd_vars->netw_membs[ip_to_id(msg.sender_ip)].send(&accept_msg);
			slave_thread_vars.shrd_vars = shrd_vars;
			slave_thread_vars.slave_id = ip_to_id(msg.sender_ip);
			pthread_create(&slave_com_threads[ip_to_id(msg.sender_ip)], NULL, &handle_orders, &slave_thread_vars);
			break;
		}
	}
}

int determine_master(){

}

void* handle_orders(void* shrd_vars_void){										//HELP PLZ!
	SlaveThreadVars* slave_thread_vars = (SlaveThreadVars*)shrd_vars_void;
	SharedVars* shrd_vars = slave_thread_vars->shrd_vars;						//Fokin' names!
	int slave_id = slave_thread_vars->slave_id;									//...
	msg_t msg;																	//Look to rename, but this will affect define
	int cheapest_slave;
	int lives = 3;
	char new_order_msg[3];
	new_order_msg[0] = NEW_ORDER;
	SelectVar slct_var = select_setup(0, 400000, shrd_vars->netw_membs[slave_id].sock_fd);

	clock_t prev_heartbeat = clock();
	while (1){
		if (select(shrd_vars->netw_membs[slave_id].sock_fd + 1, &slct_var.readfds, NULL, NULL, &slct_var.timeout))
			msg = shrd_vars->netw_membs[slave_id].recv();
		else
			msg.content[0] = NO_RESPONSE;
		switch (MSG_ID){
		case HEARTBEAT:
			shrd_vars->netw_membs[slave_id].floor = msg.content[1];
			shrd_vars->netw_membs[slave_id].dir = msg.content[2];
			lives = 3;
			prev_heartbeat = clock();
			break;

		case NEW_ORDER:
			for (int button = 0; button < N_FLOORS * 2; button++){
				if (!shrd_vars->master_q[button] && msg.content[button + 1]){
					new_order_msg[2] = button;
					cheapest_slave = cost_fun(shrd_vars, button);
					shrd_vars->netw_membs[cheapest_slave].send(new_order_msg);
					//get ack
					shrd_vars->master_q[button] = slave_id;
				}
			}
			break;

		case COMPLETED_ORDER:
			for (int button = 0; button < N_FLOORS * 2; button++){
				if (shrd_vars->master_q[button] && msg.content[button + 1]){
					shrd_vars->master_q[button] = 0;
					//ack
				}
			}
			break;

		case NO_RESPONSE:
			if ((clock() - prev_heartbeat) / CLOCKS_PER_SEC > 0.5){
				lives--;
				if (!lives){
					for (int button = 0; button < N_FLOORS * 2; button++){
						if (shrd_vars->master_q[button] == slave_id){
							//Place order in local_q / make it re-enter the network as a new order
						}
						close(shrd_vars->netw_membs[slave_id].sock_fd);
						shrd_vars->netw_membs[slave_id] = NetwMemb();
						pthread_exit();										// Might need arg for return value
					}
				}
			}
		}
	}
}

void* manage_backup(void* shrd_vars_void){
	SharedVars* shrd_vars = (SharedVars*)shrd_vars_void;
	int  backup = NO_BACKUP;
	char upgrade_msg = BECOME_BACKUP;
	int  update = 0;
	char update_msg[1 + N_FLOORS * 2];
	update_msg[0] = BACKUP_DATA;
	for (int button = 1; button < N_FLOORS * 2 +1; button++) update_msg[button +1] = 0;

	while (1){																			//Consider using a sleep here so it doesnt use more processing power than necessary
		if (backup == NO_BACKUP){
			for (int memb_id = 0; memb_id < 255; memb_id++){
				if (shrd_vars->netw_membs[memb_id].role == SLAVE_SOCK){
					shrd_vars->netw_membs[memb_id].send(&upgrade_msg);					// Need ack here and abort if not received
					backup = memb_id;
					shrd_vars->netw_membs[memb_id].role = BACKUP_SOCK;
					break;
				}
			}
		}

		else{
			for (int button = 0; button < N_FLOORS * 2; ++button){
				if (shrd_vars->master_q[button] != update_msg[button +1]){
					update_msg[button +1] = shrd_vars->master_q[button];
					update = 1;
				}
			}
			if (update){
				shrd_vars->netw_membs[backup].send(update_msg);
				update = 0;
			}
		}
	}
}

int cost_fun(shared_variables_t* shared_vars, char new_order){
	/*
	struct SharedVars{
	NetwMemb netw_membs[256];
	int master_q[N_FLOORS * 2];
	uint8_t* slave_id;
	};
	
	struct SlaveThreadVars{			//Find better name
	SharedVars* shrd_vars;
	uint8_t slave_id;
	};
	
	new_order = [ETG,DIR], DIR =-1(down), 0(idle), 1(up)
	netw_membs[i] = [ETG,DIR]
	*/
	
	int member_utilities [256];
	for(int i = 0; i < 256; ++i){
		unsigned int cost = 1000;
		int desired_direction = new_order[ETG]-(netw_membs[i][ETG]+netw_membs[i][DIR])
		if (!desired_dirction){
			return i;
		}
		else if(desired_direction < 0 && netw_membs[i][DIR] <= 0){
			cost = (unsigned)desired_direction;
		}
		else if(desired_direction < 0 && netw_membs[i][DIR] > 0){
			cost = N_FLOORS - (netw_membs[i][ETG]+netw_membs[i][DIR]) + N_FLOORS - new_order[ETG];
		}
		else if(desired_direction > 0 && netw_membs[i][DIR] >= 0){
			cost = desired_direction;
		}
		else if(desired_direction > 0 && netw_membs[i][DIR] < 0){
			cost = (netw_membs[i][ETG]+netw_membs[i][DIR]) + new_order[ETG];
		}
		if(!netw_membs[i][DIR]){
			cost--;
		}
		int member_utilities [i] = cost;
		
	}
	int min_utility = 2*N_FLOORS;
	int min_index = 0;
	for(int i = 0; i < 256; ++i){
		if (member_utilities[i] < min_utility){
			min_utility = member_utilities[i];
			min_index = i;
		}
	}
	
	return i;
}
