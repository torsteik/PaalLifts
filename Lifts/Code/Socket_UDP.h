#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

#define BUFF_SIZE	1024

struct msg_t{ //Make a member of Socket_UDP
	int sender_ip;
	char content[BUFF_SIZE];
};

class Socket_UDP{
public:
	Socket_UDP();
	Socket_UDP(int recv_ip, int send_ip, int port);
	Socket_UDP(struct sockaddr* dest_addr_in); //Remove, but maybe make 3rd option where there is no recv ip so no recv addr is created
	~Socket_UDP();

	int pid;
	char buffer[BUFF_SIZE]; //Change to private? Adjust size?

	msg_t recv();
	int send(char content[BUFF_SIZE]);

private:
	struct sockaddr_in recv_addr; //Consider changing name to bind_addr or something
	struct sockaddr_in send_addr;

	void sock_setup(); //Review
};

#endif
