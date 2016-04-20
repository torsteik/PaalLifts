#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "defines.h"
#include "NetwMemb.h"

NetwMemb::NetwMemb(){
	this->netw_role = NO_ROLE;
	this->elev_fsm_state = 0;
	this->floor = 0;
	this->dir = 0;
}

NetwMemb::NetwMemb(unsigned long bind_ip, unsigned long send_ip, int port, int init_role){
	//Create socket
	int broad = 0;
	if((bind_ip & 0xFF) == 255){
		broad = 1;
	}

	this->bind_addr.sin_family = AF_INET;
	this->bind_addr.sin_port = htons(port);
	printf("Our newest member: %lu\nNow known as: %i\n", 0xFFFFFFFF & bind_ip, ip_to_id(bind_ip));
	if(broad){
		this->bind_addr.sin_addr.s_addr = 0xFFFFFFFF & ntohl(bind_ip);
	}
	this->sock_setup(broad);

	//Store sender
	this->send_addr.sin_family = AF_INET;
	this->send_addr.sin_port = htons(port);
	this->send_addr.sin_addr.s_addr = 0xFFFFFFFF & htonl(send_ip);

	this->netw_role = init_role;
}

void NetwMemb::sock_setup(int broad){
	this->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	int option_value = 1;
	if(broad){
		printf("hei\n");
		if (setsockopt(this->sock_fd, SOL_SOCKET, SO_BROADCAST, (void*)&option_value, sizeof(option_value)) < 0)
			perror("setsockopt(SO_BROADCAST) failed.\n");
	}
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
	printf("Receiving nudes: %i\n", msg.MSG_ID);
	printf("IP raw: %u\n", sender_addr.sin_addr.s_addr);
	msg.sender_ip = 0xFFFFFFFF & ntohl(sender_addr.sin_addr.s_addr);
	return msg;
}

int NetwMemb::send(char msg_content[BUFF_SIZE]){
	printf("Sending: %i\n", msg_content[0]);
	if (sendto(this->sock_fd, msg_content, BUFF_SIZE, 0, (struct sockaddr*) &(this->send_addr), sizeof(this->send_addr)) < 0){
		perror("Error sending message.\n");
		return -1;
	}
	return 0;
}

int NetwMemb::send_and_get_ack(char msg_content[BUFF_SIZE]){
	int tries = 0;
	while (tries < 2){
		this->send(msg_content);
		struct timeval timeout;
		timeout.tv_sec  = 0;
		timeout.tv_usec = 0.2*SEC_TO_USEC; // Might be too short

		fd_set read_fd_set;
		file_descriptor_setup(&(this->sock_fd), 1, &read_fd_set);
		if (select(this->sock_fd + 1, &read_fd_set, NULL, NULL, &timeout)){
			struct RecvMsg recv_msg = this->recv();
			if (recv_msg.MSG_ID == ACKNOWLEDGE){
				return 1;
			}
		}
		tries++;
	}
	return 0;
}

void NetwMemb::send_heartbeat(char netw_master_q[N_FLOORS * 2], struct timeval timeout_leftovers){
	static unsigned long prev_heartbeat = 0; // Maybe this needs to be a member instead
	
	prev_heartbeat += (0.2*SEC_TO_USEC - timeout_leftovers.tv_usec);
	
	if (prev_heartbeat > 0.5*SEC_TO_USEC){
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
		prev_heartbeat = 0;
	}
}
