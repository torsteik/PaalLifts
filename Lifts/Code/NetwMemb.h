#ifndef NETWMEMB_H
#define NETWMEMB_H

#include <netinet/in.h>
#include <stdint.h>

// Do these defines belong here?
#define BROADCAST		255						//0x40BC576FF equals 129.241.187.255
#define NETW_INFO_PORT	32123
#define ELEV_INFO_PORT	32124

#define BUFF_SIZE		1024

#define MSG_ID			(int)msg.content[0]				//Consider making enum && move to network_fsm.h, delete?
#define ERROR			0
#define	HEARTBEAT		1								//Consider making this 3 so it sends hearts, could have slave send this as well
#define CONNECT			2
#define ACCEPT_CON		3
#define BECOME_BACKUP	4
#define BACKUP_DATA		5
#define S_ALIVE			6
#define NEW_ORDER		7
#define COMPLETED_ORDER	8
#define NO_RESPONSE		9

#define EMPTY_SOCK		0
#define SLAVE_SOCK		1
#define BACKUP_SOCK		2
#define MASTER_SOCK		3								
#define BROADCASTER		4								// Change this name? Join this and BROADCAST

unsigned long id_to_ip(uint8_t id){ return 0x40BC57600 + id; }
uint8_t ip_to_id(unsigned long ip){ return ip - 0x40BC57600; }

struct msg_t{											// Maybe change the name to something recv like
	int sender_ip;
	char content[BUFF_SIZE];
};

class NetwMemb{
public:
	NetwMemb();
	NetwMemb(int recv_ip, int send_ip, int port, int init_state);// Maybe make 3rd option where there is no recv ip so no recv addr is created
	~NetwMemb();										//Maybe this would be cool
	void sock_setup();
	msg_t recv();
	int send(char msg_content[BUFF_SIZE]);
	
	int sock_fd;
	struct sockaddr_in bind_addr;						//Could be private
	struct sockaddr_in send_addr;
	int role;											//Is this needed?
	int floor;
	int dir;
};

#endif