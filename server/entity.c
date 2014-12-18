#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include "globals.h"

//Note that creating an entity which does *not* load sectors, but spawns (using the bay module) entities that *do* load sectors, you might get a few spurious sector load / unload pairs when said entity is created.
entity* newEntity(int type, int aiType, char faction, sector *where, int32_t x, int32_t y){
	if(where == NULL) return NULL;
	entity* ret = malloc(sizeof(entity));
	ret->faction = faction;
	if(faction == 0) ret->lockSettings = 15;
	if(faction == 1) ret->lockSettings = 12;
	if(faction == 2) ret->lockSettings = 2;
	if(faction == 3) ret->lockSettings = 3;
	ret->actedFlag = globalActedFlag;
	ret->type = type;
	ret->destroyFlag = 0;
	ret->targetLock = NULL;
	ret->x = x;
	ret->y = y;
	ret->vx = ret->vy = ret->theta = 0;
	ret->sinTheta = 0;
	ret->cosTheta = 1;
	ret->turn = 0;
	ret->next = where->firstentity;
	where->firstentity = ret;
	ret->mySector = where;
	ret->minerals = 0;
	ret->trailTargets = malloc(sizeof(entity*)*2);
	ret->trailTypes = malloc(sizeof(int)*2);
	ret->numTrails = 0;
	ret->maxTrails = 2;
#define hasModules(n) \
	ret->numModules = n;\
	ret->modules = calloc(n, sizeof(void*));\
	ret->moduleDatas = calloc(n, sizeof(void*))
	if(type == 0){//human
		ret->r = 640;
		hasModules(3);
		ret->thrust = 3;
		ret->maxTurn = 6;
		ret->shield = ret->maxShield = 100;
		ret->shieldRegen = 0.05;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*missileModule.initFunc)(ret, 0, 1);
		(*lazorModule.initFunc)(ret, 1, 1);
		(*miningModule.initFunc)(ret, 2, 1);
	}else if(type == 1){//missile
		ret->r = 64;
		hasModules(0);
		ret->thrust = 3.5;
		ret->maxTurn = 2;
		ret->shield = ret->maxShield = 5;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	}else if(type == 2){//drone	
		ret->r = 640;
		hasModules(1);
		ret->thrust = 2;
		ret->maxTurn = 6;
		ret->shield = ret->maxShield = 100;
		ret->shieldRegen = 0.05;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*gunModule.initFunc)(ret, 0, 1);
	}else if(type == 3){//carrier	
		ret->r = 3480;
		hasModules(1);
		ret->thrust = 1.515;
		ret->maxTurn = 12;
		ret->shield = ret->maxShield = 300;
		ret->shieldRegen = 0.05;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*bayModule.initFunc)(ret, 0, 2.02);
		ret->minerals = 640*640*10;
	}else if (type == 12) { // Huge asteroid
		ret->r = 1300; // By rights this ought to be 1280, but I like the asteroid distribution better this way
		hasModules(0);
		ret->thrust = 1.5;
		ret->maxTurn = 14;
		ret->shield = ret->maxShield = 170;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	}else if(type == 4){//large asteroid
		ret->r = 704;
		hasModules(0);
		ret->thrust = 1.5;
		ret->maxTurn = 7;
		ret->shield = ret->maxShield = 100;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	}else if(type == 5){//medium asteroid
		ret->r = 320;
		hasModules(0);
		ret->thrust = 1.5;
		ret->maxTurn = 5;
		ret->shield = ret->maxShield = 60;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	} else if (type == 6) { // Bullet
		ret->r = 160;
		hasModules(0);
		ret->thrust = 1.5;
		ret->maxTurn = 1;
		ret->shield = ret->maxShield = 7;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	} else if (type == 7) { // Destroyer
		ret->r = 64*24;
		hasModules(3);
		ret->thrust = 2;
		ret->maxTurn = 9;
		ret->shield = ret->maxShield = 250;
		ret->shieldRegen = .06;
		ret->energy = ret->maxEnergy = 120;
		ret->energyRegen = 2;
		(*missileModule.initFunc)(ret, 0, 1);
		(*missileModule.initFunc)(ret, 1, 1);
		(*lazorModule.initFunc)(ret, 2, 1);
	} else if (type == 8) { //Minor Miner
		ret->r = 64*5+32; // 352
		hasModules(1);
		ret->thrust = 1.8;
		ret->maxTurn = 4;
		ret->shield = ret->maxShield = 10;
		ret->energy = ret->maxEnergy = 68;
		ret->shieldRegen = ret->energyRegen = 0;
		(*miningModule.initFunc)(ret, 0, 1);
		ret->lockSettings = 1;
	} else if (type == 9) { //Major Miner
		ret->r = 1280;
		hasModules(2);
		ret->minerals = 352*352*2;
		ret->thrust = 2.5;
		ret->maxTurn = 7;
		ret->shield = ret->maxShield = 250;
		ret->energy = ret->maxEnergy = 80;
		ret->shieldRegen = .05;
		ret->energyRegen = 1;
		(*miningModule.initFunc)(ret, 0, 1);
		(*miningBayModule.initFunc)(ret, 1, 1);
	} else if (type == 10) { //Planet / station
		ret->r = 4800;
		hasModules(4);
		ret->thrust = 0;
		ret->maxTurn = 20;
		ret->shield = ret->maxShield = 1000;
		ret->energy = ret->maxEnergy = 100;
		ret->shieldRegen = 0.05;
		ret->energyRegen = 2;
		//We may have to change this whole "double argument to init functions" thing, but until that point we have (shipType) + (aiType / 128)
		(*bayModule.initFunc)(ret, 0, 2 + 2.0/128);
		(*bayModule.initFunc)(ret, 1, 3 + 2.0/128);
		(*bayModule.initFunc)(ret, 2, 7 + 7.0/128);
		(*bayModule.initFunc)(ret, 3, 9 + 9.0/128);
	} else if (type == 11) { // freeze tag player
		ret->r = 640;
		hasModules(4);
		ret->thrust = 3;
		ret->maxTurn = 6;
		ret->shield = ret->maxShield = 100;
		ret->shieldRegen = 2;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*missileModule.initFunc)(ret, 0, 1);
		(*healRayModule.initFunc)(ret, 1, 1);
		(*miningModule.initFunc)(ret, 2, 1);
		(*stasisModule.initFunc)(ret, 3, 1);
	} // type 12 is a bit up from here, because it's a huge asteroid
	if(aiType == 0){
		ret->myAi = &aiHuman;
		ret->aiFuncData = malloc(sizeof(humanAiData));
		((humanAiData*)ret->aiFuncData)->keys = 0;
	}else if(aiType == 1){
		ret->myAi = &aiMissile;
		ret->aiFuncData = calloc(1, 2);
	}else if(aiType == 2){
		ret->myAi = &aiDrone;
		ret->aiFuncData = malloc(sizeof(droneAiData));
		((droneAiData*)ret->aiFuncData)->timer = 100;
		((droneAiData*)ret->aiFuncData)->repeats = 0;
		((droneAiData*)ret->aiFuncData)->target = NULL;
	}else if(aiType == 3){
		ret->myAi = &aiAsteroid;
		ret->aiFuncData = calloc(1, 1);
	} else if (aiType == 4) {
		ret->myAi = &aiPacer;
		ret->aiFuncData = NULL;
	} else if (aiType == 5) {
		puts("WTF OMG LOL BBQ");
		ret->myAi = &aiPacer;
		ret->aiFuncData = NULL;
	} else if (aiType == 6) {
		ret->myAi = &aiBullet;
		ret->aiFuncData = calloc(1, 2);
	} else if (aiType == 7) {
		ret->myAi = &aiDestroyer;
		ret->aiFuncData = malloc(sizeof(destroyerAiData));
		((destroyerAiData*)ret->aiFuncData)->shotsLeft = 0;
		((destroyerAiData*)ret->aiFuncData)->recheckTime = 1;
		((destroyerAiData*)ret->aiFuncData)->rechecks = 7;
	} else if (aiType == 8) {
		ret->myAi = &aiMinorMiner;
		ret->aiFuncData = malloc(sizeof(minorMinerAiData));
		((minorMinerAiData*)ret->aiFuncData)->phase = 1;
		((minorMinerAiData*)ret->aiFuncData)->home = NULL;
		((minorMinerAiData*)ret->aiFuncData)->pleaseTurn = 0;
	} else if (aiType == 9) {
		ret->myAi = &aiMajorMiner;
		ret->aiFuncData = malloc(sizeof(majorMinerAiData));
		((majorMinerAiData*)ret->aiFuncData)->homestation = NULL;
		((majorMinerAiData*)ret->aiFuncData)->target = NULL;
		((majorMinerAiData*)ret->aiFuncData)->recheckTime = 100;
		((majorMinerAiData*)ret->aiFuncData)->rechecks = 0;
		((majorMinerAiData*)ret->aiFuncData)->phase = 0;
	} else if (aiType == 10) {
		ret->myAi = &aiStation;
		ret->aiFuncData = calloc(1, sizeof(int));
	}
	if (ret->myAi->loadSector)
		appear(ret->mySector->x, ret->mySector->y);
	return ret;
}

void addTrail(entity* from, entity* to, char type){
	if(from->numTrails == from->maxTrails){
		from->trailTargets = realloc(from->trailTargets, sizeof(entity*)*(from->maxTrails+=2));
		from->trailTypes = realloc(from->trailTypes, sizeof(int)*from->maxTrails);
	}
	from->trailTypes[from->numTrails] = type;
	from->trailTargets[from->numTrails++] = to;
}

void tick(entity* who){
	who->thrustFlag = 0;
	who->numTrails = 0;
	who->energy += who->energyRegen;
	if(who->energy > who->maxEnergy) who->energy = who->maxEnergy;
	who->shield += who->shieldRegen;
	if(who->shield > who->maxShield) who->shield = who->maxShield;
	(*who->myAi->act)(who);
	if(who->targetLock){
		if(who->targetLock->destroyFlag){
			who->targetLock=NULL;
		}else{
			double x = displacementX(who, who->targetLock);
			double y = displacementY(who, who->targetLock);
			if(sqrt(x*x + y*y) > LOCK_RANGE) who->targetLock = NULL;
		}
	}
	who->sinTheta = sin(who->theta * (2*M_PI/16));
	who->cosTheta = cos(who->theta * (2*M_PI/16));
}

char tick2(entity* who){
	double vx = who->vx;
	double vy = who->vy;
	double v = sqrt(vx*vx + vy*vy);
	if(v <= 1.5){
		who->vx = 0;
		who->vy = 0;
	}else{
		vx /= v;
		vy /= v;
		who->vx -= vx*1.5;
		who->vy -= vy*1.5;
		who->x += who->vx;
		who->y += who->vy;
	}
	who->actedFlag = globalActedFlag;
	linkNear(who, 64*1000);
	entity *otherGuy = who->mySector->firstentity;
	double dx, dy, d, r;

	while(otherGuy){
		if(otherGuy->actedFlag != globalActedFlag || otherGuy == who){
			otherGuy = otherGuy->next;
			continue;
		}
		r = who->r + otherGuy->r;
		dx = displacementX(who, otherGuy);
		if (dx > r) {
			otherGuy = otherGuy->next;
			continue;
		}
		dy = displacementY(who, otherGuy);
		if (dy > r) {
			otherGuy = otherGuy->next;
			continue;
		}
		if (dx == 0 && dy == 0) {
			otherGuy = otherGuy->next;
			continue;
		}
		d = sqrt(dx*dx+dy*dy);
		if(d > r){
			otherGuy = otherGuy->next;
			continue;
		}
		dx/=d;
		dy/=d;
		double dvel = (who->vx-otherGuy->vx)*dx+(who->vy-otherGuy->vy)*dy;
		if(dvel > 0){
			(*who->myAi->handleCollision)(who, otherGuy);
			(*otherGuy->myAi->handleCollision)(otherGuy, who);
			double m1 = who->r;
			double m2 = otherGuy->r;
			dvel *= 2*m1/(m1+m2);
			otherGuy->vx += dvel*dx;
			otherGuy->vy += dvel*dy;
			dvel *= -m2/m1;
			who->vx += dvel*dx;
			who->vy += dvel*dy;
		}
		otherGuy = otherGuy->next;
	}
	unlinkNear();
	if(who->shield <= 0) return 2;
	uint64_t secx = who->mySector->x;
	uint64_t secy = who->mySector->y;
	if(who->x > POS_MAX){
		secx++;
		who->x -= (POS_MAX-POS_MIN+1);
	}else if(who->x < POS_MIN){
		secx--;
		who->x += (POS_MAX-POS_MIN+1);
	}
	if(who->y > POS_MAX){
		secy++;
		who->y -= (POS_MAX-POS_MIN+1);
	}else if(who->y < POS_MIN){
		secy--;
		who->y += (POS_MAX-POS_MIN+1);
	}
	if(secx!=who->mySector->x || secy!=who->mySector->y){
		sector *new = searchforsector(secx, secy);
		if(new == NULL){
			addmetosector(who, secx, secy);
			return 1;
		}
		fileMoveRequest(who, who->mySector, new);
		who->mySector = new;
	}
	return 0;
}

void thrust(entity* who){
	if (who->thrustFlag) {
		puts("It appears we're thrusting twice in a tick?");
		puts("Feel free to comment out these lines if it's okay.");
	}
	who->thrustFlag = 1;
	who->vx += who->thrust*who->cosTheta;
	who->vy += who->thrust*who->sinTheta;
}

void turn(entity* who, char dir){
	who->turn += dir;
	if(who->turn >= who->maxTurn){
		who->turn += 1 - 2*who->maxTurn;
		if(++who->theta >= 16) who->theta-=16;
	}
	else if(who->turn <= -who->maxTurn){
		who->turn -= 1 - 2*who->maxTurn;
		if(--who->theta < 0) who->theta+=16;
	}
}

void freeEntity(entity* who){
	if(who->aiFuncData) free(who->aiFuncData);
	int i = 0;
	for(; i < who->numModules; i++){
		if(who->modules[i]) (*who->modules[i]->cleanupFunc)(who, i);
	}
	if(who->modules){
		free(who->modules);
		free(who->moduleDatas);
	}
	free(who->trailTypes);
	free(who->trailTargets);
	free(who);
}
