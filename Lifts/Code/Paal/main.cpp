#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;
#define N_FLOORS 4
#define NUM_FSM_STATES 4

typedef enum tag_TEMP_STOP_substate{
	ARRIVAL = 0,
	WAIT_FOR_DOOR = 1,
	DETERMINE_NEXT_ACTION = 2
} TEMP_STOP_substate_t;

int internal_queue[] = { 0, 0, 0, 0 };
int FSM_state[] = { 1, 0, 1, 1 };
int previous_floor = 2;
int previous_dir = 1;
TEMP_STOP_substate_t substate = ARRIVAL;

//void elevator_io_init(){
//	if (!elev_init()){
//		printf("Initialization failed!\n");
//		exit(0);
//	}
//
//	//Find defined state
//	elev_set_motor_direction(DIRN_UP);
//	while (elev_get_floor_sensor_signal() == -1){ usleep(200) }
//
//	currentFloor = elev_get_floor_sensor_signal() + 1;
//	elev_set_floor_indicator(currentFloor - 1);
//}

//-----------------------------------------------------------------------------------------------------------------

int char_to_int(char c){
	return (int)c - 48;
}

void recover_internal_queue(){

	ifstream recovery_file("internal_recovery.txt", std::fstream::in);

	if (recovery_file){
		for (int i = 0; i < N_FLOORS; ++i){
			internal_queue[i] = recovery_file.get();
		}
	}
	else{
		cout << "Unable to read from internal queue's recovery file" << endl;
	}
	recovery_file.close();
}

void write_recovery_file(){

	ofstream recovery_file("internal_recovery.txt", std::fstream::out | std::fstream::trunc);

	if (!recovery_file.fail()){
		for (int i = 0; i < N_FLOORS; ++i){
			recovery_file << internal_queue[i];
		}
	}

	else{
		cout << "Unable to write to the internal queue's recovery file" << endl;
	}
	recovery_file.close();
}

//-----------------------------------------------------------------------------------------------------------------

void recover_process_pair(){

	ifstream process_recovery_file("process_state.txt", std::fstream::in);

	if (process_recovery_file){
		for (int i = 0; i < NUM_FSM_STATES; ++i){
			FSM_state[i] = process_recovery_file.get();
		}

		previous_floor = process_recovery_file.get();
		previous_dir = process_recovery_file.get();
		(TEMP_STOP_substate_t)substate = process_recovery_file.get();
	}
	else{
		cout << "Unable to read from internal queue's recovery file" << endl;
	}
	process_recovery_file.close();
}

void write_process_pair(){

	ofstream process_recovery_file("process_state.txt", std::fstream::out | std::fstream::trunc);

	if (!process_recovery_file.fail()){
		for (int i = 0; i < NUM_FSM_STATES; ++i){
			process_recovery_file << FSM_state[i];
		}

		process_recovery_file << previous_floor;
		process_recovery_file << previous_dir;
		process_recovery_file << (int)(substate);

		//Network states
	}

	else{
		cout << "Unable to write to the internal queue's recovery file" << endl;
	}
	process_recovery_file.close();
}

//-----------------------------------------------------------------------------------------------------------------
int main(){
	//elevator_io_init();
	recover_internal_queue();
	return 0;
}