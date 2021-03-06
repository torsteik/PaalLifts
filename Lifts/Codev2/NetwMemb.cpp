#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "NetwMemb.h"
#include "netw_fsm.h"

NetwMemb::NetwMemb(){
	role = NO_ROLE;
}

NetwMemb::NetwMemb(int recv_ip, int send_ip, int port, int init_role){
	//Create socket, conside using this->
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(port);
	bind_addr.sin_addr.s_addr = htonl(recv_ip);
	sock_setup();

	//Store sender
	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(port);
	send_addr.sin_addr.s_addr = htonl(send_ip);

	role = init_role;
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

RecvMsg NetwMemb::recv(){
	RecvMsg msg;
	struct sockaddr_in sender_addr;
	unsigned int sender_size = sizeof(sender_addr);
	if (recvfrom(sock_fd, msg.content, BUFF_SIZE, 0, (struct sockaddr*) &sender_addr, &sender_size) < 0){
		perror("Error receiving message.\n");
		msg.MSG_ID = ERROR;
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

void NetwMemb::send_heartbeat(){
	static clock_t prev_heartbeat = clock();
	if (clock() - prev_heartbeat > 0.5 / CLOCKS_PER_SEC){
		char heartbeat[BUFF_SIZE];
		heartbeat[0] = HEARTBEAT;
		if (this->role == SLAVE_ROLE){
			heartbeat[1] = this->floor;
			heartbeat[2] = this->dir;
			heartbeat[3] = this->elev_fsm_state;
		}
		else if (this->role == BROADCAST_ROLE){
		}
		this->send(heartbeat);
		prev_heartbeat = clock();
	}
}

RecvMsg NetwMemb::recv_and_ack(){
	RecvMsg recv_msg = this->recv();
	
	char send_msg[BUFF_SIZE];
	send_msg[0] = ACKNOWLEDGE;
	this->send(send_msg);
	
	return recv_msg;
}

int NetwMemb::send_and_get_ack(char send_msg[BUFF_SIZE]){
	int tries = 0;
	while (tries < 2){
		this->send(send_msg);

		struct timeval timeout;
		timeout.tv_sec  = 0;
		timeout.tv_usec = 0.2*SEC_TO_USEC;

		fd_set read_fd_set = file_descriptor_setup(&this->sock_fd, 1);
		if (select(this->sock_fd + 1, &read_fd_set, NULL, NULL, &timeout)){
			RecvMsg recv_msg = this->recv();
			if (recv_msg.MSG_ID == ACKNOWLEDGE){
				return 1;
			}
		}
		tries++;
	}
	return 0;
}