#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

enum msg_id{
	M_ALIVE
};

struct msg_t{
	int sender;
	msg_id id;
	char content[1024];
};

class Socket_UDP{
public:
	Socket_UDP(int ip_addr, int port);
	Socket_UDP(struct sockaddr* dest_addr_in);
	~Socket_UDP();

	int pid;
	char buffer[1024]; //Change to private? Adjust size?

	msg_t recv();

private:
	struct sockaddr_in dest_addr; //Not a good name

	void sock_setup(); //Review
};

#endif
