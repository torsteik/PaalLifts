#include "main.h"
#include "StateMachine.h"
#include "Queue.h"
#include "elev.h"

int main(){
	elev_set_motor_direction(DIRN_STOP);
	elev_init();

/**
  stateArray[4] = { initialize, emergencyStop, stopElevator, isQueueEmpty}
  Tilstandsvariablene initialiseres slik at tilstandsmaskinen går inn i
  INIT-statet og får en enkel overgang til IDLE-statet.
*/
	int stateArray[4] = { 1, 0, 1, 1 };		
	
	state_t state;
	int previouslyPressedButtons[4][3] = {
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 }
	};
	while (1){
		state = determineState(stateArray);
		stateMachine(stateArray, state);
		handleButtonPress(stateArray,previouslyPressedButtons);
	}
	return 0;
}


state_t determineState(int stateArray[]){
	if (stateArray[0])						{ return INIT; }
	else if (!stateArray[1] && stateArray[2] && stateArray[3])	{ return IDLE; }
	else if (!stateArray[1] && !stateArray[2])			{ return MOVE; }
	else if (!stateArray[1] && stateArray[2] && !stateArray[3])	{ return TEMP_STOP; }
	else if (stateArray[1])						{ return EMERGENCY_STOP; }
	else								{ return DEFAULT; }
}

void handleButtonPress(int stateArray[], int prevPresses[][3]){
	if (elev_get_stop_signal()){
		stateArray[1] = 1;
		elev_set_stop_lamp(1);
		return;
	}
	for (int floor = 0; floor < 4; ++floor){
		if(floor != 3){
			if (elev_get_button_signal(BUTTON_CALL_UP, floor) && !prevPresses[floor][0]){
				elev_set_button_lamp(BUTTON_CALL_UP, floor, 1);
				setQueue(floor, 0, 1);
			}
			prevPresses[floor][0] = elev_get_button_signal(BUTTON_CALL_UP, floor);
		}
		if(floor != 0){
			if (elev_get_button_signal(BUTTON_CALL_DOWN, floor) && !prevPresses[floor][1]){
				elev_set_button_lamp(BUTTON_CALL_DOWN, floor, 1);
				setQueue(floor, 1, 1);
			}
			prevPresses[floor][1] = elev_get_button_signal(BUTTON_CALL_DOWN, floor);
		}
		if (elev_get_button_signal(BUTTON_COMMAND, floor) && !prevPresses[floor][2]){
			elev_set_button_lamp(BUTTON_COMMAND, floor, 1);
			setQueue(floor, 2, 1);
		}
		prevPresses[floor][2] = elev_get_button_signal(BUTTON_COMMAND, floor);
	}
	return;
}

