#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>

#include "defines.h"
#include "SharedVars.h"
#include "elev.h"
#include "netw_fsm.h"
#include "elev_fsm.h"
#include "main.h"

using namespace std;

int main(){
	elev_set_motor_direction(DIRN_STOP);
	elev_init();
	sleep(1);
	recover_internal_queue();
	pthread_t main_thread_0;
	pthread_t main_thread_1;
	pthread_create(&main_thread_0, NULL, elev_fsm, NULL);
	pthread_create(&main_thread_1, NULL, netw_fsm, NULL);
			
	pthread_join(main_thread_0, NULL);
	pthread_join(main_thread_1, NULL);

	return 0;
}

// Unused functions for local backup of variables necessary to resume operation

int char_to_int(char val){
	return (val - 48);
}
char int_to_char(int val){
	return (val + 48);
}

void recover_internal_queue(){

	ifstream recovery_file("internal_recovery.txt", std::fstream::in);

	if (recovery_file){
		for (int i = 0; i < N_FLOORS; ++i){
			if(char_to_int(recovery_file.get())){
				shared.elev_q.add_new_order(i, 2);
			}
		}std::cout << std::endl;
	}
	else{
		cout << "Unable to read from internal queue's recovery file" << endl;
	}
	recovery_file.close();
}

void write_internal_queue(){

	ofstream recovery_file("internal_recovery.txt", std::fstream::out | std::fstream::trunc);

	if (!recovery_file.fail()){
		for (int i = 0; i < N_FLOORS; ++i){
			printf("On position %i, we find: %i \n", i, (int)shared.elev_q.int_q[i]);
			recovery_file << (int)shared.elev_q.int_q[i];
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
		
		shared.elev_fsm_state = process_recovery_file.get();
		shared.netw_role = process_recovery_file.get();
		
		shared.floor = process_recovery_file.get();
		shared.dir = process_recovery_file.get();

		//----------------Network states---------------------------

		for (int i = 0; i < 255; ++i){
			shared.netw_membs[i].netw_role = process_recovery_file.get();
		}
		shared.netw_fsm_state = process_recovery_file.get();
		for (int i = 0; i < 2 * N_FLOORS; ++i){
			shared.netw_master_q[i] = process_recovery_file.get();
		}
		for (int i = 0; i < 2 * N_FLOORS; ++i){
			shared.elev_q.ext_q[i] = process_recovery_file.get();
		}
		shared.recovered  = 1;
	}
	else{
		cout << "Unable to read from internal queue's recovery file" << endl;
		shared.recovered = 0;
	}
	process_recovery_file.close();

}

void write_process_pair(){

	ofstream process_recovery_file("process_state.txt", std::fstream::out | std::fstream::trunc);

	if (!process_recovery_file.fail()){
		
		process_recovery_file << shared.elev_fsm_state;
		process_recovery_file << shared.netw_role;

		process_recovery_file << shared.floor;
		process_recovery_file << shared.dir;

		//----------------Network states---------------------------

		for (int i = 0; i < 255; ++i){
			process_recovery_file << shared.netw_membs[i].netw_role;
		}
		process_recovery_file << shared.netw_fsm_state;
		for (int i = 0; i < 2 * N_FLOORS; ++i){
			process_recovery_file << shared.netw_master_q[i];
		}
		for (int i = 0; i < 2 * N_FLOORS; ++i){
			process_recovery_file << shared.elev_q.ext_q[i];
		}
	}

	else{
		cout << "Unable to write to the internal queue's recovery file" << endl;
	}
	process_recovery_file.close();
}
