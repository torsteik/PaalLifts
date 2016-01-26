#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


int main(){

//Set up socket for sending
	int  destination_port = 30000;
	char destination_ip[] = "129.241.187.159";
	
	//Create socket
	int send_sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	//Create socket address
	struct sockaddr_in send_addr;
	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(destination_port);
	inet_aton("129.241.187.255", &send_addr.sin_addr); 
	
	//Allow for reuse
	int option_lvl = 1;
	if(setsockopt(send_sock, SOL_SOCKET, SO_REUSEADDR,(void*)&option_lvl, sizeof(option_lvl)) < 0){
		printf("setsockopt(SO_REUSEADDR) failed.\n");
	}
	if(setsockopt(send_sock, SOL_SOCKET, SO_REUSEPORT,(void*)&option_lvl, sizeof(option_lvl)) < 0){
		printf("setsockopt(SO_REUSEPORT) failed.\n");
	}
		if(setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST,(void*)&option_lvl, sizeof(option_lvl)) < 0){
		printf("setsockopt(SO_BROADCAST) failed.\n");
	}

	//Bind
	int bind_status_send = bind(send_sock, (struct sockaddr*) &send_addr, sizeof(send_addr));
	if(bind_status_send != 0){
		printf("Failed to bind sender.\nERROR: %s\n", strerror(errno));
	}
	
	char send_buffer[1024] = "Spam, egg, sausage and spam";

	char send_buffer1[1024] = 
"\n								MMMMMM=\n"
"                           .MMMMMMMMMMMMMM\n"
"                         MMMMMM         MMMM\n"
"                      MMMMM              MMMMM.\n"
"                MMMMMMMM                  ?MMMM.\n"
"             .MMMMMMMM7MM                  MMMMM\n"
"             MMMMMMMMM MM                   MMMM\n"
"             MMMMMMMMM .MM                   MMM\n"
"            .MMMMMMM.   MM.M.   =MMMMMMMM.   MMM\nV"
"             MMMMMMMM.MMMMMM.  MMMMMMMM MMM  MMM\n"
"             MMMMMMMMMMMM     MMMMMMMMM  MMM MMM\n"
"            MMM    MMM:  MM   MMMMMMMMM  MMM MMM\n";
char send_buffer2[1024] = 
"\n           8MM.    MMMMMMMM=  MMMMMMMM.  .M ,MM7\n"
"          MMMMMMMM..          MMMMM.     M  MMM\n"
"         .MMMMMMMMMMMMMMM       MMMMMMMMM.  MMM\n"
"        .MMM        MMMMMMMMMMM.           MMMM\n"
"        MMM                .~MMMMMMM.      MMM.\n"
"       MMM.                               MMMM\n"                             
"      .MMM                                MMMM\n"
"      MMM                                DMMM\n"
"     MMM                                 MMMM\n"
"     MMM                                .MMM\n"
"    MMM                      MM        .MMM\n";
char send_buffer3[1024] = 
"\n    MMM                     .MM  MM.   MMM~\n"
"    MMM                     MMM .MM.   MMM                         MMMMM \n"    
"    MMM                     MM. MMM   MMM                        MMM   MMO \n"  
"    MMM                     MM  MM   .MMM                      MMMM     MM \n"
"    MM~                    MM. MMM   MMM.                     MMM      IMM \n"  
"    MM.                    MM  MM.  .MMM                    .MMM.      MM  \n"  
"    MM                     M  MMN   .MMM                   MMMM       MM. \n"   
"    MM+                   MM MMM    .MM 7MMMMMMMMMMM      MMM.       MM \n"     
"    MMM                  .M  MM      MMMMMM. .. MMMMMMM MMM.       MMM. \n"    
"    MMM                  +M MMM      MM    .MM     .MMMMMM.      .MMM.  \n"     
"    ?MM                  M  MM       .      MMM      ~MM.       MMMM  \n"       
"    .MM                 MM  MM            ,MMMMMMMMMMM         MMMM \n";
char send_buffer4[1024] =      
"\n     MMM              MM     MM          MMMMMMMMMMMMMMM     MMMMM \n"          
"     MMM            .M     N  MM                  ..MMMMM.     MMMM \n"         
"     .MM             MMMM MMMMMM                     .MMMMM        DMM\n"       
"      MMM              MMMMM.MM.             ,M,       .MMM.        MM\n"
"       MMM               M.                  MMM         MMM.  .MMMM\n"        
"       MMMM                                              MMM.    MM \n"         
"        =MMM.                            MM.             MMM      MM\n"         
"          MMMM.                          MM              MMM       M\n"         
"            MMMM                                         MMMMM,. MMM\n"        
" .          MMMMMMM.                     MMM            MMM MMMMMM.\n"          
" MMMM      MMN  .MMMMM                               .MMMD\n"                   
" MM MMN  .MM    MM MMMMMMMM~ .              M.     .MMMM\n"                     
" MM   MM$MM   MMN      MMMMMMMMMM.          M.   .MMMM  \n";
char send_buffer5[1024] =                   
"\n  M.   MMM   MM              MMMMM  MMMMMMMMM~MMMMMM \n"                        
"  MM      .MMN          .. . +MM,  8MM .MMMMMMMM. \n"
"   M?    NMM           MMMMMMMMM   MM  \n"
"   .M  .MM7            MMM MMMM   MM \n"
"     MMMM              OMM       .MM\n"
"      .                 MMM      MM\n"
"                        .MMM    MM\n"
"                         .MMD  MMM \n"
"                           MMMMM?\n"
"                            MMM.\n";
//Set up socket for recieving
	int rec_port = 30000;
	
	//Create socket
	int rec_sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	//Create socket address
	struct sockaddr_in rec_addr;
	rec_addr.sin_family = AF_INET;
	rec_addr.sin_port = htons(rec_port);
	rec_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//inet_aton(destination_ip, &rec_addr.sin_addr); 
	
	//Allow for reuse
	if(setsockopt(rec_sock, SOL_SOCKET, SO_REUSEADDR,(void*)&option_lvl, sizeof(option_lvl)) < 0){
		printf("setsockopt(SO_REUSEADDR) failed.\n");
	}
	if(setsockopt(rec_sock, SOL_SOCKET, SO_REUSEPORT,(void*)&option_lvl, sizeof(option_lvl)) < 0){
		printf("setsockopt(SO_REUSEPORT) failed.\n");
	}



	//Bind
	int bind_status_rec = bind(rec_sock, (struct sockaddr*) &rec_addr, sizeof(rec_addr));
	if(bind_status_rec < 0){
		printf("Failed to bind receiver.\nERROR: %s\n", strerror(errno));
	}

	char rec_buffer[1024];
	
//Use socket
	while(1){
		
		//Clear buffer
		for(int i=0;i<40;i++){rec_buffer[i]=0;}
		
		//Recieve message
		if(recv(rec_sock, rec_buffer, 1024, 0)){
			printf("Message: %s\n", rec_buffer);
		}
		
		//Send message
		sleep(1);
		bool troll = false;
		if(troll){
			if(sendto(send_sock, send_buffer1, 1024, 0, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0){
				printf("Failed to send message.\n");
			}
			if(sendto(send_sock, send_buffer2, 1024, 0, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0){
				printf("Failed to send message.\n");
			}
			if(sendto(send_sock, send_buffer3, 1024, 0, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0){
				printf("Failed to send message.\n");
			}
			if(sendto(send_sock, send_buffer4, 1024, 0, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0){
				printf("Failed to send message.\n");
			}
			if(sendto(send_sock, send_buffer5, 1024, 0, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0){
				printf("Failed to send message.\n");
			}
		}
		else{
			if(sendto(send_sock, send_buffer, 1024, 0, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0){
				printf("Failed to send message.\n");
			}
		}
	}
	return 0;
}
