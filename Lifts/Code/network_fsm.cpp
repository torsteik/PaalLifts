#include <netinet/ip.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "netw_fsm.h"
#include "netw_master.h"
#include "NetwMemb.h"

void network_fsm(){
	SharedVars shrd_vars;																//Should maybe be named shrd_master_vars or something
	shrd_vars.netw_membs[BROADCAST] = NetwMemb(INADDR_ANY, id_to_ip(BROADCAST), NETW_INFO_PORT, BROADCASTER); /*Review: OBS! BROADCAST need to be full IP in host byte order and BROADCASTER must be the lamest define ever, lol*/
	for (int i = 0; i < N_FLOORS * 2; i++)	shrd_vars.master_q[i] = 0;

	unsigned long master_ip;

	State state = FIND_NETWORK;
	while (1){
		switch (state){
		case FIND_NETWORK:
			master_ip = find_master(shrd_vars.netw_membs);
			if (!master_ip) state = MASTER; 
			else 			state = SLAVE;
			break;

		case MASTER:
			pthread master_threads[3];

			pthread_create(&master_threads[0], NULL, &broadcast_heartbeat, &shrd_vars);
			pthread_create(&master_threads[1], NULL, &manage_netw, &shrd_vars);			//Investigate how sockets works as shared variables
			pthread_create(&master_threads[2], NULL, &manage_backup, &shrd_vars);
			break;

		case SLAVE:

			break;
		}
	}
}

int find_master(NetwMemb netw_membs[256]){
	msg_t msg;
	SelectVar slct_var = select_setup(0, 1000000 - 1, netw_membs[BROADCAST].sock_fd);
	clock_t start = clock();
	while (slct_var.timeout.tv_usec > 0){
		if (select(netw_membs[BROADCAST].sock_fd + 1, &slct_var.readfds, NULL, NULL, &slct_var.timeout)){ //Not sure about first argument
			msg = netw_membs[BROADCAST].recv();
			if (MSG_ID == HEARTBEAT) return msg.sender_ip;
		}
		slct_var.timeout.tv_usec = (clock() - start) * SEC_TO_USEC / CLOCKS_PER_SEC;
	}
	return 0;
}

SelectVar select_setup(int sec, int usec, int fd){
	SelectVar slct_var;
	//Set timeout
	slct_var.timeout.tv_sec = sec;
	slct_var.timeout.tv_usec = usec;
	//Place the file descriptor, fd, in a file descriptor set. 
	FD_ZERO(&slct_var.readfds);
	FD_SET(fd, &slct_var.readfds);
	return slct_var;
}
