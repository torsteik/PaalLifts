//Network
#include <netinet/ip.h>
#include <sys/time.h>
#include <sys/select.h>
//Threading
#include <pthread.h>
//Other
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "network_fsm.h"
#include "Socket_UDP.h"

void network_fsm(){
	state_t state = FIND_NETWORK;
	//Socket_UDP netw_info_sock(INADDR_ANY, BROADCAST, NETW_INFO_PORT);
	Socket_UDP netw_members[256]; //List of members of the network. Members are id'd by their IP and broadcast's ID is taken by an INADDR_ANY socket
	netw_members[255] = Socket_UDP(INADDR_ANY, BROADCAST, NETW_INFO_PORT);

	while (1){
		switch (state){
		case FIND_NETWORK:
			if (!find_master(netw_members))  state = MASTER; //OBS, make sure ip = .0 is not part of network
			else 								state = SLAVE;
			break;

		case MASTER:
			pthread broadcast_alive_thread; //Consider making a pthread array; would make naming simpler
			pthread manage_connections_thread;

			str

			pthread_create(&broadcast_alive_thread, NULL, &broadcast_alive_func, netw_members); //Most likely prone to error, terrible variable names aswell
			pthread_create(&manage_connections_thread, NULL, &manage_connections_func, netw_members); //Investigate how sockets works as shared variables
			
			break;

		case SLAVE:

			break;
		}
	}
}

int find_master(Socket_UDP* netw_members){
	msg_t msg;
	//Set timeout
	struct timeval timeout; // Consider making the select() setup a func or try to simplify somehow
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000000 -1; //Not sure if this can be more than a second
	//Set processes to watch
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET((netw_members[255]).pid, &readfds);
	//Start M_ALIVE signal search
	clock_t start = clock();
	while (timeout.tv_usec > 0){
		if (select(&netw_members[255] + 1, &readfds, NULL, NULL, &timeout)){ //Not sure about first argument
			msg = netw_members[255].recv();
			if (MSG_ID == M_ALIVE) return msg.sender_ip;
		}
		timeout.tv_usec = (clock() - start) * SEC_TO_USEC / CLOCKS_PER_SEC;
	}
	return 0;
}

void* broadcast_alive_func(void* netw_members_void){
	Socket_UDP* netw_members = (Socket_UDP*)netw_members_void;
	char msg = M_ALIVE; //Could be an error in whats sent here
	while (1){
		if (!netw_members[255].send(&msg))
			printf("Failed to broadcast M_ALIVE");
		usleep(0.4*SEC_TO_USEC);
	}
}

void* manage_connections_func(void* netw_members_void){
	Socket_UDP* netw_members = (Socket_UDP*)netw_members_void;
	msg_t msg;
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
			//Accept new slave
			netw_members[something(msg.sender_ip)] = Socket_UDP(msg.sender_ip, msg.sender_ip, ELEV_INFO_PORT)///ERMAGERDHH! FOKIN' WOT M8, U HAVIN' A GIGGLE!??? Find out how to get the last byte of host byte ordered long
			//Send confirmation
			break;

		default:
			//Nothing of intrest was received
			break;
		}
	}
}

int determine_master(){

}tv_usec = (clock()-start)*1000000/CLOCKS_PER_SEC;
	}
}
