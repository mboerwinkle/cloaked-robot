#include <stdlib.h>
#include <math.h>
#include "globals.h"

module missileModule;

static void missileInit(entity* who, int ix, double value){
	who->modules[ix] = &missileModule;
	who->moduleDatas[ix] = calloc(1, sizeof(double));
}

static void realCleanup(entity* who, int ix){
	free(who->moduleDatas[ix]);
	who->modules[ix] = NULL;
}

static void missileAct(entity* who, int ix, double energy, char action){
	double* charge = (double*)who->moduleDatas[ix];
	*charge += energy;
	if(*charge < 10 || !action) return;
	*charge = 0;
	entity* what = newEntity(1, who->x, who->y);
	entity* target = mySector.firstentity;
	while(target){
		if(target != who) break;
		target = target->next;
	}
	what->vx = who->vx;
	what->vy = who->vy;
	what->theta = who->theta;
	*(entity**)what->aiFuncData = target;
	addEntity(what);
}

void initModules(){
	missileModule.initFunc=missileInit;
	missileModule.actFunc=missileAct;
	missileModule.cleanupFunc=realCleanup;
}
