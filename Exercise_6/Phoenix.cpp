#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>


enum state_t {MASTER, BACKUP};
enum substate_m_t {CREATE_BACKUP, OPERATE_M};
enum substate_b_t {OPERATE_B, EVOLVE};

int main(void){
	int port = 44420;
	char ip_addr[] ="129.241.187.32";

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
	/*
	struct timeval timeout;
	timeout.tv_sec = 3;
	if(setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO,(void*) &timeout, sizeof(timeout)) < 0){
		perror("setsockopt(SO_REUSEADDR) failed.\n");
	}
	*/
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
	uint16_t legacy = 1;
	uint8_t heartbeat = 3;
	// Determine state
	state_t state;
	
	// Figure out how to use select() and make the socket block
	sleep(3);// LAME, remove when select is introduced
	
	int master_sock;
	unsigned int master_len;
	struct sockaddr_in master_addr;
	master_addr.sin_family = AF_INET;

	master_sock = accept(server_sock,(struct sockaddr*) &master_addr, &master_len);
	if( master_sock < 0){ state = MASTER; }
	else				{ state = BACKUP; }
	
	substate_m_t substate_m = CREATE_BACKUP;
	substate_b_t substate_b = OPERATE_B;

	switch(state){
		case MASTER:
			switch(substate_m){
				case CREATE_BACKUP:{
					// Spawn new process
					pid_t pid = fork();
					if(!pid){
						printf("Wouaaa wouaaa wu wu wouaaaaaaaaaa!\n");
						// Restart child process
						if(execl("Phoenix", "Phoenix", (char*)NULL) < 0){
							perror("Failed to run exec().\n");
						}
					}
					else if(pid < 0){
						perror("Miscarriage.\n");
						sleep(1);
						printf("Trying again...\n");
					}
					else{
						// Connect master to backup
						printf("It's a boy!");
						if(connect(client_sock, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0){
							perror("Failed to connect socket.\n");
						}
						substate_m = OPERATE_M;
					}
					break;
				}
				case OPERATE_M:{
					sleep(1);// LAME, remove when select is introduced
					printf("%i", legacy);
					if(send(client_sock,(void*) &legacy, 16, 0) < 0){
						perror("Failed to send message.\n");
					}
					legacy++;
					break;
				}
			}
		case BACKUP:
			switch(substate_b){
				case OPERATE_B:{
					//recv()
					sleep(1);// LAME, remove when select is introduced
					if(!recv(master_sock,(void*) &legacy, 16, 0)){
						heartbeat--;
					}
					else{
						heartbeat = 3;
					}

					if(!heartbeat){
						printf("Senpai is dead.\n");
						if(close(master_sock)){
							perror("Failed to close socket\n");
						}
						if(close(server_sock)){
							perror("Failed to close socket\n");
						}
						state = MASTER;
					}
					break;
				}
			}
	}
}
