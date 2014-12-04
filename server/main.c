#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include "globals.h"

int main(){
	srandom(time(NULL));
	struct timespec t = {.tv_sec=0};
	struct timespec lastTime = {.tv_sec = 0, .tv_nsec = 0};
	struct timespec otherTime = {.tv_sec = 0, .tv_nsec = 0};

	sector *conductor, *tmp, **runner;
	listrootsector = NULL;
	initModules();
	initAis();
	//start network listening thread
	pthread_t id;
	pthread_create(&id, NULL, netListen, NULL);
	newEntity(2, 2, 1, searchforsector(0, 0), 5000, 5000);
//	mkship("yo");
//	loadship("yo");
//	move(0, 0, 1, 0);
	short adventuretime = 300;//nope, make that "char timetillpotentialtospawnastroidssothatwedontcheckeachtickandslowshitdown"
	while(1){
		conductor = listrootsector;
		if(adventuretime-- == 0) adventuretime = 300;
		while(conductor != NULL){
			if(adventuretime == 300 && conductor->realnumber == 0){
				spawnstroids(conductor);
			}
			run(conductor);
			conductor = conductor->nextsector;
		}
		conductor = listrootsector;
		while(conductor != NULL){
			run2(conductor);
			conductor = conductor->nextsector;
		}
		cleanup();
		runner = &listrootsector;
		while(*runner != NULL){
			if((*runner)->number == 0){
				tmp = *runner;
				*runner = (*runner)->nextsector;
				unloadsector(tmp);
			}else runner = &(*runner)->nextsector;
		}
		sendInfo();
		globalActedFlag ^= 1;
		clock_gettime(CLOCK_MONOTONIC, &otherTime);
		int32_t sleep = (int32_t)25000000 - (otherTime.tv_nsec-lastTime.tv_nsec) - 1000000000l*(otherTime.tv_sec-lastTime.tv_sec);
		if(sleep > 0){
			t.tv_nsec = sleep;
			nanosleep(&t, NULL);
		}
		clock_gettime(CLOCK_MONOTONIC, &lastTime);
	}
	return(0);
}
