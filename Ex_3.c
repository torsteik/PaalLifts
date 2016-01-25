#include <sys/socket.h>


int main(){
	int port_nr 	= 30000;
	char* ip_addr 	= "";

//Socket set up:
	struct sockaddr_in our_addr;
	our_addr.sin_family 	= AF_INET;		//IPv4
	our_addr.sin_port	= htons(port_nr);	//Port nummer og konv til internet byte
	inet_aton(ip_addr, &our_addr.sin_addr.s_addr);	//IP address
	
	int our_sock = socket(AF_INET, SOCK_STREAM, 0);	//Tilrettelegger for en socket
	bind(our_sock,(struct sockaddr*) &our_addr, sizeof(our_addr));	//Gir den navnløse socketen, laget med socket(), en identitet via en "sockaddr" type
	
}
