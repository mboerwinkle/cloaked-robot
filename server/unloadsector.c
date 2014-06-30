#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "globals.h"
static FILE *fp;
void wchar(char target){
	fprintf(fp, "%c\n", target);
}
void wint(int target){
	fprintf(fp, "%d\n", target);
}
void wlint(int32_t target){
	fprintf(fp, "%d\n", target);
}
void wdouble(double target){
	fprintf(fp, "%f\n", target);
}
int unloadsector(sector *target){
	writesectortofile(target);
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
int writesectortofile(sector *target){
	char name[35];
	sprintf(name, "sectors/%lX_%lX", target->x, target->y);
	fp = fopen(name, "w");
	entity *conductor = target->firstentity;
	while(conductor != NULL){
		fprintf(fp, "entity\n");
		wint(conductor->type);
		wdouble(conductor->vx);
		wdouble(conductor->vy);
		wdouble(conductor->r);
		wlint(conductor->x);
		wlint(conductor->y);
		wdouble(conductor->shield);
		wdouble(conductor->maxShield);
		wdouble(conductor->energy);
		wdouble(conductor->maxEnergy);
		wdouble(conductor->energyRegen);
		wint(conductor->turn);
		wint(conductor->maxTurn);
		wdouble(conductor->thrust);
		wchar(conductor->theta);
		wint(conductor->numModules);
		fprintf(fp, "end\n");
		conductor = conductor->next;
	}
	fclose(fp);
	return(0);
}
