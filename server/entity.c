#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "globals.h"

entity* newEntity(int type, long int x, long int y){
	entity* ret = malloc(sizeof(entity));
	ret->x = x;
	ret->y = y;
	ret->vx = ret->vy = ret->theta = 0;
	ret->sinTheta = 0;
	ret->cosTheta = 1;
	if(type == 0){
		ret->aiFunc = aiHuman;
		ret->aiFuncData = malloc(sizeof(struct aiHumanData));
		((struct aiHumanData*)ret->aiFuncData)->player = -1;
		ret->r = 2000;
		ret->numModules = 4;
		ret->modules = calloc(4, sizeof(void *));
		ret->moduleDatas = calloc(4, sizeof(void*));
		ret->thrust = 6;
		ret->maxTurn = 9;
		(*missileModule.initFunc)(ret, 3, 1);
	}else if(type == 1){
		ret->aiFunc = aiMissile;
		ret->aiFuncData = malloc(sizeof(int*));
		ret->r = 5;
		ret->numModules = 0;
		ret->modules = NULL;
		ret->moduleDatas = NULL;
		ret->thrust = 7;
		ret->maxTurn = 12;
	}
	return ret;
}

void tick(entity* who){
	if(who->aiFunc) (*who->aiFunc)(who);
	if(who->vx!=0 || who->vy!=0){
		double vx = who->vx;
		double vy = who->vy;
		double v = sqrt(vx*vx + vy*vy);
		vx /= v;
		vy /= v;
		entity *otherGuy = mySector.firstentity, *collision = NULL;
		double minDist = v;

		while(otherGuy){
			if(otherGuy == who){
				otherGuy = otherGuy->next;
				continue;
			}
			int dx = otherGuy->x - who->x;
			int dy = otherGuy->y - who->y;
			double offset = fabs(dx*vy - dy*vx);
			double r = who->r + otherGuy->r;
			if(offset >= r) continue;
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
		if(collision){
			double dx = collision->x - who->x;
			double dy = collision->y - who->y;
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
