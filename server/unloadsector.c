#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "globals.h"
int unloadsector(long long int x, long long int y){
	writesectortofile(x, y);
	printf("unloadsector called %lld, %lld\n", x, y);
	sector *conductor;
	if(listrootsector == NULL){
		return(-1);
	}
	conductor = listrootsector;
	if(listrootsector->x == x && listrootsector->y == y){
		listrootsector = listrootsector->nextsector;
		free(conductor);
	}
	else{
		while(conductor != NULL && (conductor->nextsector->x != x || conductor->nextsector->y != y)){
			conductor = conductor->nextsector;
		}
		if(conductor == NULL){
			return(-1);
		}
		free(conductor->nextsector);
	}
	return(0);
}
int writesectortofile(long long int x, long long int y){
	//writedatshit
	return(0);
}
