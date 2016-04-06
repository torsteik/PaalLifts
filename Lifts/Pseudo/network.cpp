#include defines.h

//Set initial state
state cur_state = FIND_NETWORK;
substate_m cur_substate_m = SETUP;
//Backup starts as an invalid ID
uint8_t backup = NO_IP;
//List of what IP's are included in our network
uint8_t ID_list[256];


while (1){ //OBS REMOVE WHILE(1) or MAKE SURE IT WONT SPAWN INF THREADS
	switch (cur_state){
	case FIND_NETWORK:
		if (!find_master()){ cur_state = MASTER; }
		else			   { cur_state = SLAVE; }
		
		break

	case MASTER:
		start(manage_connections);
		start(broadcast_IAMALIVE);
		start(backup_manager);
		start(logic);

		while(!network_error);

		broadcast_death();
		close(sockets);
		close(threads);
		cur_state = FIND_NETWORK;

		break

	case SLAVE:
		//Probably dont need threads here
		start(serve); //Includes functionality for storing backup info
		start(backup_com);

		wait_for(network_error);

		close(sockets); //Might need one to wait for backup
		close(threads);

		if(i_am_backup){
			//Become master and setup the new network
			broadcast_promotion()
			//close things if needed
			cur_state = MASTER;
		}
		else{
			//Wait for new master
			receive_from_broadcast() //Waits for IP from backup
			//close things if needed
		}
		break
	}
}
