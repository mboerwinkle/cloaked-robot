#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "globals.h"
int unloadsector(sector *target){
	writesectortofile(target->x, target->y);
	printf("unloadsector called %lld, %lld\n", target->x, target->y);
	//free all entities
	free(target);
	return(0);
}
int writesectortofile(long long int x, long long int y){
	//writedatshit
	return(0);
}
