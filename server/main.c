#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"

int main(){
	sector *conductor;
	listrootsector = malloc(sizeof(sector));
	listrootsector->nextsector = NULL;
	//start network listening thread
	while(1){
		conductor = listrootsector;
		while(conductor->nextsector != NULL){
			conductor = conductor->nextsector;
			run(conductor);
		}
	}
	return(0);
}
