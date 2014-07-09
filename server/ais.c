#include <stdio.h>
#include <math.h>
#include "globals.h"

ai aiMissile;
ai aiHuman;
ai aiDrone;

static void lock(entity* who){
	linkNear(who, LOCK_RANGE);
	double bestScore = LOCK_RANGE*2;
	who->targetLock = NULL;
	entity* runner = who->mySector->firstentity;
	double dx, dy;
	int64_t d;
	while(runner){
		if(runner == who){
			runner = runner->next;
			continue;
		}
		dx = displacementX(who, runner);
		dy = displacementY(who, runner);
		d = sqrt(dx*dx+dy*dy);
		if(d > LOCK_RANGE){
			runner = runner->next;
			continue;
		}
		double angle = atan2(dy, dx);
		if(angle < 0) angle += 2*M_PI;
		angle = fabs(angle - (M_PI_4/2)*who->theta);
		if(angle>M_PI) angle = 2*M_PI - angle;
		double score = d*(1+angle/M_PI);
		if(score < bestScore){
			bestScore = score;
			who->targetLock = runner;
		}
		runner = runner->next;
	}
	unlinkNear();
}

static void aiHumanAct(entity* who){
	char data = *(char*)who->aiFuncData;
	if(data & 0x10){
		lock(who);
	}
	if(data & 0x20){
		who->targetLock = NULL;
	}
	if(data & 0x01) turn(who, -1);
	if(data & 0x02) turn(who, 1);
	if(data & 0x04) thrust(who);
	(*who->modules[0]->actFunc)(who, 0, data&0x08);
}

static void aiDroneAct(entity* who){
	double vx = who->vx;
	double vy = who->vy;
	droneAiData *data = who->aiFuncData;
	if(data->timer == 200){
		data->timer = 0;
	}
	data->timer++;
	if(vx*vx+vy*vy < 22500){
		thrust(who);
	}
	if(data->timer == 1 && who->targetLock == NULL){
		lock(who);
	}
	entity* target = who->targetLock;
	if(target == NULL){		
		(*who->modules[0]->actFunc)(who, 0, 0);
		return;
	}
	(*who->modules[0]->actFunc)(who, 0, 1);
	int64_t dx = displacementX(who, target);
	int64_t dy = displacementY(who, target);
	double unx = who->cosTheta;
	double uny = who->sinTheta;
	double x = dy*unx - dx*uny;
	double y = dx*unx + dy*uny;

	if(data->timer == 1){
		if(dx*dx+dy*dy < ((int64_t)LOCK_RANGE/3)*(LOCK_RANGE/3)){
			data->Attack = 1;
			puts("Attack!");
		}
		if(dx*dx+dy*dy > (int64_t)LOCK_RANGE*LOCK_RANGE){
			data->Attack = 0;
			puts("Defend Teh Base!");
		}
	}

	if(data->Attack == 0){
		if(x > 0){
			turn(who, 1);
		} 
		if(x < 0){
			turn(who, -1);
		}
	}
	else{
		if(y == 0) y++;
		if(x+(8000/y) > 0){
			turn(who, 1);
		}
		if(x+(8000/y) < 0){
			turn(who, -1);
		}
	}
	
}

static void noCareCollision(entity* me, entity* him){}

static void aiMissileAct(entity* who){
	uint16_t* ttl = who->aiFuncData;
	if(++(*ttl) == 40*6) who->shield = 0;
	entity* target = who->targetLock;
	if(target == NULL) return;
	thrust(who);

	int64_t dx = displacementX(who, target);
	int64_t dy = displacementY(who, target);
	if(target->actedFlag == globalActedFlag){
		dx -= target->vx;
		dy -= target->vy;
	}

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
	
	dvx = who->vx - target->vx;
	dvy = who->vy - target->vy;
	if(dvx == 0 && dvy == 0) return;
	double dv = sqrt(dvx*dvx + dvy*dvy);
	unx = dvx/dv;
	uny = dvy/dv;
	y = dx*unx + dy*uny;
	x = dy*unx - dx*uny;
	int64_t r = who->r + target->r;
	if(r <= fabs(x)) return;
	double var = sqrt(r*r - x*x);
	y -= var;
	if(y >= dv || y < 0) return;
	who->x += dx+target->vx-who->vx - var/2*unx;
	who->y += dy+target->vy-who->vy - var/2*uny;
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
	aiDrone.act = aiDroneAct;
	aiDrone.handleCollision = noCareCollision;
}
