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
	int byung_port = 12000;

	char server_ip_addr[] = "129.241.187.23";;
	char my_ip_addr[] = "129.241.187.159";
	char byung_ip_addr[] = "129.241.187.142";

//Set up socket(client?)

	//Socket
	int client_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Address
	struct sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = server_port2;
	inet_aton(server_ip_addr, &client_addr.sin_addr);

	//Socket settings
	int opt_val = 1;
	if(setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR,(void*) &opt_val, sizeof(opt_val)) < 0){
		printf("setsockopt(SO_REUSEADDR) failed.\n");
	}


	char client_buffer[1024] = "Wololo!"; 

//Put it to use

	//Connect client to server
	if(connect(client_sock, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0){
		printf("Failed to connect socket.\nERROR: %s\n", strerror(errno));
	}
	while(1){

		//Send from client to server
		if (send(client_sock, client_buffer, 1024, 0) < 0){
			printf("Failed to send message.\n");
		}

		

		if (recv(client_sock, client_buffer, 1024, 0)){
			printf("Message received as client:\t%s\n", client_buffer);
		}
		sleep(1);
	}
}
