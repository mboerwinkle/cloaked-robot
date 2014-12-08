#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "globals.h"

module missileModule;
module lazorModule;
module gunModule;
module bayModule;
module miningModule;
module miningBayModule;
module spawnModule;

#define vecToTheta(x, y) ((int)((atan2(y, x) + M_PI*(2+1.0/16)) / (M_PI/8)) % 16)

typedef struct{
	entity* target;
	int counter;
}miningModuleData;

typedef struct {
	char charge;
	int cost, type, aiType;
} bayModuleData;

#define MAX_MINING_DRONES 8
typedef struct {
	entity *(dronesDeployed[MAX_MINING_DRONES]);
	entity *(asteroidsTargeted[MAX_MINING_DRONES]);
	int numDeployed;
	int charge;
} miningBayModuleData;

static void missileInit(entity* who, int ix, double value){
	who->modules[ix] = &missileModule;
	who->moduleDatas[ix] = calloc(1, 1);
}

static void lazorInit(entity* who, int ix, double value){
	who->modules[ix] = &lazorModule;
	who->moduleDatas[ix] = calloc(1, 1);
}

static void gunInit(entity* who, int ix, double value){
	who->modules[ix] = &gunModule;
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
	bayModuleData *data = malloc(sizeof(bayModuleData));
	who->moduleDatas[ix] = data;
	data->charge = 0;
	data->type = (int)value;
	data->aiType = 100 * (value - data->type);
	entity *killMe = newEntity(data->type, data->aiType, who->faction, who->mySector, 0, 0);
	data->cost = killMe->r * killMe->r + killMe->minerals;
	who->mySector->firstentity = killMe->next;
	if (killMe->myAi->loadSector)
		disappear(killMe->mySector->x, killMe->mySector->y);
	freeEntity(killMe);
}

static void miningBayInit(entity *who, int ix, double value)
{
	who->modules[ix] = &miningBayModule;
	miningBayModuleData *data = malloc(sizeof(miningBayModuleData));
	who->moduleDatas[ix] = data;
	data->numDeployed = 0;
	data->charge = 0;
}

static void realCleanup(entity* who, int ix){
	free(who->moduleDatas[ix]);
	who->modules[ix] = NULL;
}

static void missileAct(entity* who, int ix, char action){
	char* charge = (char*)who->moduleDatas[ix];
	if(*charge < 60){
		(*charge)++;
		return;
	}
	char rand = random()%256;
	if(!action || who->energy < MISSILE_E_COST) return;
	*charge = 1;
	who->energy -= MISSILE_E_COST;
	entity* what = newEntity(1, 1, who->faction, who->mySector, who->x, who->y);
	what->vx = who->vx + who->cosTheta*60 + -who->sinTheta*rand*.5;
	what->vy = who->vy + who->sinTheta*60 + who->cosTheta*rand*.5;
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

static void gunAct(entity* who, int ix, char action){
	char* charge = (char*)who->moduleDatas[ix];
	if(*charge < 40){
		(*charge)++;
		return;
	}
	if(!action || who->energy < 50) return;
	double cos, sin;
	if (who->targetLock == NULL) {
		cos = who->cosTheta;
		sin = who->sinTheta;
	} else {
		int64_t rx = displacementX(who, who->targetLock);
		int64_t ry = displacementY(who, who->targetLock);
		double dvx = who->targetLock->vx - who->vx;
		double dvy = who->targetLock->vy - who->vy;
		double vy = sqrt(dvx*dvx + dvy*dvy);
		double unx = dvx / vy;
		double uny = dvy / vy;
		double y = -unx*rx - uny*ry;
		double x = uny*rx - unx*ry;
#define bulletV 110
		/*
		x / bulletV / cos = y / (vy - bulletV * sin)
		x * (vy - bulletV * sin) = y * bulletV * cos
		x*vy - x*bulletV*sin = y*bulletV*cos
		x*vy - x*bulletV*sin - y*bulletV*cos = 0;
		*/

		if (x == 0) {
			if (y > 0) {
				cos = -unx;
				sin = -uny;
			} else {
				cos = unx;
				sin = uny;
			}
		} else {
			char flopSides;
			if (x < 0) {
				x = -x;
				flopSides = 1;
			} else {
				flopSides = 0;
			}
			double c = x * vy / bulletV;
			double factor = y/x;
			int r = fabs(y);
			// x*vy/bulletV = x*sin + y*cos;
			// So a circle w/ radius r
			// Intersects a line w/ y-intercept c*abs(factor)
			//		    and x-intercept c*sgn(factor)
			//The magnitude of the normal vector we're using, which is <factor, 1>
			double mag = sqrt(factor*factor + 1);
			factor /= mag;
			double closestApproach = c * fabs(factor);
			if (closestApproach > r) {
				//He can't be hit :'(
				return;
			}
			double normFrac = closestApproach / r;
			double tangFrac = sqrt(1 - normFrac*normFrac);
			mag = 1/mag;
			//So now the *unit* normal vector is <factor, mag>
			cos = normFrac * factor + tangFrac * mag;
			if (cos >= 0) {
				sin = normFrac * mag - tangFrac * factor;
			} else {
				cos = normFrac * factor - tangFrac * mag;
				if (cos < 0)
					return;
				sin = normFrac * mag + tangFrac * factor;
			}
			if (flopSides) cos = -cos;

			double oldSin = sin;
			sin = -cos*unx + sin*uny;
			cos = oldSin*unx + cos*uny;
		}
	}

	*charge = 1;
	who->energy -= 50;


	entity* what = newEntity(6, 6, who->faction, who->mySector, who->x, who->y);
	what->vx = who->vx + cos*bulletV;
	what->vy = who->vy + sin*bulletV;
	what->theta = vecToTheta(cos, sin);
}

static void bayAct(entity *who, int ix, char action){
	bayModuleData *data = (bayModuleData*)who->moduleDatas[ix];
	char* charge = &data->charge;
	if(*charge < 120){
		(*charge)++;
		return;
	}
	if(!action || who->energy < 100 || who->minerals < data->cost) return;
	*charge = 1;
	who->energy -= 100;
	who->minerals -= data->cost;
	entity* what = newEntity(data->type, data->aiType, who->faction, who->mySector, who->x+10*who->cosTheta, who->y+10*who->sinTheta);
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
			if(dist <= who->r + data->target->r + miningRange){
				data->counter += 3;
				addTrail(who, data->target, 1);
				who->energy -= 2;
				if(data->counter >= data->target->maxShield){
					data->target->shield = -1000; // Very dead
					double minerals = data->target->r*data->target->r + data->target->minerals;
					int size = (2.0/3)*minerals;
					who->minerals += minerals - size + (size%(704*704)%(320*320));
					data->target = NULL;
				}
				return;
			}else{
				data->target = NULL;
			}
		}
	}
	data->counter = 0;
	int64_t dx, dy;
	if (who->targetLock && who->targetLock->faction == 0) {
		dx = displacementX(who, who->targetLock);
		dy = displacementY(who, who->targetLock);
		int dist = sqrt(dx*dx + dy*dy) - who->r - who->targetLock->r;
		if(dist <= miningRange)
			data->target = who->targetLock;
		return;
	}
	linkNear(who, 1000*64+miningRange);
	entity* runner = who->mySector->firstentity;
	int bestDist = miningRange + 1;
	while(runner){
		if (runner->faction != 0) {
			runner = runner->next;
			continue;
		}
		dx = displacementX(who, runner);
		dy = displacementY(who, runner);
		int dist = sqrt(dx*dx + dy*dy) - who->r - runner->r;
		if(dist < bestDist){
			bestDist = dist;
			data->target = runner;
		}
		runner = runner->next;
	}
	unlinkNear();
}

static void miningBayAct(entity *who, int ix, char action)
{
	miningBayModuleData* data = (miningBayModuleData*)who->moduleDatas[ix];
	int i = data->numDeployed-1;
	for (; i >= 0; i--) {
		if (data->dronesDeployed[i]->destroyFlag) {
			data->numDeployed--;
			data->dronesDeployed[i] = data->dronesDeployed[data->numDeployed];
			data->asteroidsTargeted[i] = data->asteroidsTargeted[data->numDeployed];
			continue;
		}
		if (data->asteroidsTargeted[i] && data->asteroidsTargeted[i]->destroyFlag)
			data->asteroidsTargeted[i] = NULL;
	}
	if(data->charge < 8){
		data->charge++;
		return;
	}
	if(!action || data->numDeployed == MAX_MINING_DRONES || who->minerals < 352*352) return;
	entity *runner = who->mySector->firstentity;
	int64_t dx, dy;
	int bestScore = miningRange*4+1;
	entity *winner = NULL;
	linkNear(who, miningRange*4);
	while (runner) {
		if (runner->faction != 0) {
			runner = runner->next;
			continue;
		}
		dx = displacementX(who, runner);
		dy = displacementY(who, runner);
		int dist = sqrt(dx*dx + dy*dy);
		if (dist < bestScore) {
			for (i = data->numDeployed-1; i >= 0; i--) {
				if (runner == data->asteroidsTargeted[i])
					break;
			}
			if (i < 0) {
				winner = runner;
				bestScore = dist;
			}
		}
		runner = runner->next;
	}
	unlinkNear();
	if (winner == NULL)
		return;
	entity* what = newEntity(8, 8, who->faction, who->mySector, who->x, who->y);
	data->charge = 1;
	who->minerals -= 352*352;
	((minorMinerAiData*)what->aiFuncData)->home = who;
	data->asteroidsTargeted[data->numDeployed] = winner;
	data->dronesDeployed[data->numDeployed] = what;
	data->numDeployed++;
	what->vx = who->vx;
	what->vy = who->vy;
	what->theta = vecToTheta(displacementX(what, winner), displacementY(what, winner));
	what->targetLock = winner;
}

void initModules(){
	missileModule.initFunc=missileInit;
	missileModule.actFunc=missileAct;
	missileModule.cleanupFunc=realCleanup;
	lazorModule.initFunc=lazorInit;
	lazorModule.actFunc=lazorAct;
	lazorModule.cleanupFunc=realCleanup;
	gunModule.initFunc=gunInit;
	gunModule.actFunc=gunAct;
	gunModule.cleanupFunc=realCleanup;
	bayModule.initFunc=bayInit;
	bayModule.actFunc=bayAct;
	bayModule.cleanupFunc=realCleanup;
	miningModule.initFunc=miningInit;
	miningModule.actFunc=miningAct;
	miningModule.cleanupFunc=realCleanup;
	miningBayModule.initFunc=miningBayInit;
	miningBayModule.actFunc=miningBayAct;
	miningBayModule.cleanupFunc=realCleanup;
}
