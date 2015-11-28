#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#include "globals.h"
#include "networking.h"

loadRequest *lastLoadRequest = NULL;

int main(){
	//srandom(time(NULL));
	initNetworking();
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
	pthread_detach(id);
	//newEntity(NULL, 2, 2, 1, searchforsector(0, 0), 5000, 5000, 0, 0);// WTF???
//	mkship("yo");
//	loadship("yo");
//	move(0, 0, 1, 0);
	short adventuretime = 300;//nope, make that "char timetillpotentialtospawnastroidssothatwedontcheckeachtickandslowshitdown"
	//nope, make that "char timeTillPotentialToSpawnAstroidsSoThatWeDontCheckEachTickAndSlowShitDown"
	while(1){
		if (firstLoadRequest != lastLoadRequest) {
			puts("Request detected");
			loadRequest *first = firstLoadRequest;
			loadRequest *runner = first;
			while (runner != lastLoadRequest) {
				loadRequest *next = runner->next;
				client *cli = runner->cli;
				cli->myShip = loadship(runner->name);
				if (cli->myShip == NULL) {
					printf("Failed to load ship \"%s\"\n", runner->name);
					free(cli);
				} else {
					cli->next = clientList;
					clientList = cli;
				}
				if (runner != first) free(runner); // Leave the first one allocated, so the next value of first is something new.
				runner = next;
			}
			if (runner) free(runner); // Clean up the still-allocated first one from last time.
			lastLoadRequest = first;
		}
		if(adventuretime-- == 0) {
			adventuretime = 300;
			int min = INT_MAX;
			for (conductor = listrootsector; conductor; conductor = conductor->nextsector) {
				if (conductor->topGuarantee == NULL) continue;
				if (conductor->realnumber == 0) {
					//spawnstroids(conductor);
				}
				if (conductor->topGuarantee->pto < min)
					min = conductor->topGuarantee->pto;
			}
			if (min) {
				for (conductor = listrootsector; conductor; conductor = conductor->nextsector) {
					conductor->topGuarantee->pto -= min;
				}
			}
		}
		conductor = listrootsector;
		while(conductor != NULL){
			run(conductor);
			conductor = conductor->nextsector;
		}
		doStep();
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
