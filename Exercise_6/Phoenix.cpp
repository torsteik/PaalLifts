#include <sys/socket.h>
#include <netinet.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>



enum state_t {MASTER, BACKUP};
enum substate_m_t {CREATE_BACKUP, OPERATE};
enum substate_b_t {OPERATE, EVOLVE};

int main(){
	int port = 44420;
	int ip_addr ="129.241.187.147";

// TCP client()
	//Socket
	int client_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Address
	struct sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);
	inet_aton(ip_addr, &client_addr.sin_addr);

	//Socket settings
	int opt_val = 1;
	if(setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
		printf("setsockopt(SO_REUSEADDR) failed.\n");
	}

	char client_buffer[1024] = "Wololo!"; 

// TCP server()
	//Socket
	int server_sock = socket(AF_INET, (SOCK_STREAM|SOCK_NONBLOCK), 0);

	//Address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	//Socket settings
	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
		perror("setsockopt(SO_REUSEADDR) failed.\n");
	}
	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
		perror("setsockopt(SO_REUSEADDR) failed.\n");
	}
	//Bind
	if(bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		perror("Failed to bind server socket.\n");
	}
	//Start listening
	if(listen(server_sock, 1) < 0){
		perror("Failed to listen.\n");
	}

// FSM
	// Determine state
	state_t state;
	int master_sock = accept(server_sock,(struct sockaddr*) &server_addr, sizeof(server_addr));
	if( master_sock < 0){ state = MASTER; }
	else				{ state = BACKUP; }
	
	substate_m_t substate_m = CREATE_BACKUP;
	substate_b_t substate_b = OPERATE;

	switch(state){
		case MASTER:
			switch(substate_m){
				case CREATE_BACKUP:
					//fork()
					//delay(), probably not necessary
					//Connect client to server
					if(connect(client_sock, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0){
						perror("Failed to connect socket.\n");
					}
					substate_m = OPERATE;
				case OPERATE:
					//iterate()
					//alert_backup()
			}
		case BACKUP:
			switch(substate_b){
				case OPERATE:
					//recv()
					//store()
				case EVOLVE:
					//prepare to becom master
			}
	}

}
