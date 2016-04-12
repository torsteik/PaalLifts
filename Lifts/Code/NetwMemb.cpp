#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

#include "NetwMemb.h"

NetwMemb::NetwMemb(){
	role = EMPTY_SOCK;
}


NetwMemb::NetwMemb(int recv_ip, int send_ip, int port, int init_state){
	//Create socket, conside using this->
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(port);
	bind_addr.sin_addr.s_addr = htonl(recv_ip);
	sock_setup();

	//Store sender
	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(port);
	send_addr.sin_addr.s_addr = htonl(send_ip);

	role = init_state;
}


void NetwMemb::sock_setup(){
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	int option_value = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (void*)&option_value, sizeof(option_value)) < 0)
		perror("setsockopt(SO_BROADCAST) failed.\n");
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&option_value, sizeof(option_value)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed.\n");
	if (bind(sock_fd, (struct sockaddr*) &bind_addr, sizeof(bind_addr)))
		perror("Failed to bind socket.\n");
}

msg_t NetwMemb::recv(){
	msg_t msg;
	struct sockaddr_in sender_addr; 
	if (recvfrom(sock_fd, msg.content, BUFF_SIZE, 0, (struct sockaddr*) &sender_addr, sizeof(sender_addr)) < 0){
		perror("Error receiving message.\n");
		msg.content[0] = ERROR;
	}
	msg.sender_ip = ntohl(sender_addr.sin_addr.s_addr);
	return msg;
}

int NetwMemb::send(char msg_content[BUFF_SIZE]){
	if (sendto(sock_fd, msg_content, BUFF_SIZE, 0, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0){
		perror("Error sending message.\n");
		return -1;
	}
	return 0;
}