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
#include "netw_master.h"
#include "Socket_UDP.h"

void network_fsm(){
	state_t state = FIND_NETWORK;
	
	Socket_UDP netw_members[256]; //List of members of the network. Members are id'd by their IP and broadcast's ID is taken by an INADDR_ANY socket
	netw_members[255] = Socket_UDP(INADDR_ANY, BROADCAST, NETW_INFO_PORT, BROADCAST_SOCK);
	
	char master_q[N_FLOORS*2];
	for (int i = 0; i < N_FLOORS * 2; i++)	master_q[i] = 255; //OBS 255, not 0

	

	while (1){
		switch (state){
		case FIND_NETWORK:
			if (!find_master(netw_members)) state = MASTER; //OBS, make sure ip = .0 is not part of network or do i want this to be possible and instead use 255 as the false value
			else 							state = SLAVE;
			break;

		case MASTER:
			pthread broadcast_alive_thread; //Consider making a pthread array; would make naming simpler. I second this!
			pthread manage_connections_thread;
			pthread manage_backup_thread;
			pthread distribute_orders_thread;
			
			pthread_create(&broadcast_alive_thread, NULL, &broadcast_alive_func, netw_members); //Most likely prone to error, terrible variable names aswell
			pthread_create(&manage_connections_thread, NULL, &manage_connections_func, netw_members); //Investigate how sockets works as shared variables
			pthread_create(&manage_backup_thread, NULL, &manage_backup_func, netw_members);
			pthread_create(&distribute_orders_thread, NULL, )
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
