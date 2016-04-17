#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "NetwMemb.h"


unsigned long id_to_ip(char id){ return id + 0x40BC57600; }
char ip_to_id(unsigned long ip){ return ip - 0x40BC57600; }

NetwMemb::NetwMemb(){
	this->netw_role = NO_ROLE;
}

NetwMemb::NetwMemb(int bind_ip, int send_ip, int port, int init_role){
	//Create socket
	this->bind_addr.sin_family = AF_INET;
	this->bind_addr.sin_port = htons(port);
	this->bind_addr.sin_addr.s_addr = htonl(bind_ip);
	this->sock_setup();

	//Store sender
	this->send_addr.sin_family = AF_INET;
	this->send_addr.sin_port = htons(port);
	this->send_addr.sin_addr.s_addr = htonl(send_ip);

	this->netw_role = init_role;
}

void NetwMemb::sock_setup(){
	this->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

	int option_value = 1;
	if (setsockopt(this->sock_fd, SOL_SOCKET, SO_BROADCAST, (void*)&option_value, sizeof(option_value)) < 0)
		perror("setsockopt(SO_BROADCAST) failed.\n");
	if (setsockopt(this->sock_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&option_value, sizeof(option_value)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed.\n");
	if (bind(this->sock_fd, (struct sockaddr*) &(this->bind_addr), sizeof(this->bind_addr)))
		perror("Failed to bind socket.\n");
}

struct RecvMsg NetwMemb::recv(){
	struct RecvMsg msg;
	struct sockaddr_in sender_addr;
	unsigned int sender_size = sizeof(sender_addr);
	if (recvfrom(this->sock_fd, msg.content, BUFF_SIZE, 0, (struct sockaddr*) &sender_addr, &sender_size) < 0){
		perror("Error receiving message.\n");
		msg.MSG_ID = ERROR;
	}
	msg.sender_ip = ntohl(sender_addr.sin_addr.s_addr);
	return msg;
}

int NetwMemb::send(char msg_content[BUFF_SIZE]){
	if (sendto(this->sock_fd, msg_content, BUFF_SIZE, 0, (struct sockaddr*) &(this->send_addr), sizeof(this->send_addr)) < 0){
		perror("Error sending message.\n");
		return -1;
	}
	return 0;
}

int NetwMemb::send_and_get_ack(char send_msg[BUFF_SIZE]){
	int tries = 0;
	while (tries < 2){
		this->send(send_msg);

		struct timeval timeout;
		timeout.tv_sec  = 0;
		timeout.tv_usec = 0.2*SEC_TO_USEC; // Might be too short

		fd_set read_fd_set;
		file_descriptor_setup(&(this->sock_fd), &read_fd_set);
		if (select(this->sock_fd + 1, &read_fd_set, NULL, NULL, &timeout)){
			struct RecvMsg recv_msg = this->recv();
			if (recv_msg.MSG_ID == ACKNOWLEDGE){ // else print for debug
				return 1;
			}
		}
		tries++;
	}
	return 0;
}

void NetwMemb::send_heartbeat(std::atomic<char> netw_master_q[N_FLOORS * 2]){
	static clock_t prev_heartbeat = clock(); // Maybe this needs to be a member instead
	if (clock() - prev_heartbeat > 0.5 / CLOCKS_PER_SEC){
		char heartbeat[BUFF_SIZE];
		heartbeat[0] = HEARTBEAT;
		if (this->netw_role == SLAVE_ROLE){
			heartbeat[1] = this->floor;
			heartbeat[2] = this->dir;
			heartbeat[3] = this->elev_fsm_state;
		}
		else if (this->netw_role == BROADCAST_ROLE){
			for(int button = 0; button < N_FLOORS * 2; ++button){
				heartbeat[1 + button] = netw_master_q[button];
			}
		}
		this->send(heartbeat);
		prev_heartbeat = clock();
	}
}