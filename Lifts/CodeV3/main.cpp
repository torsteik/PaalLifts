#include <pthread.h>

#include "defines.h"
#include "netw_fsm.h"
#include "elev_driver.h"


int main(){
	pthread_t main_threads[2];
	struct SharedVars shared;

	pthread_create(&main_threads[0], NULL, &elev_fsm, NULL);
	pthread_create(&main_threads[1], NULL, &netw_fsm, NULL);

	for(int i = 0; i < 2; ++i){
		pthread_join(main_threads[i]);
	}
}

