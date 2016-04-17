#include <string.h>

#include "defines.h"
#include "Queue.h"
#include "elev.h"

Queue::Queue(){
	//memset(this->int_q, 0, sizeof(this->int_q));
	//memset(this->ext_q, 0, sizeof(this->ext_q));
	//memset(this->orders_new, 0, sizeof(this->orders_new));
	//memset(this->orders_complete, 0, sizeof(this->orders_complete));
}

int Queue::get_q(int floor, int button){
	if		(button == 0){ return (int)this->ext_q[floor]; }
	else if (button == 1){ return (int)this->ext_q[N_FLOORS + floor]; }
	else 				 { return this->int_q[floor]; }
}

void Queue::add_new_order(int floor, int button){
	if (button == 0){ 
		this->orders_new[0] = NEW_ORDER;
		this->orders_new[1 + floor] = 1; 
	}
	else if (button == 1){ 
		this->orders_new[0] = NEW_ORDER;
		this->orders_new[1 + N_FLOORS + floor] = 1; 
	}
	else{ 
		this->int_q[floor] = 1;
		elev_set_button_lamp(BUTTON_COMMAND, floor, 1);
	}
}

void Queue::add_order_from_netw(int button){
	this->ext_q[button] = 1;
	if(button < N_FLOORS){ elev_set_button_lamp(BUTTON_CALL_UP, button, 1); }
	else 				 { elev_set_button_lamp(BUTTON_CALL_DOWN, button - N_FLOORS, 1); }
}

void Queue::recycle_order(int button){
	int floor;
	int button_type;
	if(button < N_FLOORS){
		floor = button;
		button_type = 0;
	}
	else{
		floor = button - N_FLOORS;
		button_type = 1;
	}
	this->add_new_order(floor, button_type);
}

void Queue::register_completed_order(int floor){
	this->int_q[floor] = 0;
	this->ext_q[floor] = 0;
	this->ext_q[N_FLOORS + floor] = 0;

	if (floor != (N_FLOORS-1)){	elev_set_button_lamp(BUTTON_CALL_UP, floor, 0); }
	if (floor != 0)			  { elev_set_button_lamp(BUTTON_CALL_DOWN, floor, 0); }
	elev_set_button_lamp(BUTTON_COMMAND, floor, 0);

	this->orders_complete[0] = COMPLETED_ORDER;
	this->orders_complete[1 + floor] = 1;
	this->orders_complete[1 + N_FLOORS + floor] = 1;
}

void Queue::sync_q_with_netw(std::atomic<char> netw_master_q[N_FLOORS * 2]){
	for(int button = 0; button < N_FLOORS * 2; ++button){
		if(!netw_master_q[1 + button]){
			ext_q[button] = 0;
		}	// I think this below is right, but not sure if we want to turn off lights as well
		if(button < N_FLOORS){ elev_set_button_lamp(BUTTON_CALL_UP, button, (bool)netw_master_q[1 + button]); }
		else 				 { elev_set_button_lamp(BUTTON_CALL_DOWN, button - N_FLOORS, (bool)netw_master_q[1 + button]; }
	}
}

int Queue::is_queue_empty(){
	for (int floor = 0; floor < N_FLOORS; ++floor){
		for (int button= 0; button < 3; ++button){
			if (this->get_q(floor, button)){
				return 0;
			}
		}
	}
	return 1;
}