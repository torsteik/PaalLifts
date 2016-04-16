#include "Queue.h"

Queue::Queue(){
	memset(this->int_q, 0, sizeof(this->int_q));
	memset(this->ext_q, 0, sizeof(this->ext_q));
	memset(this->orders_new, 0, sizeof(this->orders_new));
	memset(this->orders_complete, 0, sizeof(this->orders_complete));
}

int Queue::getQ(int floor, int button){
	if		(button == 0){ return (int)this->ext_q[floor]; }
	else if (button == 1){ return (int)this->ext_q[N_FLOORS + 1 + floor]; }
	else 				 { return this->int_q[floor]; }
}

void Queue::add_new_order(int floor, int button){
	if (button == 0){ 
		this->new_orders[0] = NEW_ORDERS;
		this->new_orders[1 + floor] = 1; 
		//elev_set_button_lamp(BUTTON_CALL_UP, floor, 1);
	}
	else if (button == 1){ 
		this->new_orders[0] = NEW_ORDERS;
		this->new_orders[1 + N_FLOORS + floor] = 1; 
		//elev_set_button_lamp(BUTTON_CALL_DOWN, floor, 1);
	}
	else{ 
		this->int_q[floor] = 1;
		elev_set_button_lamp(BUTTON_COMMAND, floor, 1);
	}
}

void Queue::add_order_from_netw(int button){
	this->ext_q[button] = 1;
	if(button < N_FLOORS){
		elev_set_button_lamp(BUTTON_CALL_UP, floor, 1);
	}
	else{
		elev_set_button_lamp(BUTTON_CALL_DOWN, floor, 1);
	}
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
	this->ext_q[floor] = 0;
	this->ext_q[N_FLOORS + floor] = 0;
	this->int_q[floor] = 0;

	if (floor != (N_FLOORS-1)){	elev_set_button_lamp(BUTTON_CALL_UP, floor, 0); }
	if (floor != 0)			  { elev_set_button_lamp(BUTTON_CALL_DOWN, floor, 0); }
	elev_set_button_lamp(BUTTON_COMMAND, floor, 0);

	this->orders_complete[0] = COMPLETED_ORDER;
	this->orders_complete[1 + floor] = 1;
	this->orders_complete[1 + N_FLOORS + floor] = 1;
}

void Queue::sync_q_with_netw(char netw_master_q){
	for(int button = 0; button < N_FLOORS * 2; ++button){
		if(!netw_master_q[1 + button]){
			ext_q[button] = 0;
		}
		else{
			if(button < N_FLOORS){
				elev_set_button_lamp(BUTTON_CALL_UP, floor, 1);
			}
			else{
				elev_set_button_lamp(BUTTON_CALL_DOWN, floor, 1);
			}
		}
	}
}

int Queue::is_queue_empty(){
	for (int floor = 0; floor < 4; ++floor){
		for (int button= 0; button < 3; ++button){
			if (this->getQ(floor, button)){
				return 0;
			}
		}
	}
	return 1;
}
