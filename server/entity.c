#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include "globals.h"

//Note that creating an entity which does *not* load sectors, but spawns (using the bay module) entities that *do* load sectors, you might get a few spurious sector load / unload pairs when said entity is created.
entity* newEntity(guarantee *creator, int type, int aiType, char faction, sector *where, int32_t x, int32_t y, int32_t vx, int32_t vy){
	if(where == NULL) return NULL;
	entity* ret = malloc(sizeof(entity));
	ret->faction = faction;
	if(faction == 0) ret->lockSettings = 15;
	if(faction == 1) ret->lockSettings = 12;
	if(faction == 2) ret->lockSettings = 2;
	if(faction == 3) ret->lockSettings = 3;
	ret->numContacts = ret->maxContacts = 0;
	ret->contacts = NULL;
	ret->actedFlag = globalActedFlag;
	ret->type = type;
	ret->destroyFlag = 0;
	ret->thrustFlag = 0;
	ret->targetLock = NULL;
	//ret->x = x;
	//ret->y = y;
	ret->theta = 0;
	ret->sinTheta = 0;
	ret->cosTheta = 1;
	ret->turn = 0;
	ret->next = where->firstentity;
	where->firstentity = ret;
	ret->mySector = where;
	ret->minerals = 0;
	ret->trailXs = malloc(sizeof(int64_t)*2);
	ret->trailYs = malloc(sizeof(int64_t)*2);
	ret->trailTypes = malloc(sizeof(int)*2);
	ret->numTrails = 0;
	ret->maxTrails = 2;
	int32_t r;
#define hasModules(n) \
	ret->numModules = n;\
	ret->modules = calloc(n, sizeof(void*));\
	ret->moduleDatas = calloc(n, sizeof(void*));\
	{\
		int32_t pos[2] = {x, y};\
		int32_t vel[2] = {vx, vy};\
		ret->me = createEntityGuarantee(creator, where, r, pos, vel, ret);\
	}
	if(type == 0){//human
		r = 640*16;
		ret->minerals = 1;
		hasModules(3);
		ret->thrust = 3*16;
		ret->maxTurn = 6;
		ret->shield = ret->maxShield = 100;
		ret->shieldRegen = 0.05;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*missileModule.initFunc)(ret, 0, 1);
		(*lazorModule.initFunc)(ret, 1, 1);
		(*miningModule.initFunc)(ret, 2, 1);
	}else if(type == 1){//missile
		r = 64*16;
		hasModules(0);
		ret->thrust = 3.5*16;
		ret->maxTurn = 2;
		ret->shield = ret->maxShield = 5;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	}else if(type == 2){//drone	
		r = 640*16;
		hasModules(1);
		ret->thrust = 2*16;
		ret->maxTurn = 6;
		ret->shield = ret->maxShield = 100;
		ret->shieldRegen = 0.05;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*gunModule.initFunc)(ret, 0, 1);
	}else if(type == 3){//carrier	
		r = 3480*16;
		hasModules(2);
		ret->thrust = 1.7*16;
		ret->maxTurn = 12;
		ret->shield = ret->maxShield = 300;
		ret->shieldRegen = 0.05;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*bayModule.initFunc)(ret, 0, 2 + 2.0/128);
		(*miningBayModule.initFunc)(ret, 1, 1);
		ret->minerals = 640*640*10+2*MINOR_MINER_COST;
	}else if (type == 12) { // Huge asteroid
		r = 1300*16; // By rights this ought to be 1280, but I like the asteroid distribution better this way
		hasModules(0);
		ret->thrust = 1.5*16;
		ret->maxTurn = 14;
		ret->shield = ret->maxShield = 170;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	}else if(type == 4){//large asteroid
		r = 704*16;
		hasModules(0);
		ret->thrust = 1.5*16;
		ret->maxTurn = 7;
		ret->shield = ret->maxShield = 100;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	}else if(type == 5){//medium asteroid
		r = 320*16;
		hasModules(0);
		ret->thrust = 1.5*16;
		ret->maxTurn = 5;
		ret->shield = ret->maxShield = 60;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	} else if (type == 6) { // Bullet
		r = 160*16;
		hasModules(0);
		ret->thrust = 1.5*16;
		ret->maxTurn = 1;
		ret->shield = ret->maxShield = 7;
		ret->shieldRegen = 0;
		ret->energy = ret->maxEnergy = ret->energyRegen = 0;
	} else if (type == 7) { // Destroyer
		r = 64*24*16;
		hasModules(3);
		ret->thrust = 2*16;
		ret->maxTurn = 9;
		ret->shield = ret->maxShield = 250;
		ret->shieldRegen = .06;
		ret->energy = ret->maxEnergy = 120;
		ret->energyRegen = 2;
		(*missileModule.initFunc)(ret, 0, 1);
		(*missileModule.initFunc)(ret, 1, 1);
		(*lazorModule.initFunc)(ret, 2, 1);
	} else if (type == 8) { //Minor Miner
		r = MINOR_MINER_R*16; // 352*16
		hasModules(1);
		ret->thrust = 1.8*16;
		ret->maxTurn = 4;
		ret->shield = ret->maxShield = 10;
		ret->energy = ret->maxEnergy = 68;
		ret->shieldRegen = ret->energyRegen = 0;
		(*miningModule.initFunc)(ret, 0, 1);
		ret->lockSettings = 1;
	} else if (type == 9) { //Major Miner
		r = 1280*16;
		hasModules(2);
		ret->minerals = MINOR_MINER_COST*2;
		ret->thrust = 2.5*16;
		ret->maxTurn = 7;
		ret->shield = ret->maxShield = 250;
		ret->energy = ret->maxEnergy = 80;
		ret->shieldRegen = .05;
		ret->energyRegen = 1;
		(*miningModule.initFunc)(ret, 0, 1);
		(*miningBayModule.initFunc)(ret, 1, 1);
	} else if (type == 10) { //Planet / station
		r = 4800*16;
		hasModules(4);
		ret->thrust = 0;
		ret->maxTurn = 20;
		ret->shield = ret->maxShield = 1000;
		ret->energy = ret->maxEnergy = 100;
		ret->shieldRegen = 0.05;
		ret->energyRegen = 2;
		//We may have to change this whole "double argument to init functions" thing, but until that point we have (shipType) + (aiType / 128)
		(*bayModule.initFunc)(ret, 0, 2 + 2.0/128);
		(*bayModule.initFunc)(ret, 1, 3 + 3.0/128);
		(*bayModule.initFunc)(ret, 2, 7 + 7.0/128);
		(*bayModule.initFunc)(ret, 3, 9 + 9.0/128);
	} else if (type == 11) { // freeze tag player
		r = 640*16;
		hasModules(4);
		ret->thrust = 3*16;
		ret->maxTurn = 6;
		ret->shield = ret->maxShield = 100;
		ret->shieldRegen = 2;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*missileModule.initFunc)(ret, 0, 1);
		(*healRayModule.initFunc)(ret, 1, 1);
		(*miningModule.initFunc)(ret, 2, 1);
		(*stasisModule.initFunc)(ret, 3, 1);
	} else {// type 12 is a bit up from here, because it's a huge asteroid
		printf("Error: Unknown ship type when creating entity: %d\n", type);
		r = 640*16;
		hasModules(0);
	}
	if((aiType&0x3F) == 0){
		ret->myAi = &aiHuman;
		humanAiData *data = malloc(sizeof(humanAiData));
		ret->aiFuncData = data;
		data->keys = 0;
		data->replayMode = aiType>>6;
		if (data->replayMode) {
			data->prevKeys = 0;
			if (1 == data->replayMode) {
				data->replayFd = open("replay.rep", O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG);
				data->dittoCounter = 0;
			} else {
				data->replayFd = open("replay.rep", O_RDONLY);
				if (sizeof(int) != read(data->replayFd, &data->dittoCounter, sizeof(int))) {
					puts("Corrupt replay file.");
					data->replayMode = 0;
				} else if (1 != read(data->replayFd, &data->keys, 1)) {
					puts("Corrupt replay file");
					data->replayMode = 0;
				}
			}
		}
	} else if (aiType == 1){
		ret->myAi = &aiMissile;
		ret->aiFuncData = calloc(1, 2);
	} else if (aiType == 2){
		ret->myAi = &aiDrone;
		ret->aiFuncData = malloc(sizeof(droneAiData));
		((droneAiData*)ret->aiFuncData)->timer = 100;
		((droneAiData*)ret->aiFuncData)->target = NULL;
	} else if (aiType == 3) {
		ret->myAi = &aiCarrier;
		ret->aiFuncData = malloc(sizeof(carrierAiData));
		((carrierAiData*)ret->aiFuncData)->timer = 100;
		((carrierAiData*)ret->aiFuncData)->target = NULL;
		((carrierAiData*)ret->aiFuncData)->mineSuccess = -1;
	} else if (aiType == 4){
		ret->myAi = &aiAsteroid;
		ret->aiFuncData = calloc(1, 1);
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
		((majorMinerAiData*)ret->aiFuncData)->phase = 0;
	} else if (aiType == 10) {
		ret->myAi = &aiStation;
		ret->aiFuncData = calloc(1, sizeof(int));
	} else {
		printf("Error in entity.c: Unknown aiType %d\n", aiType);
		ret->myAi = &aiPacer;
		ret->aiFuncData = NULL;
	}
	if (ret->myAi->loadSector)
		appear(ret->mySector->x, ret->mySector->y);
	return ret;
}

void addTrail(entity* from, entity* to, char type){
	if(from->numTrails == from->maxTrails){
		from->trailXs = realloc(from->trailXs, sizeof(int64_t)*(from->maxTrails+=2));
		from->trailYs = realloc(from->trailYs, sizeof(int64_t)*from->maxTrails);
		from->trailTypes = realloc(from->trailTypes, sizeof(int)*from->maxTrails);
	}
	from->trailTypes[from->numTrails] = type;
	from->trailXs[from->numTrails] = displacementX(from, to);
	from->trailYs[from->numTrails++] = displacementY(from, to);
}

void tick(entity* who){
	who->thrustFlag = 0;
	who->numTrails = 0;
	who->energy += who->energyRegen;
	if(who->energy > who->maxEnergy) who->energy = who->maxEnergy;
	who->shield += who->shieldRegen;
	if(who->shield > who->maxShield) who->shield = who->maxShield;
	if(who->targetLock){
		if(who->targetLock->destroyFlag){
			who->targetLock=NULL;
		}else{
			double x = displacementX(who, who->targetLock);
			double y = displacementY(who, who->targetLock);
			if(sqrt(x*x + y*y) > LOCK_RANGE) who->targetLock = NULL;
		}
	}
	(*who->myAi->act)(who);
	who->sinTheta = sin(who->theta * (2*M_PI/16));
	who->cosTheta = cos(who->theta * (2*M_PI/16));
	guarantee *g = who->me;
	double vx = g->vel[0];
	double vy = g->vel[1];
	double v = sqrt(vx*vx + vy*vy);
	if(v <= 1.5*16){
		if (v == 0) return;
		g->vel[0] = 0;
		g->vel[1] = 0;
	}else{
		vx /= v;
		vy /= v;
		g->vel[0] -= (int)(vx*1.5*16);
		g->vel[1] -= (int)(vy*1.5*16);
		//who->x += who->vx;
		//who->y += who->vy;
	}
	guaranteeMoved(g, 0);
}

char tick2(entity* who){
	if(who->shield <= 0) return 2;
	uint64_t secx = who->mySector->x;
	uint64_t secy = who->mySector->y;
	guarantee *g = who->me;
	if(g->pos[0] > POS_MAX){
		secx++;
		g->pos[0] -= (POS_MAX-POS_MIN+1);
	}else if(g->pos[0] < POS_MIN){
		secx--;
		g->pos[0] += (POS_MAX-POS_MIN+1);
	}
	if(g->pos[1] > POS_MAX){
		secy++;
		g->pos[1] -= (POS_MAX-POS_MIN+1);
	}else if(g->pos[1] < POS_MIN){
		secy--;
		g->pos[1] += (POS_MAX-POS_MIN+1);
	}
	if(secx!=who->mySector->x || secy!=who->mySector->y){
		//puts("Someone's left the sector, and things are probably about to break :(");
		sector *new = searchforsector(secx, secy);
		if(new == NULL){
			addmetosector(who, secx, secy);
			return 1;
		}
		guarantee *newG = createEntityGuarantee(getCloseEntGuarantee(new, g->pos[0], g->pos[1]), new, g->r, g->pos, g->vel, who);
		fileMoveRequest(who, who->mySector, new);
		destroyGuarantee(g);
		who->me = newG;
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
	guarantee *g = who->me;
	g->vel[0] += who->thrust*who->cosTheta;
	g->vel[1] += who->thrust*who->sinTheta;
	guaranteeMoved(g, 0);
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
	free(who->trailXs);
	free(who->trailYs);
	free(who->contacts);
	free(who);
}
