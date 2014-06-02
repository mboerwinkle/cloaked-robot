#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"

int main(){
	sector *conductor;
	listrootsector = NULL;
	//start network listening thread
	while(1){
		conductor = listrootsector;
		while(conductor != NULL){
			run(conductor);
			conductor = conductor->nextsector;
		}
	}
	return(0);
}
