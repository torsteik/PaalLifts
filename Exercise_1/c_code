#include <stdio.h>
#include <pthread.h>

int i = 0;

void* threadIterateUp(){
	for(int j = 0; j < 1000000; j++){
		i++;
	}
}
void* threadIterateDown(){
	for(int j = 0; j < 1000000; j++){
		i--;
	}
}

int main(void) {
	pthread_t thread1;
	pthread_t thread2;
	
	pthread_create(&thread1, NULL, threadIterateUp, NULL);
	pthread_create(&thread2, NULL, threadIterateDown, NULL);
	
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	
	printf("Value of i: %i\n", i);
	return 0;
}
