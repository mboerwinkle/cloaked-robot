#include <stdlib.h>
#include <math.h>
#include "globals.h"

module turnModule;
module thrustModule;
module missileModule;

static void turnInit(entity* who, int ix, double value){
	who->modules[ix] = &turnModule;
	who->moduleDatas[ix] = malloc(sizeof(double));
	*(double*)who->moduleDatas[ix] = value;
}

static void thrustInit(entity* who, int ix, double value){
	who->modules[ix] = &thrustModule;
	who->moduleDatas[ix] = malloc(sizeof(double));
	*(double*)who->moduleDatas[ix] = value;
}

static void missileInit(entity* who, int ix, double value){
	who->modules[ix] = &missileModule;
	who->moduleDatas[ix] = calloc(1, sizeof(double));
}

static void realCleanup(entity* who, int ix){
	free(who->moduleDatas[ix]);
	who->modules[ix] = NULL;
}

/*static void fauxCleanup(entity* who, int ix){
	who->modules[ix] = NULL;
}*/

static void turnAct(entity* who, int ix, double energy){
	who->theta += *((double*)who->moduleDatas[ix])*energy;
	while(who->theta > 2*M_PI) who->theta -= 2*M_PI;
	while(who->theta < -2*M_PI) who->theta += 2*M_PI;
}

static void thrustAct(entity* who, int ix, double energy){
	thrust(who, *((double*)who->moduleDatas[ix])*energy);
}

static void missileAct(entity* who, int ix, double energy){
	double* charge = (double*)who->moduleDatas[ix];
	*charge += 1.0;
	if(*charge < 10) return;
	*charge -= 10;
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
	turnModule.initFunc=turnInit;
	turnModule.actFunc=turnAct;
	turnModule.cleanupFunc=realCleanup;

	thrustModule.initFunc=thrustInit;
	thrustModule.actFunc=thrustAct;
	thrustModule.cleanupFunc=realCleanup;

	missileModule.initFunc=missileInit;
	missileModule.actFunc=missileAct;
	missileModule.cleanupFunc=realCleanup;
}
