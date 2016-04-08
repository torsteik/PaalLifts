#include <netinet/ip.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>

#include "network_fsm.h"
#include "Socket_UDP.h"

void network_fsm(){
	state_t state = FIND_NETWORK;
	Socket_UDP status_sock(INADDR_ANY, STATUS_PORT);

	while(1){
		switch(state){
			case FIND_NETWORK:
				if(!find_master(&status_sock)) state = MASTER;
				else 			 state = SLAVE;
				break;

			case MASTER:
				
				break;
			case SLAVE:
	
				break;
		}
	}
}

int find_master(Socket_UDP* status_sock){
	clock_t start = clock();

	struct timeval timeout; // Consider making the select() setup a func or try to simplify somehow
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000000;

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(status_sock.pid, &readfds);

	while(timeout.tv_usec > 0){
		if(select(status_sock +1, &readfds, NULL, NULL, &timeout)){
			//Receive function
		}
		timeout.tv_usec = (clock()-start)*1000000/CLOCKS_PER_SEC;
	}
}
