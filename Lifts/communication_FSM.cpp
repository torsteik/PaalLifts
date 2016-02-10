// Network
#include <sys/socket.h>
// Standard
#include <iostream>
#include <errno.h>
// Local
#include "communication_FSM.h"

using namespace std;

/*	NOTES:
	Might want to create a slave and a master module.
	Remember to set the SO_KEEPALIVE option when needed.
*/
void communication_FSM(){
	role_t role = MASTER;
	master_state_t master_state = FIND_NETWORK;
	
	while (1){

		switch (role){
		case MASTER:

			switch (master_state){
			case FIND_NETWORK:
				
			}
		case SLAVE:

		}
	}
}

string find_network(){
	// Create socket for searching
	string ip_address = "000.000.000."; 
	int port = 15994;

	int client_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = port;

	int opt_val = 1;
	if (setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt_val, sizeof(opt_val)) < 0){
		perror("Failed to apply SO_REUSEADDR.\n");
	}

	for (int i = 0; i < 255; ++i){
		
	}
}
