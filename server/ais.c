#include <stdio.h>
#include <math.h>
#include "globals.h"

ai aiMissile;
ai aiHuman;

static void aiHumanAct(entity* who){
	char data = *(char*)who->aiFuncData;
	if(data & 0x10){
		linkNear(who, 64*300);
		double bestScore = 64*300*2;
		who->targetLock = NULL;
		entity* runner = who->mySector->firstentity;
		int64_t dx, dy;
		int64_t d;
		while(runner){
			if(runner == who){
				runner = runner->next;
				continue;
			}
			dx = displacementX(who, runner);
			dy = displacementY(who, runner);
			d = dx*dx+dy*dy;
			if(d > 64*64*300*300){
				runner = runner->next;
				continue;
			}
			double angle = atan2(dy, dx);
			if(angle < 0) angle += 2*M_PI;
			angle = fabs(angle - (M_PI_4/2)*who->theta);
			if(angle>M_PI) angle = 2*M_PI - angle;
			double score = sqrt(d)*(1+angle/M_PI);
			if(score < bestScore){
				bestScore = score;
				who->targetLock = runner;
			}
			runner = runner->next;
		}
		unlinkNear();
	}
	if(data & 0x20){
		who->targetLock = NULL;
	}
	if(data & 0x01) turn(who, -1);
	if(data & 0x02) turn(who, 1);
	if(data & 0x04) thrust(who);
	(*who->modules[0]->actFunc)(who, 0, data&0x08);
}

static void noCareCollision(entity* me, entity* him){}

static void aiMissileAct(entity* who){
	entity* target = who->targetLock;
	if(target == NULL) return;
	thrust(who);

	int64_t dx = displacementX(who, target);
	int64_t dy = displacementY(who, target);

	double unx = -who->cosTheta;
	double uny = -who->sinTheta;

	double y = dx*unx + dy*uny;
	double x = dy*unx - dx*uny;

	double dvx = who->vx - target->vx;
	double dvy = who->vy - target->vy;

	double vy = dvx*unx + dvy*uny;
	double vx = dvy*unx - dvx*uny;

	char spin = 1;
	if(vx == 0){
		if((x>0) ^ (vy>0)) turn(who, -spin);
		else turn(who, spin);
		return;
	}
	if(vx<0){
		vx *= -1;
		spin *= -1;
		x *= -1;
	}

	if(x<0){
		turn(who, spin);
		return;
	}
	double t = x/vx;
	if(vy*t-who->thrust/2*t*t < y) turn(who, -spin);
	else turn(who, spin);
	return;
}

static void aiMissileCollision(entity* me, entity* him){
	if(him != me->targetLock) return;
	me->shield = 0;
	him->shield-=10;
}

void initAis(){
	aiHuman.act = aiHumanAct;
	aiHuman.handleCollision = noCareCollision;
	aiMissile.act = aiMissileAct;
	aiMissile.handleCollision = aiMissileCollision;
}
