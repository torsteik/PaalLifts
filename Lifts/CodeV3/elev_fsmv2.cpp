#include <time.h>

#include "elev_driver.h"
#include "elev_fsm.h"
#include "elev.h"
#include "Queue.h"

int temp_stop_substate; // Move

void* stateMachine(){
	time_t timerStart;

	temp_stop_substate = WAIT_FOR_DOOR;
	while(1){ //OBS IKKE WHILE 1 OM DU BRUKER MAIN
		
		switch (shared->elev_fsm_state){
		case INIT:
			shared->dir = 1;
			if (!findPosition()){
				elev_set_motor_direction(DIRN_DOWN);
				while (!findPosition()){}
				elev_set_motor_direction(DIRN_STOP);
			}
			shared->elev_fsm_state = IDLE;
			break;

		case IDLE:
			findDestination();
			break;

		case MOVE:
			evaluateFloor();
			break;

		case TEMP_STOP:
			switch (temp_stop_substate){
			case ARRIVAL:
				elev_set_door_open_lamp(1);
				timerStart = clock();
				shared->elev_q->register_completed_order(shared->floor);
				temp_stop_substate = WAIT_FOR_DOOR;
				break;
			case WAIT_FOR_DOOR:
				for (int button = 0; button < 3; ++button){
					if (shared->elev_q->getQ(shared->floor, button)){
						temp_stop_substate = ARRIVAL;
						break;
					}
				}
				if ((clock() - timerStart) / CLOCKS_PER_SEC; >= 3){
					elev_set_door_open_lamp(0);
					temp_stop_substate = DETERMINE_NEXT_ACTION;
				}
				break;

			case DETERMINE_NEXT_ACTION:
				findDestination();
				temp_stop_substate = ARRIVAL;
				break;

			default:
				break;
			}
			break;
		}
	}
}

int findPosition(){
	int currentLocation = elev_get_floor_sensor_signal();
	if (currentLocation > -1){
		shared->floor = currentLocation;
		elev_set_floor_indicator(currentLocation);
		return 1;
	}
	else{
		return 0;
	}
}

//Rename to 'try_find_destination'?
void findDestination(){
	if (shared->elev_q->is_queue_empty()){
		elev_fsm_state = IDLE;
		return;
	}
	else{

		int logic_1, logic_2;
		if (shared->dir){
			logic_1 = 1;
			logic_2 = 0;
		}
		else{
			logic_1 = -1;
			logic_2 = 1;
		}
		for (int floor_itr = shared->dir; logic_1*floor_itr >= logic_1*(N_FLOORS-1)*logic_2; floor_itr -= logic_1){
			for (int button = 0; button < 3; ++button){
				if (shared->elev_q->getQ(floor, button)){
					if (floor_itr != shared->floor){
						elev_set_motor_direction((elev_motor_direction_t)logic_1); // Could logic one be switched with prev_dir?
						shared->elev_fsm_state = MOVE;
					}
					else{
						shared->elev_fsm_state = TEMP_STOP;
					}
					return;
				}
			}
		}
		//Destination is the other way
		shared->elev_fsm_state = MOVE;
		elev_set_motor_direction(logic_1); //DIRN UP or opposite dir or something
		shared->dir = logic_2;
		return;
	}
}

void evaluateFloor(){
	int current_floor = elev_get_floor_sensor_signal();
	if (current_floor > -1){
		shared->floor = current_floor;
		elev_set_floor_indicator(shared->floor);
		
		int logic_1, logic_2, logic_3;
		if(shared->dir){
			logic_1 = 1;
			logic_2 = -1;
			logic_3 = 0;
		}
		else{
			logic_1 = 2;
			logic_2 = 1;
			logic_3 = 3;//N_FLOORS
		}
		//Sjekker bestillinger knyttet til 
		//BUTTON_DOWN og BUTTON_COMMAND(nåværende retning) i denne etasjen.
		for (int button = 1; button < 3; button += logic_1){
			if (shared->elev_q->getQ(shared->floor, button)){
				elev_set_motor_direction(DIRN_STOP);
				shared->elev_fsm_state = TEMP_STOP;
				return;
			}
		}
		//Sjekker bestillinger knyttet til etasjene i denne retningen.
		for (int floor_itr = shared->floor + logic_2; -logic_2*floor_itr >= -logic_2*logic_3; floor_itr += logic_2){
			for (int button = 0; button < 3; ++button){
				if (shared->elev_q->getQ(shared->floor, button)){
					return;
				}
			}
		}
		//Eneste gjenstående mulighet er bestillinger
		//knyttet til motsatt retning i nåværende etasje.
		elev_set_motor_direction(DIRN_STOP);
		shared->elev_fsm_state = TEMP_STOP;
		temp_stop_substate = DETERMINE_NEXT_ACTION;
		return;
	}
}

void handleButtonPress(){
	static int previouslyPressedButtons[4][3] = {
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 }
	};

	for (int floor = 0; floor < 4; ++floor){
		if(floor != N_FLOORS -1 ){
			if (elev_get_button_signal(BUTTON_CALL_UP, floor) && !prevPresses[floor][0]){
				shared->elev_q->add_new_order(floor, 0);
			}
			prevPresses[floor][0] = elev_get_button_signal(BUTTON_CALL_UP, floor);
		}
		if(floor != 0){
			if (elev_get_button_signal(BUTTON_CALL_DOWN, floor) && !prevPresses[floor][1]){
				shared->elev_q->add_new_order(floor, 1);
			}
			prevPresses[floor][1] = elev_get_button_signal(BUTTON_CALL_DOWN, floor);
		}
		if (elev_get_button_signal(BUTTON_COMMAND, floor) && !prevPresses[floor][2]){
			shared->elev_q->add_new_order(floor, 2);
		}
		prevPresses[floor][2] = elev_get_button_signal(BUTTON_COMMAND, floor); // Move into if?
	}
	return;
}