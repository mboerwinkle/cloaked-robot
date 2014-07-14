#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include "globals.h"

entity* newEntity(int type, int aiType, char faction, sector *where, int32_t x, int32_t y){
	if(where == NULL) return NULL;
	entity* ret = malloc(sizeof(entity));
	ret->faction = faction;
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
	ret->trailTargets = malloc(sizeof(entity*)*2);
	ret->numTrails = 0;
	ret->maxTrails = 2;
	if(type == 0){
		ret->x += 5000 + POS_MIN;
		ret->y += 5000 + POS_MIN;
		ret->r = 640;
		ret->numModules = 2;
		ret->modules = calloc(2, sizeof(void *));
		ret->moduleDatas = calloc(2, sizeof(void*));
		ret->thrust = 3;
		ret->maxTurn = 6;
		ret->shield = ret->maxShield = 100;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*missileModule.initFunc)(ret, 0, 1);
		(*lazorModule.initFunc)(ret, 1, 1);
	}else if(type == 1){
		ret->r = 64;
		ret->numModules = 0;
		ret->modules = NULL;
		ret->moduleDatas = NULL;
		ret->thrust = 4;
		ret->maxTurn = 2;
		ret->shield = ret->maxShield = 5;
	}else if(type == 2){		
		ret->x += 5000 + POS_MIN;
		ret->y += 5000 + POS_MIN;
		ret->r = 640;
		ret->numModules = 1;
		ret->modules = calloc(1, sizeof(void *));
		ret->moduleDatas = calloc(1, sizeof(void*));
		ret->thrust = 3;
		ret->maxTurn = 6;
		ret->shield = ret->maxShield = 100;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*missileModule.initFunc)(ret, 0, 1);
	}
	if(aiType == 0){
		ret->myAi = &aiHuman;
		ret->aiFuncData = malloc(sizeof(humanAiData));
		((humanAiData*)ret->aiFuncData)->lockSettings = 2;
	}else if(aiType == 1){
		ret->myAi = &aiMissile;
		ret->aiFuncData = calloc(2, 1);
	}else if(aiType == 2){
		ret->myAi = &aiDrone;
		ret->aiFuncData = malloc(sizeof(droneAiData));
		((droneAiData*)ret->aiFuncData)->Attack = 0;
		((droneAiData*)ret->aiFuncData)->timer = 100;
	}
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
	who->numTrails = 0;
	who->energy += who->energyRegen;
	if(who->energy > who->maxEnergy) who->energy = who->maxEnergy;
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
	if(v <= 0.5){
		who->vx = 0;
		who->vy = 0;
	}else{
		vx /= v;
		vy /= v;
		who->vx -= vx*0.5;
		who->vy -= vy*0.5;
		who->x += who->vx;
		who->y += who->vy;
	}
	who->actedFlag = globalActedFlag;
	linkNear(who, 6400);
	entity *otherGuy = who->mySector->firstentity;
	double dx, dy, d;

	while(otherGuy){
		if(otherGuy->actedFlag != globalActedFlag || otherGuy == who){
			otherGuy = otherGuy->next;
			continue;
		}
		dx = displacementX(who, otherGuy);
		dy = displacementY(who, otherGuy);
		d = sqrt(dx*dx+dy*dy);
		if(d > who->r + otherGuy->r || d==0){
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
	if(who->shield <= 0) return 1;
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
		if(new == NULL) return 1;
		fileMoveRequest(who, who->mySector, new);
		who->mySector = new;
	}
	return 0;
}

void thrust(entity* who){
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
	free(who);
}
