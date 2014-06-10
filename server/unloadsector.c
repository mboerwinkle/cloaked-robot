#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "globals.h"
int unloadsector(sector *target){
	writesectortofile(target->x, target->y);
	printf("unloadsector called %ld, %ld\n", target->x, target->y);
	entity* conductor  = target->firstentity;
	entity *tmp;
	while(conductor){
		tmp = conductor->next;
		freeEntity(conductor);
		conductor = tmp;
	}
	free(target);
	return(0);
}
int writesectortofile(uint64_t x,uint64_t y){
	//writedatshit
	return(0);
}
