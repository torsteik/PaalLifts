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

void elevator_io_init(){
	if (!elev_init()){
		printf("Initialization failed!\n");
		exit(0);
	}

	//Find defined state
	elev_set_motor_direction(DIRN_UP);
	while (elev_get_floor_sensor_signal() == -1){ usleep(200) }

	currentFloor = elev_get_floor_sensor_signal() + 1;
	elev_set_floor_indicator(currentFloor - 1);
}

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

int init(){
	elevator_io_init();
	recover_internal_queue();
}