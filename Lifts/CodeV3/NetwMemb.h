#ifndef NETWMEMB_H
#define NETWMEMB_H

#include <netinet/in.h>
#include <sys/select.h>

#include "defines.h"

unsigned long id_to_ip(char id);
char ip_to_id(unsigned long ip);

struct RecvMsg{
	int sender_ip;
	char content[BUFF_SIZE];
};

class NetwMemb{
public:
	NetwMemb();
	NetwMemb(int bind_ip, int send_ip, int port, int init_state);
	
	// Network
	int sock_fd;
	struct sockaddr_in bind_addr;
	struct sockaddr_in send_addr;
	std::atomic<int> netw_role;
	
	// Elevator 
	std::atomic<int> elev_fsm_state;
	std::atomic<int> floor;
	std::atomic<int> dir;

	void sock_setup();
	struct RecvMsg recv();
	int send(char msg_content[BUFF_SIZE]);
	int send_and_get_ack(char msg_content[BUFF_SIZE]);
	void send_heartbeat(char netw_master_q[N_FLOORS * 2]);
};

#endif // ERROR???