#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "globals.h"
int unloadsector(long long int x, long long int y){
	printf("unloadsector called %lld, %lld\n", x, y);
	sector *conductor;
	sector *tmp;
	conductor = listrootsector;
	while(conductor->nextsector != NULL && (conductor->nextsector->x != x || conductor->nextsector->y != y)){
		conductor = conductor->nextsector;
	}
	if(conductor->nextsector == NULL){
		return(-1);
	}
	tmp = conductor->nextsector;
	conductor->nextsector = conductor->nextsector->nextsector;
	//free all branching lists
	free(tmp);
	return(0);
}
