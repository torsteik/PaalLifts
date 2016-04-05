#include defines.h

//Set initial state
state cur_state = FIND_NETWORK;
substate_m cur_substate_m = SETUP;
//Backup starts as an invalid ID
uint8_t backup = NO_IP;
//List of what IP's are included in our network
uint8_t ID_list[256];


while (1){
	switch (cur_state){
	case FIND_NETWORK:
		if (!find_master()){ cur_state = MASTER; }
		else			   { cur_state = SLAVE; }
		break

	case MASTER:
		switch (cur_substate_m){
		case SETUP: //Start threads
			start(recieve_connection);
			start(broadcast_IAMALIVE)
			...
			/*
			PLACE THIS SOMEWHERE
			if (backup == NO_IP){
				find_backup();
				start(backup_com);
			} 
			//When a dead backup is found backup will be set to NO_IP and backup thread will be closed
			*/
			break

		case RUN: //Check shared variables to see if we want to change state
			if (network_error){
				cur_substate_m = CLOSE_NETWORK;
				break
			}
			...
			break

		case CLOSE_NETWORK:	
			broadcast_death(); //Signal shutdown of network to participant
			/*
			Close threads and sockets
			*/
			cur_substate_m = SETUP;
			cur_state_m = FIND_NETWORK;
			break
		}

	case SLAVE:
		switch (cur_substate_s){
		case SETUP:
			
			break
		
		case BACKUP: //Only for setting up threads, need seperate way to indicate that this is a backup
	
			break
		}
		break
	}
}


