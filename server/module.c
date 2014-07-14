#include <stdlib.h>
#include <math.h>
#include "globals.h"

module missileModule;
module lazorModule;

static void missileInit(entity* who, int ix, double value){
	who->modules[ix] = &missileModule;
	who->moduleDatas[ix] = calloc(1, 1);
}

static void lazorInit(entity* who, int ix, double value){
	who->modules[ix] = &lazorModule;
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
	entity* what = newEntity(1, 1, 0, who->mySector, who->x, who->y);
	what->vx = who->vx;
	what->vy = who->vy;
	what->theta = who->theta;
	what->targetLock = who->targetLock;
	what->faction = who->faction;
	appear(who->mySector->x, who->mySector->y);//TODO: Tidy this up.
}

static void lazorAct(entity* who, int ix, char action){
	char* charge = (char*)who->moduleDatas[ix];
	if(*charge < 10){
		(*charge)++;
		return;
	}
	if(!action || who->energy < 20) return;
	linkNear(who, 6400);
	entity* target = NULL;
	entity* runner = who->mySector->firstentity;
	int64_t bestDist = (int64_t)6400*6400;
	while(runner){
		if(runner == who || !(who->lockSettings & (1<<runner->faction))){
			runner = runner->next;
			continue;
		}
		int64_t dx = displacementX(who, runner);
		int64_t dy = displacementY(who, runner);
		dx = dx*dx+dy*dy;
		if(dx > 0 /*no overflows*/ && dx < bestDist){
			bestDist = dx;
			target = runner;
		}
		runner = runner->next;
	}
	unlinkNear();
	if(target == NULL) return;
	*charge = 1;
	who->energy -= 20;
	target->shield -= 5;
	addTrail(who, target, 0);
}

void initModules(){
	missileModule.initFunc=missileInit;
	missileModule.actFunc=missileAct;
	missileModule.cleanupFunc=realCleanup;
	lazorModule.initFunc=lazorInit;
	lazorModule.actFunc=lazorAct;
	lazorModule.cleanupFunc=realCleanup;
}
