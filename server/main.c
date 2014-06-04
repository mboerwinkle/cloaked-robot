#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "globals.h"

int main(){
	sector *conductor;
	listrootsector = NULL;
	initModules();
	//start network listening thread
	pthread_t id;
	pthread_create(&id, NULL, netListen, NULL);
	mkship("yo");
	loadship("yo");
	while(1){
		conductor = listrootsector;
		while(conductor != NULL){
			run(conductor);
			conductor = conductor->nextsector;
		}
	}
	return(0);
}
