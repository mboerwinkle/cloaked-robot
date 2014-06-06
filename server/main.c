#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "globals.h"

int main(){
	struct timespec t = {.tv_sec=0};
	struct timespec lastTime = {.tv_sec = 0, .tv_nsec = 0};
	struct timespec otherTime = {.tv_sec = 0, .tv_nsec = 0};

	sector *conductor;
	listrootsector = NULL;
	initModules();
	//start network listening thread
	pthread_t id;
	pthread_create(&id, NULL, netListen, NULL);
//	mkship("yo");
//	loadship("yo");
//	move(0, 0, 1, 0);
	while(1){
		conductor = listrootsector;
		while(conductor != NULL){
			run(conductor);
			conductor = conductor->nextsector;
		}
		clock_gettime(CLOCK_MONOTONIC, &otherTime);
		long int sleep = (long int)25000000 - (otherTime.tv_nsec-lastTime.tv_nsec) - 1000000000l*(otherTime.tv_sec-lastTime.tv_sec);
		if(sleep > 0){
			t.tv_nsec = sleep;
			nanosleep(&t, NULL);
		}
		clock_gettime(CLOCK_MONOTONIC, &lastTime);
	}
	return(0);
}
