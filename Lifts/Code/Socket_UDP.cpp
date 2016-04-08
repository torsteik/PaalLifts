#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Socket_UDP.h"

Socket_UDP::Socket_UDP(int ip_addr, int port){
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = htonl(ip_addr);
	sock_setup();
}

Socket_UDP(struct sockaddr* dest_addr_in){
	&dest_addr = (struct sockaddr_in*) dest_addr_in;
	sock_setup();
}

void Socket_UDP::sock_setup(){ //Might be better to change the name as this sounds like the purpose of the constructor
	pid = socket(AF_INET, (SOCK_DGRAM|SOCK_NONBLOCK), 0);
	int option_value = 1; //Can prob simplify this
	if(setsockopt(pid, SOL_SOCKET, SO_BROADCAST,(void*)&option_value, sizeof(option_value)) < 0)
		perror("setsockopt(SO_BROADCAST) failed.\n");
	if(setsockopt(pid, SOL_SOCKET, SO_REUSEADDR,(void*)&option_value, sizeof(option_value)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed.\n");
	if(bind(pid,(struct sockaddr*) &dest_addr, sizeof(dest_addr)))
		perror("Failed to bind socket.\n");
}
