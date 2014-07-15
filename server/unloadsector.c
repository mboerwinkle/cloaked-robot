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
void writeentitytofile(entity *who){
		fprintf(fp, "entity\n");
		wint(who->type);
		wdouble(who->vx);
		wdouble(who->vy);
		wdouble(who->r);
		wlint(who->x);
		wlint(who->y);
		wdouble(who->shield);
		wdouble(who->maxShield);
		wdouble(who->energy);
		wdouble(who->maxEnergy);
		wdouble(who->energyRegen);
		wint(who->turn);
		wint(who->maxTurn);
		wdouble(who->thrust);
		wchar(who->theta);
		wint(who->numModules);
		fprintf(fp, "end\n");
}
int writesectortofile(sector *target){
	char name[35];
	sprintf(name, "sectors/%lX_%lX", target->x, target->y);
	fp = fopen(name, "w");
	entity *conductor = target->firstentity;
	while(conductor != NULL){
		writeentitytofile(conductor);
		conductor = conductor->next;
	}
	fclose(fp);
	return(0);
}
