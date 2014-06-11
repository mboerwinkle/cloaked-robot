#include <stdlib.h>
#include <math.h>
#include "globals.h"

module missileModule;

static void missileInit(entity* who, int ix, double value){
	who->modules[ix] = &missileModule;
	who->moduleDatas[ix] = calloc(1, 1);
}

static void realCleanup(entity* who, int ix){
	free(who->moduleDatas[ix]);
	who->modules[ix] = NULL;
}

static void missileAct(entity* who, int ix, char action){
	char* charge = (char*)who->moduleDatas[ix];
	if(*charge < 10){
		(*charge)++;
		return;
	}
	if(!action || who->energy < 10) return;
	*charge = 1;
	who->energy -= 10;
	entity* target = who->mySector->firstentity;
	while(target){
		if(target != who) break;
		target = target->next;
	}
	entity* what = newEntity(1, who->mySector, who->x, who->y);
	what->vx = who->vx;
	what->vy = who->vy;
	what->theta = who->theta;
	*(entity**)what->aiFuncData = target;
	appear(who->mySector->x, who->mySector->y);//TODO: Tidy this up.
}

void initModules(){
	missileModule.initFunc=missileInit;
	missileModule.actFunc=missileAct;
	missileModule.cleanupFunc=realCleanup;
}
