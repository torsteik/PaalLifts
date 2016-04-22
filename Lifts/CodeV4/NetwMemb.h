#ifndef NETWMEMB_H
#define NETWMEMB_H

#include <netinet/in.h>
#include <sys/select.h>

#include "defines.h"

struct RecvMsg{
	int sender_ip;
	char content[BUFF_SIZE];
};

class NetwMemb{
public:
	NetwMemb();
	NetwMemb(unsigned long bind_ip, unsigned long send_ip, int port, int init_state);

	// Network
	int sock_fd;
	struct sockaddr_in bind_addr;
	struct sockaddr_in send_addr;
	int netw_role;
	
	// Elevator 
	int elev_fsm_state;
	int floor;
	int dir;

	void sock_setup(int broad);
	struct RecvMsg recv();
	int send(char msg_content[BUFF_SIZE]);
	int send_and_get_ack(char msg_content[BUFF_SIZE]);
	void send_heartbeat(char netw_master_q[N_FLOORS * 2], struct timeval timeout_leftovers);

};
#endif // ERROR???