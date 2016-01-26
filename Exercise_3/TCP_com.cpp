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
	int my_port = 20015;		//workspace value

	char broadc_ip_addr[] = "129.241.187.255";;
	char my_ip_addr[] = "129.241.187.159";

//Set up socket(server?)

	//Socket
	int server_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = my_port;
	inet_aton(my_ip_addr, &server_addr.sin_addr);

	//Socket settings
	int opt_val = 1;
	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
		printf("setsockopt(SO_REUSEADDR) failed.\n");
	}

	//Bind
	if(bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		printf("Failed to bind server socket.\nERROR: %s\n", strerror(errno));
	}

//Set up empty address to be filled by client connecting to server
	struct sockaddr_in con_client_addr;
	con_client_addr.sin_family = AF_INET;

//Set up socket(client?)

	//Socket
	int client_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Address
	struct sockaddr_in client_addr;
	client_addr.sin_family = PF_INET;
	client_addr.sin_port = my_port;
	inet_aton(my_ip_addr, &client_addr.sin_addr);



	char client_buffer[1024] = "Wololo!"; 
	char server_buffer[1024] = "Pan comido!";


//Put it to use

	//Server starts to listen
	if(!listen(server_sock, 1)){
		printf("Successfully put socket in connection-mode\n");
	}

	//Connect client to server
	if(connect(client_sock, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0){
		printf("Failed to connect socket.\nERROR: %s\n", strerror(errno));
	}

	//Accept connection request
	unsigned int con_client_length;
	int con_client_descriptor = accept(server_sock, (struct sockaddr*) &con_client_addr, &con_client_length);
	if( con_client_descriptor < 0){
		printf("Failed to accept connection.\nERROR: %s\n", strerror(errno));
	}

	//Send from client to server
	if (send(client_sock, client_buffer, 1024, 0) < 0){
		printf("Failed to send message.\n");
	}

	//Server reads message and responds
	sleep(1);
	if (recv(server_sock, server_buffer, 1024, 0)){
		printf("Message received:\t%s\n", server_buffer);
	}

	if (sendto(server_sock, server_buffer, 1024, 0, (struct sockaddr*) &con_client_addr, sizeof(con_client_addr)) < 0){
		printf("Failed to send message.\nERROR: %s\n", strerror(errno));
	}

	//Receive response as client
	if (recv(client_sock, client_buffer, 1024, 0)){
		printf("Message received:\t%s\n", client_buffer);
	}	
	return 0;
}

//Smooth kompileringskommando: clear; clang++-3.6 -Wall -g -fsanitize=address udp.cpp
