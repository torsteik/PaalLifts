#include <string.h>
#include <sys/select.h>

#include "defines.h"

struct SharedVars* shared;

SharedVars::SharedVars(){
	this->recovered.store(0);
	for(int i = 0; i < N_FLOORS * 2; ++i){
		netw_master_q[i].store(0);
	}
	this->backup = 0;
}

void file_descriptor_setup(int* sock_fd_set, fd_set* read_fd_set){ 
	FD_ZERO(read_fd_set);
	for (int i = 0; i < sizeof(sock_fd_set); ++i){
		FD_SET(sock_fd_set[i], read_fd_set);
	}
}