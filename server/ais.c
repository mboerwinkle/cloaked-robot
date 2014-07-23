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
		if(runner->r < 64*5 || runner == who || !(who->lockSettings & (1<<runner->faction))){
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
	humanAiData *data = who->aiFuncData;
	if(data->keys & 0x08){
		lock(who);
	}
	if(data->keys & 0x10){
		who->targetLock = NULL;
	}
	if(data->keys & 0x01) turn(who, -1);
	if(data->keys & 0x02) turn(who, 1);
	if(data->keys & 0x04) thrust(who);
	(*who->modules[0]->actFunc)(who, 0, data->keys&0x20);
	(*who->modules[1]->actFunc)(who, 1, data->keys&0x40);
}

static void aiDroneAct(entity* who){
	double vx = who->vx;
	double vy = who->vy;
	double dx, dy;
	droneAiData *data = who->aiFuncData;
	if(data->timer == 200){
		data->timer = 0;
	}
	data->timer++;
	if(data->timer == 1 && who->targetLock == NULL){
		linkNear(who, 64*6400);
		double bestScore = 64*6400*4;
		data->target = NULL;
		entity* runner = who->mySector->firstentity;
		int64_t d;
		while(runner){
			if(runner->r < 64*5 || runner == who){
				runner = runner->next;
				continue;
			}
			dx = displacementX(who, runner);
			dy = displacementY(who, runner);
			d = sqrt(dx*dx+dy*dy);
			if(d > 64*6400){
				runner = runner->next;
				continue;
			}
			double angle = atan2(dy, dx);
			if(angle < 0) angle += 2*M_PI;
			angle = fabs(angle - (M_PI_4/2)*who->theta);
			if(angle>M_PI) angle = 2*M_PI - angle;
			double score = d*(1+angle/M_PI);
			if(!(who->lockSettings & (1<<runner->faction))) score += 64*6400;
			if(score < bestScore){
				bestScore = score;
				data->target = runner;
			}
			runner = runner->next;
		}
		unlinkNear();
	}
	if(data->target == NULL){	
		if(vx*vx+vy*vy < 62500){
			thrust(who);
		}
		return;
	}
	dx = displacementX(who, data->target);
	dy = displacementY(who, data->target);
	double unx = who->cosTheta;
	double uny = who->sinTheta;
	double x = dy*unx - dx*uny;
	double dvx = who->vx - data->target->vx;
	double dvy = who->vy - data->target->vy;	
	double unvx = unx*dvx;
	double unvy = uny*dvy;
	if(sqrt(dx * dx + dy * dy) <= LOCK_RANGE && (who->lockSettings & (1<<data->target->faction))) who->targetLock = data->target;
	if(unvx + unvy < 0 || dvy * dvy + dvx *dvx < 62500){
		thrust(who);
	}
	if(who->targetLock == NULL){
		(*who->modules[0]->actFunc)(who, 0, 0);
		if(x+(data->target->r+who->r+500) > 0){
			turn(who, 1);
		}
		if(x+(data->target->r+who->r+500) < 0){
			turn(who, -1);
		}
	}
	else{		
		(*who->modules[0]->actFunc)(who, 0, 1);
		double y = dy*uny + dx*unx;
		if((x!=0 && y/x<5) || y<0){
			turn(who, x>0?1:-1);
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
	}else{
		if(vx<0){
			vx *= -1;
			spin *= -1;
			x *= -1;
		}

		if(x<0){
			turn(who, spin);
		}else{
			double t = x/vx;
			if(vy*t-who->thrust/2*t*t < y) turn(who, -spin);
			else turn(who, spin);
		}
	}
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
	him->shield-=40;
}

void initAis(){
	aiHuman.loadSector = 1;
	aiHuman.act = aiHumanAct;
	aiHuman.handleCollision = noCareCollision;
	aiMissile.loadSector = 0;
	aiMissile.act = aiMissileAct;
	aiMissile.handleCollision = aiMissileCollision;
	aiDrone.loadSector = 0;
	aiDrone.act = aiDroneAct;
	aiDrone.handleCollision = noCareCollision;
}
