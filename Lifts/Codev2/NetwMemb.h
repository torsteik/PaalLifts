#ifndef NETWMEMB_H
#define NETWMEMB_H

#include <netinet/in.h>
#include <stdint.h>

//------NETWORK ADDRESS------
#define MY_ID			0
#define BROADCAST		255						//0x40BC576FF equals 129.241.187.255
#define NETW_INFO_PORT	32123
#define ELEV_INFO_PORT	32124
//------MSG FORMAT-----------
#define BUFF_SIZE		1024
#define MSG_ID			content[0]				//Consider making enum && move to network_fsm.h, delete?
#define ERROR			1
#define NO_RESPONSE		2
#define	HEARTBEAT		3
#define CONNECT			4
#define ACCEPT_CON		5
#define TOGGLE_BACKUP	4
#define BACKUP_DATA		5
#define NEW_ORDER		7
#define COMPLETED_ORDER	8
//------NETW MEMB ROLES------
#define NO_ROLE			0
#define SLAVE_ROLE		1
#define BACKUP_ROLE		2
#define MASTER_ROLE		3
#define BROADCAST_ROLE	4

unsigned long id_to_ip(uint8_t id){ return 0x40BC57600 + id; }
uint8_t ip_to_id(unsigned long ip){ return ip - 0x40BC57600; }

struct RecvMsg{											// Maybe change the name to something recv like
	int sender_ip;
	char content[BUFF_SIZE];
};

class NetwMemb{
public:
	NetwMemb();
	NetwMemb(int recv_ip, int send_ip, int port, int init_state);// Maybe make 3rd option where there is no recv ip so no recv addr is created
	~NetwMemb();										//Maybe this would be cool
	void sock_setup();
	RecvMsg recv();
	int send(char msg_content[BUFF_SIZE]);
	void send_heartbeat();
	
	int sock_fd;
	struct sockaddr_in bind_addr;						//Could be private
	struct sockaddr_in send_addr;
	int role;											//prefix netw?
	int elev_fsm_state;
	int floor;
	int dir;
};

#endif
