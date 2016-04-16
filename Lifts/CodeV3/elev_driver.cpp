#include "elev_driver.h"
#include "elev_fsm.h"
#include "Queue.h"
#include "elev.h"

void* elev_driver(void* shared_void){
	elev_set_motor_direction(DIRN_STOP);
	elev_init();

	int stateArray[4] = { 1, 0, 1, 1 };		
	
	state_t state;
	while (1){
		state = determineState(stateArray);
		stateMachine(stateArray, state);
		handleButtonPress(stateArray);
	}
}


state_t determineState(int stateArray[]){
	if 		( stateArray[0])									{ return INIT; }
	else if (!stateArray[1] && stateArray[2] && stateArray[3])	{ return IDLE; }
	else if (!stateArray[1] && !stateArray[2])					{ return MOVE; }
	else if (!stateArray[1] && stateArray[2] && !stateArray[3])	{ return TEMP_STOP; }
	else if ( stateArray[1])									{ return EMERGENCY_STOP; }
	else														{ return DEFAULT; }
}

void handleButtonPress(int stateArray[], SharedVars* shared){
	static int previouslyPressedButtons[4][3] = {
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 }
	};

	if (elev_get_stop_signal()){
		stateArray[1] = 1;
		//state = EMERGENCY_STOP
		elev_set_stop_lamp(1);
		return;
	}
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

