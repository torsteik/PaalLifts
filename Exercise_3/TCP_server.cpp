#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


int main(){

	int server_port1 = 33546;	//0-terminated
	int server_port2 = 34933;	//fixed size
	int my_port = 20011;		//workspace value

	char server_ip_addr[] = "129.241.187.23";;
	char my_ip_addr[] = "129.241.187.159";

//Set up socket(client?)

	//Socket
	int client_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Address
	struct sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(server_port2);
	inet_aton(server_ip_addr, &client_addr.sin_addr);


	//Socket settings
	int opt_val = 1;
	if(setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
		printf("setsockopt(SO_REUSEADDR) failed.\n");
	}


	char client_buffer[1024] = "Connect to: 129.241.187.159:20011"; 

//Put it to use

	//Connect client to server
	if(connect(client_sock, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0){
		printf("Failed to connect socket.\nERROR: %s\n", strerror(errno));
	}

	//Send from client to server
	if (send(client_sock, client_buffer, 1024, 0) < 0){
		printf("Failed to send message.\n");
	}


//Set up socket(server?)

	//Socket
	int server_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(my_port);

	//Socket settings
	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
		printf("setsockopt(SO_REUSEADDR) failed.\n");
	}

	//Bind
	if(bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		printf("Failed to bind server socket.\nERROR: %s\n", strerror(errno));
	}

	//Start listening
	if( listen(server_sock, 1) < 0){
		printf("Failed to listen.\nERROR: %s\n", strerror(errno));
	}

	//Set up empty address to be filled by client connecting to server
	int con_client_sock;
	unsigned int con_client_len;
	struct sockaddr_in con_client_addr;
	con_client_addr.sin_family = AF_INET;


	//Action!
	char con_client_buffer[1024];
	char server_response_buffer[1024] = "Wololo!";
	while(1){
		con_client_sock = accept(server_sock, (struct sockaddr*) &con_client_addr, &con_client_len);

		//Check for responses
		if(recv(con_client_sock, con_client_buffer, 1024, 0) > 0){
			printf("Message received from connected client:\n\t%s\n", con_client_buffer);
		
			//Reply
			if(sendto(con_client_sock, server_response_buffer, 1024, 0, (struct sockaddr*) &con_client_addr, sizeof(con_client_addr)) > 0){
				printf("Message sent to connected client\n\t%s\n", server_response_buffer);
			}
		}
	}
	return 0;
}
