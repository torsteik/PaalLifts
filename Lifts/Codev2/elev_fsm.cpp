#include "main.h"
#include "StateMachine.h"
#include "elev.h"
#include "Queue.h"
#include <time.h>

//stateArray[4] = { initialize, emergencyStop, stopElevator, isQueueEmpty }


//---------------State elements---------------
#define INITIALIZE 0
#define EMERGENCY_STOP 1
#define STOP_ELEVATOR 2
#define IS_QUEUE_EMPTY 3
//--------------------------------------------

void stateMachine(int stateArray[], state_t state, SharedVars* shared){
	static TEMP_STOP_substate_t substate;
	static int previousFloor;
	static int previousDirection; //1 tilsvarer ned og 0 opp.
	static time_t timerStart;
	time_t currentTime;

	switch (state){
	case INIT:
		previousDirection = 1;
		substate = WAIT_FOR_DOOR;
		if (findPosition(&previousFloor)){
			stateArray[0] = 0;
			break;
		}
		else{
			elev_set_motor_direction(DIRN_DOWN);
			while (!findPosition(&previousFloor)){}
			elev_set_motor_direction(DIRN_STOP);
			stateArray[0] = 0;
			break;
		}
	case IDLE:
		findDestination(stateArray, &previousDirection, &previousFloor);
		break;
	case MOVE:
		evaluateFloor(stateArray, &previousDirection, &previousFloor);
		break;
	case TEMP_STOP:
		switch (substate){
		case ARRIVAL:
			elev_set_door_open_lamp(1);
			timerStart = clock();
			updateQueue(elev_get_floor_sensor_signal());
			substate = WAIT_FOR_DOOR;
			break;
		case WAIT_FOR_DOOR:
			for (int button = 0; button < 3; ++button){
				if (getQueue(previousFloor, button)){
					substate = ARRIVAL;
					break;
				}
			}
			currentTime = (clock() - timerStart) / CLOCKS_PER_SEC;
			if (currentTime >= 3){
				elev_set_door_open_lamp(0);
				substate = DETERMINE_NEXT_ACTION;
			}
			break;

		case DETERMINE_NEXT_ACTION:
			findDestination(stateArray, &previousDirection, &previousFloor);
			substate = ARRIVAL;
			break;
		default:
			break;
		}
		break;
	case EMERGENCY_STOP:
		elev_set_motor_direction(DIRN_STOP);
		for (int floor = 0; floor < 4; ++floor){
			updateQueue(floor);
		}
		while (elev_get_stop_signal()){}
		elev_set_stop_lamp(0);
		stateArray[1] = 0;
		stateArray[2] = 1;
		if (elev_get_floor_sensor_signal() > -1){
			stateArray[3] = 0;
		}
		else{
			stateArray[3] = 1;
		}
		break;
	default:
		break;
	}
}

int findPosition(int* prevFloor){
	int currentLocation = elev_get_floor_sensor_signal();
	if (currentLocation > -1){
		*prevFloor = currentLocation;
		elev_set_floor_indicator(currentLocation);
		return 1;
	}
	else{
		return 0;
	}
}

int isQueueEmpty(){
	int button;
	for (int floor = 0; floor < 4; ++floor){
		for (int button_type = 0; button_type < 3; ++button_type){
			button = getQueue(floor, button_type);
			if (button){
				return 0;
			}
		}
	}
	return 1;
}

//Rename to 'try_find_destination'?
void findDestination(int stateArray[], int* prevDirection, int* prevFloor){
	if (isQueueEmpty()){
		stateArray[3] = 1;
		return;
	}
	else{
		stateArray[3] = 0;
		find_destination_(stateArray,prevDirection, prevFloor);	//Remove underscore
	}
}

void find_destination_(int stateArray[], int* previous_direction, int* prevFloor){
	int logic_1, logic_2;
	if (*previous_direction){
		logic_1 = 1;
		logic_2 = 0;
	}
	else{
		logic_1 = -1;
		logic_2 = 1;
	}
	int currentFloor = elev_get_floor_sensor_signal();
	for (int floor = *prevFloor; logic_1*floor >= logic_1*3*logic_2; floor -= logic_1){
		for (int button = 0; button < 3; ++button){
			if (getQueue(floor, button)){
				if (floor != *prevFloor){
					stateArray[2] = 0;
					elev_set_motor_direction((tag_elev_motor_direction)logic_1);
					return;
				}
				else{
					if (*prevFloor == currentFloor){
						return;
					}
					///////Dette gjeleder ved noen nødstopp situasjoner///////
					else{
						stateArray[2] = 0;
						elev_set_motor_direction((tag_elev_motor_direction)logic_1);
						*previous_direction = logic_2;
						return;
					}
					//////////////////////////////////////////////////////////
				}
			}
		}
	}
	stateArray[2] = 0;
	elev_set_motor_direction(DIRN_UP);
	*prevDirection = 0;
	return;
}

void evaluateFloor(int stateArray[], int* prevDirection, int* prevFloor){
	int currentFloor = elev_get_floor_sensor_signal();
	if (currentFloor > -1){
		*prevFloor = currentFloor;
		elev_set_floor_indicator(currentFloor);
		check_buttons_acc_to_dir(currentFloor, prevDirection)
	}
}

void check_buttons_acc_to_dir(int currentFloor, int* prevDirection){
	int logic_1, logic_2, logic_3;
	if(*prevDirection){
		logic_1 = 1;
		logic_2 = -1;
		logic_3 = 0;
	}
	else{
		logic_1 = 2;
		logic_2 = 1;
		logic_3 = 3;
	}
	//Sjekker bestillinger knyttet til 
	//BUTTON_DOWN og BUTTON_COMMAND i denne etasjen.
	for (int button = 1; button < 3; button += logic_1){
		if (getQueue(currentFloor, button)){
			elev_set_motor_direction(DIRN_STOP);
			stateArray[2] = 1;
			return;
		}
	}
	//Sjekker bestillinger knyttet til etasjene under.
	for (int floor = currentFloor + logic_2; -logic_2*floor >= -logic_2*logic_3; floor += logic_2){
		for (int button = 0; button < 3; ++button){
			if (getQueue(floor, button)){
				return;
			}
		}
	}
	//Eneste gjenstående mulighet er bestillinger
	//knyttet til motsatt retning i nåværende etasje.
	elev_set_motor_direction(DIRN_STOP);
	stateArray[2] = 1;
}

void updateQueue(int floor){
	for (int button = 0; button < 3; ++button){
		setQueue(floor, button, 0); //Update completed orders 
		
	}
	if (floor != 3){
		elev_set_button_lamp(BUTTON_CALL_UP, floor, 0);
	}
	if (floor != 0){
		elev_set_button_lamp(BUTTON_CALL_DOWN, floor, 0);
	}
	elev_set_button_lamp(BUTTON_COMMAND, floor, 0);

}