#include <stdio.h>
#include <sys/select.h>
#include <time.h>

#include "main.h"
#include "netw_fsm.h"
#include "netw_slave.h"
#include "NetwMemb.h"

int serve(SharedVars* shared, unsigned long master_ip){

	shared->netw_membs[ip_to_id(master_ip)] = NetwMemb(master_ip, master_ip, ELEV_INFO_PORT, MASTER_ROLE);
	int sock_fd_set[2] = { shared->netw_membs[BROADCAST].sock_fd, shared->netw_membs[ip_to_id(master_ip)].sock_fd };

	int lives = 3;
	clock_t prev_heartbeat = clock();
	while (1){
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 400000;
		fd_set read_fd_set = select_setup(sock_fd_set, 2); //Keep an eye on these
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout) > -1){

			/* Check broadcast socket */
			msg_t broadcast_msg;
			if (FD_ISSET(shared->netw_membs[BROADCAST].sock_fd, &read_fd_set)){
				broadcast_msg = shared->netw_membs[BROADCAST].recv();
			} else{
				broadcast_msg.MSG_ID = NO_RESPONSE;
			}
			/* Check master socket */
			msg_t master_msg;
			if (FD_ISSET(shared->netw_membs[ip_to_id(master_ip)].sock_fd, &read_fd_set)){
				master_msg = shared->netw_membs[ip_to_id(master_ip)].recv();
			} else{
				master_msg.MSG_ID = NO_RESPONSE;
			}
			/* Read broadcasted message */
			if (broadcast_msg.MSG_ID == HEARTBEAT){
				lives = 3;
				prev_heartbeat = clock();
			} else if (broadcast_msg.MSG_ID == NO_RESPONSE){
				if ((clock() - prev_heartbeat) / CLOCKS_PER_SEC > 0.5){
					lives--;
					if (!lives){
						if (!shared->backup){
							return FSM_FIND_NETWORK;
						} else{
							return FSM_MASTER;
						}
					}
				}
			}
			/* Read message from master */
			if (master_msg.MSG_ID == NEW_ORDER){
				shared->local_q[master_msg.content[1]] = 1;
				//Send ack
			} else if (master_msg.MSG_ID == BECOME_BACKUP){
				shared->backup = -1;
			} else if (master_msg.MSG_ID == BACKUP_DATA){
				for (int button = 0; button < N_FLOORS; button++){
					shared->netw_master_q[button] = master_msg.content[button + 1];
				}
			}
		}
	}
}
