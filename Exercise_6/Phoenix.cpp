#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>


enum state_t {MASTER, BACKUP};
enum substate_m_t {CREATE_BACKUP, OPERATE_M};
enum substate_b_t {SOCK_SETUP, OPERATE_B};

int main(){
	// Cannot use same port for backup as the one master had(yes you can, just close it)
	int port = 33327;
	char ip_addr[] ="129.241.187.159";
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

// TCP server()
	//Socket
	int server_sock = socket(AF_INET, (SOCK_STREAM|SOCK_NONBLOCK), 0);

	//Address
	struct sockaddr_in server_addr;
	bzero((char *) &server_addr, sizeof(server_addr)); // Get to know this guy
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;		   // and this guy.
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
	//Bind
	if(bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		perror("Failed to bind server socket.\n");
		return 0;
	}
	else{ printf("Bind successfull.\n"); }
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
	substate_b_t substate_b = SOCK_SETUP;
	
	while(1000){
		switch(state){
			case MASTER:{
				switch(substate_m){
					case CREATE_BACKUP:{
						// Close server
						if(close(server_sock)){
							perror("Failed to close socket\n");
						}
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
							sleep(1);
							if(connect(client_sock, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0){
								perror("Failed to connect socket.\n");
							}
							else{
								//printf("Connect successfull\n");
							}
							substate_m = OPERATE_M;
						}
						printf("Backup created\n");
						break;
					}
					case OPERATE_M:{
						sleep(1);// LAME, remove when select is introduced
						printf("Legacy: %i\n", legacy);
						if(send(client_sock,(void*) &legacy, 16, 0) < 0){
							perror("Failed to send message.\n");
						}
						legacy++;
						break;
					}
				}
				break;
			}
			case BACKUP:{
				switch(substate_b){
					case SOCK_SETUP:{
						if(setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
							perror("setsockopt(SO_REUSEADDR) failed.\n");
						}
						struct timeval tv;
						tv.tv_sec = 1;
						tv.tv_usec = 0;
						if(setsockopt(master_sock, SOL_SOCKET, SO_RCVTIMEO,(struct timeval*) &tv, sizeof(struct timeval)) < 0){
							perror("setsockopt(SO_RCVTIMEO) failed.\n");
						}
						substate_b = OPERATE_B;
						break;
					}
					case OPERATE_B:{
						//sleep(1);// LAME, remove when select is introduced
						if(!recv(master_sock,(void*) &legacy, 16, 0)){
							heartbeat--;
						}
						else{
							heartbeat = 3;
						}
						printf("Backed up: %i\n", legacy);
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
				break;
			}
		}
	}
}
