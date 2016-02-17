#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>


enum state_t {MASTER, BACKUP};
enum substate_m_t {CREATE_BACKUP, OPERATE_M};
enum substate_b_t {SOCK_SETUP, OPERATE_B};

int main(int argr, char* argv[]){
	pid_t my_pid = getpid();
	pid_t dads_pid = getppid();
	printf("Hi there, my name is %i and my dads name is %i.\n", my_pid, dads_pid);
	int port = 30003;
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
		printf("setsockopt(SO_REUSEADDR) failed for client_sock.\n");
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
	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
		perror("setsockopt(SO_REUSEADDR) failed for server_sock.\n");
	}
	//Bind
	if(bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		perror("Failed to bind server socket.\n");
		return 0;
	}

	//Start listening
	if(listen(server_sock, 1) < 0){
		perror("Failed to listen.\n");
	}
	else{
		printf("%c says: Listening\n", argv[1][0]);
	}

// FSM
	uint16_t legacy = 1;
	uint8_t heartbeat = 3;
	
	// Make socket shell for potential master
	int master_sock;
	unsigned int master_len;
	struct sockaddr_in master_addr;
	master_addr.sin_family = AF_INET;

	// Determine state
	state_t state;

	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	
	fd_set readfds;
	fd_set writefds;

	FD_ZERO(&readfds);
	FD_SET(server_sock, &readfds);

	int sret = select(server_sock +1, &readfds, NULL, NULL, &timeout);
	if(sret > 0){
		printf("%c says: Found connection request.\n", argv[1][0]);
		master_sock = accept(server_sock,(struct sockaddr*) &master_addr, &master_len);
		state = BACKUP;
	}
	else if(!sret){ 
		printf("%c says: Did not find connection request\n", argv[1][0]);
		state = MASTER;
	}
	else{ perror("Failed to complete accept select.\n");	}

	substate_m_t substate_m = CREATE_BACKUP;
	substate_b_t substate_b = SOCK_SETUP;

	// Go time
	while(1){
		switch(state){
			case MASTER:{
				switch(substate_m){
					case CREATE_BACKUP:{
						// Close server and remove its FD, also add client_sock's FD
						if(close(server_sock)){
							perror("Failed to close socket\n");
						}
						FD_ZERO(&readfds);
						FD_ZERO(&writefds);
						FD_SET(client_sock, &writefds);
						timeout.tv_sec = 1;
						// Spawn new process
						pid_t pid = fork();
						if(!pid){
							printf("Wouaaa wouaaa wu wu wouaaaaaaaaaa!\n");
							// Restart child process
							int new_pid = int(argv[1][0]) + 1;
							char new_pid_ch = char(new_pid);
							if(execl("Phoenix", "Phoenix", &new_pid_ch, (char*)NULL) < 0){
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
							printf("%c says: It's a boy!\n", argv[1][0]);
							sleep(1);
							printf("%c says: Sending connection request\n", argv[1][0]);
							if(connect(client_sock, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0){
								perror("Failed to connect socket.\n");
							}
							else{
								printf("%c says: Connection request sent\n", argv[1][0]);
							}
							substate_m = OPERATE_M;
						}
						break;
					}
					case OPERATE_M:{
						sleep(1); // No DDOS please
						select(client_sock +1, NULL, &writefds, NULL, &timeout);
						printf("%c says: Legacy: %i\n", argv[1][0], legacy);
						if(send(client_sock,(void*) &legacy, sizeof(uint16_t), 0) < 0){
							perror("Failed to send message.\n");
						}
						else{ legacy++; }
						break;
					}
				}
				break;
			}
			case BACKUP:{
				switch(substate_b){
					case SOCK_SETUP:{
						// Set up master_sock and replace server FD with master FD
						if(setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
							perror("setsockopt(SO_REUSEADDR) failed for master_sock.\n");
						}
						FD_ZERO(&readfds);
						FD_SET(master_sock, &readfds);
						timeout.tv_sec = 3;

						substate_b = OPERATE_B;
						break;
					}
					case OPERATE_B:{
						sleep(1);
						sret = select(master_sock +1, &readfds, NULL, NULL, &timeout);
						if(!sret){
							printf("Failed to receive.\n");
							heartbeat--;
						}
						else if(sret > 0){
							recv(master_sock,(void*) &legacy, sizeof(uint16_t), 0);
							heartbeat = 3;
						}
						else{
							perror("Failed to complete receive select\n");
						}
						
						if(!heartbeat){
							printf("Senpai is dead.\n");
							if(close(master_sock)){
								perror("Failed to close master_socket\n");
							}
							if(close(server_sock)){
								perror("Failed to close server_socket\n");
							}
							state = MASTER;
							legacy++;
						}
						break;
					}
				}
				break;
			}
		}
	}
}
