#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include<fstream>

using namespace std;

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

void recover_queue(){
	ifstream myReadFile;
	myReadFile.open("text.txt");
	string
}

int init(){
	elevator_io_init();
	recover_queue();
}