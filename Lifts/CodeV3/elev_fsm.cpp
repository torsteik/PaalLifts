#include <time.h>

#include "elev_driver.h"
#include "elev_fsm.h"
#include "elev.h"
#include "Queue.h"


//stateArray[4] = { initialize, emergencyStop, stopElevator, isQueueEmpty }


//---------------State elements---------------
#define INITIALIZE 		0
#define EMERGENCY_STOP 	1
#define STOP_ELEVATOR 	2
#define IS_QUEUE_EMPTY 	3
//--------------------------------------------

void stateMachine(int stateArray[], state_t state, SharedVars* shared){
	static TEMP_STOP_substate_t substate;
	static int previousFloor;
	static int previousDirection; //1 tilsvarer ned og 0 opp.
	static time_t timerStart;
	time_t currentTime;

	while(1){ //OBS IKKE WHILE 1 OM DU BRUKER MAIN
		switch (state){
		case INIT:
			previousDirection = 1;
			shared->dir = 1; //Slå sammen dir og prev_dir
			substate = WAIT_FOR_DOOR; // Move
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
			// Move break out and set state =  IDLE

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
				shared->elev_q->register_completed_order(elev_get_floor_sensor_signal());
				substate = WAIT_FOR_DOOR;
				break;
			case WAIT_FOR_DOOR:
				for (int button = 0; button < 3; ++button){
					if (shared->elev_q->getQ(previousFloor, button)){
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
				shared->elev_q->register_completed_order(floor);
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

//Rename to 'try_find_destination'?
void findDestination(int stateArray[], int* prevDirection, int* prevFloor, SharedVars* shared){
	if (shared->elev_q->is_queue_empty()){
		stateArray[3] = 1;
		// state = IDLE
		return;
	}
	else{
		stateArray[3] = 0;
		find_destination_(stateArray,prevDirection, prevFloor);	//Remove underscore
	}
}

void find_destination_(int stateArray[], int* previous_direction, int* prevFloor, SharedVars* shared){
	int logic_1, logic_2;
	if (*previous_direction){
		logic_1 = 1;
		logic_2 = 0;
	}
	else{
		logic_1 = -1;
		logic_2 = 1;
	}
	for (int floor = *prevFloor; logic_1*floor >= logic_1*3*logic_2; floor -= logic_1){
		for (int button = 0; button < 3; ++button){
			if (shared->elev_q->getQ(floor, button)){
				if (floor != *prevFloor){
					stateArray[2] = 0;
					// state = MOVE
					elev_set_motor_direction((tag_elev_motor_direction)logic_1); // Could logic one be switched with prev_dir?
					return;
				}
				else{
					if (*prevFloor == elev_get_floor_sensor_signal(){
						//state = TEMP_STOP
						return;
					}
					///////Dette gjeleder ved noen nødstopp situasjoner///////
					else{
						// I think this is: elev had stopped between 2 floors and is ordered to continue in its prev dir
						stateArray[2] = 0;
						// state = MOVE
						elev_set_motor_direction((tag_elev_motor_direction)logic_1);
						*previous_direction = logic_2;
						shared->dir = logic_2;
						return;
					}
					//////////////////////////////////////////////////////////
				}
			}
		}
	}
	//Destination is the other way
	stateArray[2] = 0;
	//state = MOVE
	elev_set_motor_direction(logic_1); //DIRN UP or opposite dir or something
	*prevDirection = logic_2;
	return;
}

void evaluateFloor(int stateArray[], int* prevDirection, int* prevFloor, SharedVars* shared){
	int currentFloor = elev_get_floor_sensor_signal();
	if (currentFloor > -1){
		*prevFloor = currentFloor;
		shared->floor = currentFloor; // Slå sammen floor og prevFloor??
		elev_set_floor_indicator(currentFloor);
		check_buttons_acc_to_dir(currentFloor, prevDirection)
	}
}

void check_buttons_acc_to_dir(int currentFloor, int* prevDirection, SharedVars* shared){
	int logic_1, logic_2, logic_3;
	if(*prevDirection){
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
		if (shared->elev_q->getQ(currentFloor, button)){
			elev_set_motor_direction(DIRN_STOP);
			stateArray[2] = 1;
			//state = TEMP_STOP;
			return;
		}
	}
	//Sjekker bestillinger knyttet til etasjene i denne retningen.
	for (int floor = currentFloor + logic_2; -logic_2*floor >= -logic_2*logic_3; floor += logic_2){
		for (int button = 0; button < 3; ++button){
			if (shared->elev_q->getQ(floor, button)){
				return;
			}
		}
	}
	//Eneste gjenstående mulighet er bestillinger
	//knyttet til motsatt retning i nåværende etasje.
	elev_set_motor_direction(DIRN_STOP);
	stateArray[2] = 1;
	//state = TEMP_STOP and substate = DETERMINE_NEXT_ACTION
}

