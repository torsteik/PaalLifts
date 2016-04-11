#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

#define BUFF_SIZE	1024

#define EMPTY_SOCK		0
#define SLAVE_SOCK		1
#define BACKUP_SOCK		2
#define MASTER_SOCK		3
#define BROADCAST_SOCK	4

struct msg_t{ //Make a member of Socket_UDP
	int sender_ip;
	char content[BUFF_SIZE];
};


class Socket_UDP{
public:
	Socket_UDP();
	Socket_UDP(int recv_ip, int send_ip, int port, int init_state);
	Socket_UDP(struct sockaddr* dest_addr_in); //Remove, but maybe make 3rd option where there is no recv ip so no recv addr is created
	~Socket_UDP();

	int state; //state might not be a good name as it's not really a state, more like a 'who'
	int pid;	// OBS! It should be called fd not pid
	char buffer[BUFF_SIZE]; //Change to private? Adjust size?

	int floor;
	int dir;

	msg_t recv();
	int send(char content[BUFF_SIZE]);

private:
	struct sockaddr_in recv_addr; //Consider changing name to bind_addr or something
	struct sockaddr_in send_addr;

	void sock_setup(); //Review
};

#endif
