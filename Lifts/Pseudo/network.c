#include defines.c

state cur_state = FIND_NETWORK;
uint8_t backup = NO_IP;

uint8_t ID_list[256];


while(1){
	switch (state){
		case FIND_NETWORK:
			if(!find_master()){ state = MASTER; }
			else			  {	state = request_state(); }
			break

		case MASTER:
			switch(substate_m){
				case SETUP:
					start(recieve_connection);
					...
					break
					
				case RUN:
					broadcast_IMALIVE();
					update_backup();
					...
					break
			}
			break

		case BACKUP:
			break

		case SLAVE:
			break
	}
}
