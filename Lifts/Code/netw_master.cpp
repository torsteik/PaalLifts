#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/select.h>

#include <pthread.h>

#include "netw_master.h"
#include "network_fsm.h"
#include "Socket_UDP.h"

void* broadcast_alive_func(void* netw_members_void){
	Socket_UDP* netw_members = (Socket_UDP*)netw_members_void;
	char alive_msg = M_ALIVE; //Could be an error in whats sent here
	while (1){
		if (!netw_members[255].send(&alive_msg))
			printf("Failed to broadcast M_ALIVE");
		usleep(0.4*SEC_TO_USEC);
	}
}

void* manage_connections_func(void* netw_members_void){ //Change name, it's about creating not managing. Maybe include recv functionality.
	Socket_UDP* netw_members = (Socket_UDP*)netw_members_void;
	msg_t msg;

	pthread member_com_threads[256]; //Maybe reduce to 255
	shared_variables_t* shared_vars; // might just need to update the one passed to manage_connections_func. No dont make shallow copy of ip_id
	while (1){
		msg = netw_members[255].recv();

		switch (MSG_ID){
		case ERROR:
			printf("Error receiving in manage_connecions_func(...).\n");
			break;

		case M_ALIVE:
			//Agree on who's boss
			determine_master();
			break;

		case CONNECT:
			char accept_msg = ACCEPT_CON;
			//Accept new slave
///////////////////////////////////////////
			netw_members[something(msg.sender_ip)] = Socket_UDP(msg.sender_ip, msg.sender_ip, ELEV_INFO_PORT, SLAVE_SOCK);///ERMAGERDHH! FOKIN' WOT M8, U HAVIN' A GIGGLE!??? Find out how to get the last byte of host byte ordered long
///////////////////////////////////////////
			//Send confirmation
			if (!netw_members[something(msg.sender_ip)].send(&accept_msg))
				printf("Failed to accept connection.\n");
			
			pthread_create(&member_com_threads[something(msg.sender_ip)], NULL, &distribute_orders, shared_vars);
			break;

		default:
			//Nothing of intrest was received
			break;
		}
	}
}

int determine_master(){

}

void* manage_backup_func(void* backup_func_arg_void){
	backup_func_arg_t* backup_func_arg = (backup_func_arg_t*)backup_func_arg_void;
	Socket_UDP* netw_members = backup_func_arg->netw_members_arg; 
	char* master_q = backup_func_arg->master_q_arg;
	
	int backup = NO_BACKUP;
	char upgrade_msg = BECOME_BACKUP;
	int update = 0;
	char update_msg[1 + N_FLOORS * 2]; //Contains backup command and previous version of master_q
	update_msg[0] = BACKUP_DATA;
	for (int i = 1; i < 1 + N_FLOORS * 2; i++)	master_q[i] = 255; //OBS 255, not 0

	while (1){
		//Consider using a sleep here so it doesnt use more processing power than necessary
		if (backup == NO_BACKUP){
			//Search for slave
			for (int i = 0; i < 255; ++i){
				if (netw_members[i].state == SLAVE_SOCK){
					//Upgrade slave to backup
					if (!netw_members[i].send(&upgrade_msg))
						break;
					backup = i;
					netw_members[i].state = BACKUP_SOCK;
					break;
				}
			}
		}

		else{
			//If there is a change in master_Q, update backup
			for (int i = 0; i < N_FLOORS * 2; ++i){
				if (master_q[i] != update_msg[i]){
					update_msg[i] = master_q[i];
					update = 1;
				}
			}
			if (update)
				netw_members[backup].send(update_msg);
		}
	}
}

void* distribute_orders(void* shared_vars_void){
	shared_variables_t* shared_vars = (shared_variables_t*)shared_vars_void;
	msg_t msg;
	int cheapest_slave;
	char new_order_msg[3];
	new_order_msg[0] = NEW_ORDER;
	int lives = 3;
	
	struct timeval timeout; // Consider making the select() setup a func or try to simplify somehow
	timeout.tv_sec = 0;
	timeout.tv_usec = 400000;
	//Set processes to watch
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET((shared_vars->netw_members[shared_vars->ip]).pid, &readfds);  //Macro shared vars or make a new pointer or some shit

	clock_t prev_heartbeat = clock();

	while (1){
		if (select(shared_vars->netw_members[shared_vars->ip].pid + 1, &readfds, NULL, NULL, &timeout)){ //Not sure about first argument
			msg = shared_vars->netw_members[shared_vars->ip].recv();
		}
		switch (MSG_ID){
		case S_ALIVE:
			shared_vars->netw_members[shared_vars->ip].floor = msg.content[1];
			shared_vars->netw_members[shared_vars->ip].dir	 = msg.content[2];
			lives = 3;
			prev_heartbeat = clock();
			break;

		case NEW_ORDER:
			for (int i = 0; i < N_FLOORS * 2; i++){
				if (shared_vars->master_q[i] != 255 && msg.content[i + 1]){  // Meaning the order is not already active
					new_order_msg[2] = i;
					cheapest_slave = cost_fun(shared_vars, i);
					shared_vars->netw_members[cheapest_slave].send(new_order_msg);
					//get ack
					shared_vars->master_q[i] = shared_vars->ip;
				}
			}
		case COMPLETED_ORDER:
		}
	}
}

int cost_fun(shared_variables_t* shared_vars, char new_order){

}
