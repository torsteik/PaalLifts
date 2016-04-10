// Network
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// Other
#include <stdio.h>
#include <errno.h>

#include "Socket_UDP.h"

Socket_UDP::Socket_UDP(){
	state = EMPTY_SOCK;
}


Socket_UDP::Socket_UDP(int recv_ip, int send_ip, int port, int init_state){
	//Create socket
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = htons(port);
	recv_addr.sin_addr.s_addr = htonl(recv_ip);
	sock_setup();

	//Store sender
	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(port);
	send_addr.sin_addr.s_addr = htonl(send_ip);

	state = init_state;
}


void Socket_UDP::sock_setup(){ //Might be better to change the name as this sounds like the purpose of the constructor
	pid = socket(AF_INET, SOCK_DGRAM, 0); //Try avoiding NONBLOCK 
	int option_value = 1; //Can prob simplify this
	if (setsockopt(pid, SOL_SOCKET, SO_BROADCAST, (void*)&option_value, sizeof(option_value)) < 0)
		perror("setsockopt(SO_BROADCAST) failed.\n");
	if (setsockopt(pid, SOL_SOCKET, SO_REUSEADDR, (void*)&option_value, sizeof(option_value)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed.\n");
	if (bind(pid, (struct sockaddr*) &recv_addr, sizeof(recv_addr)))
		perror("Failed to bind socket.\n");

	functional = 1;
}

msg_t Socket_UDP::recv(){
	msg_t msg;
	struct sockaddr_in sender_addr; //OBS might be a bad name considering send_addr already exists
	
	if (recvfrom(pid, buffer, BUFF_SIZE, 0, (struct sockaddr*) &sender_addr, sizeof(sender_addr)) < 0){
		perror("Error receiving message.\n");
		*msg.content = NULL;
		return nullptr; //OBS review
	}
	msg.sender_ip = ntohl(sender_addr.sin_addr.s_addr);
	msg.content = buffer;
	return msg_t;
}

int Socket_UDP::send(char content[BUFF_SIZE]){
	if (sendto(pid, content, BUFF_SIZE, 0, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0){
		perror("Error sending message.\n");
		return -1;
	}
	return 0;
}
