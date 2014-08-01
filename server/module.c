#include <stdlib.h>
#include <math.h>
#include "globals.h"

module missileModule;
module lazorModule;
module bayModule;
module miningModule;
typedef struct{
	entity* target;
	int counter;
}miningModuleData;

static void missileInit(entity* who, int ix, double value){
	who->modules[ix] = &missileModule;
	who->moduleDatas[ix] = calloc(1, 1);
}

static void lazorInit(entity* who, int ix, double value){
	who->modules[ix] = &lazorModule;
	who->moduleDatas[ix] = calloc(1, 1);
}

static void miningInit(entity* who, int ix, double value){
	who->modules[ix] = &miningModule;
	miningModuleData* data = malloc(sizeof(miningModuleData));
	data->target = NULL;
	who->moduleDatas[ix] = data;
}

static void bayInit(entity* who, int ix, double value){
	who->modules[ix] = &bayModule;
	who->moduleDatas[ix] = calloc(1, 1);
}

static void realCleanup(entity* who, int ix){
	free(who->moduleDatas[ix]);
	who->modules[ix] = NULL;
}

static void missileAct(entity* who, int ix, char action){
	char* charge = (char*)who->moduleDatas[ix];
	char random = rand()%256;
	if(*charge < 60){
		(*charge)++;
		return;
	}
	if(!action || who->energy < 60) return;
	*charge = 1;
	who->energy -= 60;
	entity* what = newEntity(1, 1, who->faction, who->mySector, who->x, who->y);
	what->vx = who->vx + who->cosTheta*60 + -who->sinTheta*random*.5;
	what->vy = who->vy + who->sinTheta*60 + who->cosTheta*random*.5;
	what->theta = who->theta;
	what->targetLock = who->targetLock;
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
		dx = dx*dx+dy*dy-who->r-runner->r;
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

static void bayAct(entity *who, int ix, char action){
	char* charge = (char*)who->moduleDatas[ix];
	if(*charge < 120){
		(*charge)++;
		return;
	}
	if(!action || who->energy < 100) return;
	*charge = 1;
	who->energy -= 100;
	entity* what = newEntity(2, 2, who->faction, who->mySector, who->x, who->y);
	what->vx = who->vx;
	what->vy = who->vy;
	what->theta = who->theta;
}

static void miningAct(entity* who, int ix, char action){
	miningModuleData* data = (miningModuleData*)who->moduleDatas[ix];
	if(!action || who->energy<2){
		data->target = NULL;
		return;
	}
	if(data->target){
		if(data->target->destroyFlag || data->target->shield <= 0){
			data->target = NULL;
		}else{
			int64_t dx = displacementX(who, data->target);
			int64_t dy = displacementY(who, data->target);
			double dist = sqrt(dx*dx + dy*dy);
			if(dist <= who->r + data->target->r + 500){
				data->counter += 3;
				addTrail(who, data->target, 1);
				who->energy -= 2;
				if(data->counter >= data->target->maxShield){
					data->target->shield = -1000; // Very dead
					who->minerals += data->target->r*data->target->r/3;
					data->target = NULL;
					return;
				}
			}else{
				data->target = NULL;
			}
		}
	}
	linkNear(who, 8500);
	entity* runner = who->mySector->firstentity;
	int bestDist = 501;
	while(runner){
		if(runner->faction != 0){
			runner = runner->next;
			continue;
		}
		int64_t dx = displacementX(who, runner);
		int64_t dy = displacementY(who, runner);
		int dist = sqrt(dx*dx + dy*dy) - who->r - runner->r;
		if(dist < bestDist){
			bestDist = dist;
			data->target = runner;
		}
		runner = runner->next;
	}
	unlinkNear();
	data->counter = 0;
}

void initModules(){
	missileModule.initFunc=missileInit;
	missileModule.actFunc=missileAct;
	missileModule.cleanupFunc=realCleanup;
	lazorModule.initFunc=lazorInit;
	lazorModule.actFunc=lazorAct;
	lazorModule.cleanupFunc=realCleanup;
	bayModule.initFunc=bayInit;
	bayModule.actFunc=bayAct;
	bayModule.cleanupFunc=realCleanup;
	miningModule.initFunc=miningInit;
	miningModule.actFunc=miningAct;
	miningModule.cleanupFunc=realCleanup;
}
