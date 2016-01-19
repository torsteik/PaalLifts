#include <stdio.h>
#include <pthread.h>

typedef struct{
	int value;
	pthread_mutex_t fast_mutex;
} shared_int_t;

shared_int_t i = {
	.value = 0,
	.fast_mutex = PTHREAD_MUTEX_INITIALIZER
};

void* threadIterateUp(){
	for(int j = 0; j < 1000000; j++){ 
		pthread_mutex_lock(&i.fast_mutex);
		i.value++; 
		pthread_mutex_unlock(&i.fast_mutex);	
	}
	
}
void* threadIterateDown(){
	for(int j = 0; j < 1000000; j++){
		pthread_mutex_lock(&i.fast_mutex);
		i.value--; 
		pthread_mutex_unlock(&i.fast_mutex);
	}
}

int main(void) {
	pthread_t thread1;
	pthread_t thread2;
	
	pthread_create(&thread1, NULL, threadIterateUp,   NULL);
	pthread_create(&thread2, NULL, threadIterateDown, NULL);
	
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	
	printf("Value of i: %i\n", i.value);
	return 0;
}
