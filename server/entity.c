#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include "globals.h"

entity* newEntity(int type, sector *where, int32_t x, int32_t y){
	if(where == NULL) return NULL;
	entity* ret = malloc(sizeof(entity));
	ret->type = type;
	ret->x = x + 5000 + POS_MIN;
	ret->y = y + 5000 + POS_MIN;
	ret->vx = ret->vy = ret->theta = 0;
	ret->sinTheta = 0;
	ret->cosTheta = 1;
	ret->turn = 0;
	ret->next = where->firstentity;
	where->firstentity = ret;
	ret->mySector = where;
	if(type == 0){
		ret->aiFunc = aiHuman;
		ret->aiFuncData = malloc(1);
		*(char*)ret->aiFuncData = 0;
		ret->r = 640;
		ret->numModules = 4;
		ret->modules = calloc(4, sizeof(void *));
		ret->moduleDatas = calloc(4, sizeof(void*));
		ret->thrust = 3;
		ret->maxTurn = 6;
		ret->shield = ret->maxShield = 100;
		ret->energy = ret->maxEnergy = 100;
		ret->energyRegen = 1;
		(*missileModule.initFunc)(ret, 3, 1);
	}else if(type == 1){
		ret->aiFunc = aiMissile;
		ret->aiFuncData = malloc(sizeof(int*));
		ret->r = 5;
		ret->numModules = 0;
		ret->modules = NULL;
		ret->moduleDatas = NULL;
		ret->thrust = 5;
		ret->maxTurn = 7;
	}
	return ret;
}

char tick(entity* who){
	who->energy += who->energyRegen;
	if(who->energy > who->maxEnergy) who->energy = who->maxEnergy;
	if(who->aiFunc) (*who->aiFunc)(who);
	double vx = who->vx;
	double vy = who->vy;
	double v = sqrt(vx*vx + vy*vy);
	if(v <= 0.5){
		who->vx = 0;
		who->vy = 0;
	}else{
		vx /= v;
		vy /= v;
		double minDist = v-0.5;
		who->vx -= vx*0.5;
		who->vy -= vy*0.5;
		entity *otherGuy = who->mySector->firstentity, *collision = NULL;
		int64_t dx, dy;

		while(otherGuy){
			if(otherGuy == who){
				otherGuy = otherGuy->next;
				continue;
			}
			dx = displacementX(who, otherGuy);
			dy = displacementY(who, otherGuy);
			double offset = fabs(dx*vy - dy*vx);
			double r = who->r + otherGuy->r;
			if(offset >= r || isnan(offset)){
				otherGuy = otherGuy->next;
				continue;
			}
			double dist = dx*vx + dy*vy - sqrt(r*r-offset*offset);
			if(dist<0 || dist >= minDist){
				otherGuy = otherGuy->next;
				continue;
			}
			minDist = dist;
			collision = otherGuy;
			otherGuy = otherGuy->next;
		}
		who->x += trunc(vx*minDist);
		who->y += trunc(vy*minDist);
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
		if(collision){
			double dx = displacementX(who, collision);
			double dy = displacementY(who, collision);
			double d = sqrt(dx*dx+dy*dy);
			dx/=d;
			dy/=d;
			double dvel = (who->vx-collision->vx)*dx+(who->vy-collision->vy)*dy;
			if(dvel > 0){
				double m1 = who->r;// * who->r;
				double m2 = collision->r;// * collision->r;
				dvel *= 2*m1/(m1+m2);
				collision->vx += dvel*dx;
				collision->vy += dvel*dy;
				dvel *= -m2/m1;
				who->vx += dvel*dx;
				who->vy += dvel*dy;
			}
		}
	}
	who->sinTheta = sin(who->theta * (2*M_PI/16));
	who->cosTheta = cos(who->theta * (2*M_PI/16));
	return 0;
}

/*void drawEntity(entity* who, double cx, double cy, double zoom){
	setColorWhite();
	double x = (who->x - cx)*zoom;
	double y = (who->y - cy)*zoom;
	double r = who->r*zoom;
	drawCircle(x, y, r);
	drawLine(x+r*who->cosTheta, y+r*who->sinTheta, x-r*who->sinTheta, y+r*who->cosTheta);
	drawLine(x+r*who->cosTheta, y+r*who->sinTheta, x+r*who->sinTheta, y-r*who->cosTheta);
}*/

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
