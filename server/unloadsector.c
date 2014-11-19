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
	free(target);
	return(0);
}
void writeentitytofile(entity *who){
		fprintf(fp, "entity\n");
		wint(who->type);
		if(who->myAi == &aiHuman) wint(0);
		else if(who->myAi == &aiMissile) wint(1);
		else if(who->myAi == &aiDrone) wint(2);
		else if(who->myAi == &aiAsteroid) wint(3);
		else if(who->myAi == &aiPacer) wint(4);
		else if(who->myAi == &aiBullet) wint(6);
		else perror("Unknown ai in unloadsector.c\n");
		wint(who->faction);
		wlint(who->x);
		wlint(who->y);
		fprintf(fp, "end\n");
}
int writesectortofile(sector *target){
	char name[35];
	sprintf(name, "sectors/%lX_%lX", target->x, target->y);
	fp = fopen(name, "w");
	entity *next, *conductor = target->firstentity;
	while(conductor != NULL){
		writeentitytofile(conductor);
		next = conductor->next;
		freeEntity(conductor);
		conductor = next;
	}
	fclose(fp);
	return(0);
}
void addmetosector(entity* who, uint64_t x, uint64_t y){
	char name[35];
	sprintf(name, "sectors/%lX_%lX", x, y);
	fp = fopen(name, "a");
	who->x = (int64_t)(POS_MAX-POS_MIN+1)*random()/RAND_MAX + POS_MIN;
	who->y = (int64_t)(POS_MAX-POS_MIN+1)*random()/RAND_MAX + POS_MIN;
	writeentitytofile(who);
	fclose(fp);
}
