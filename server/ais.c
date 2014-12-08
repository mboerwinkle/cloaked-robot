#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "globals.h"

ai aiMissile;
ai aiHuman;
ai aiDrone;
ai aiAsteroid;
ai aiPacer;
ai aiBullet;
ai aiDestroyer;
ai aiMinorMiner;
ai aiMajorMiner;
ai aiStation;

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
		d = sqrt(dx*dx+dy*dy)-runner->r;
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

//dx and dy are the destination's position relative to me, and
//vx and vy are my velocity relative to the destination.
//Returns whether or not it made a turn, which is good for long-term planning.
static char gotoPt(entity *who, int64_t dx, int64_t dy, double vx, double vy) {
	double desiredDir;
	if (vx == 0 && vy == 0) {
		desiredDir = atan2(dy, dx);
	} else {
		double vel = sqrt(vx*vx + vy*vy);
		double unx = vx / vel;
		double uny = vy / vel;
		double dist = unx * dx + uny * dy;
		double stoppingDist = 0.5 * vel * vel / who->thrust + vel * 2 * who->maxTurn;
		if (dist > stoppingDist) {
			desiredDir = atan2(vy, vx);
		} else {
			desiredDir = atan2(-vy, -vx);
		}
		vy = unx*dy - uny*dx;
		if (fabs(vy) > fabs(dist-stoppingDist)/2) {
			if ((vy < 0) ^ (dist > stoppingDist))
				desiredDir += M_PI/4;
			else
				desiredDir -= M_PI/4;
		}
	}
	desiredDir = desiredDir - who->theta * (2*M_PI/16);
	while (desiredDir < -M_PI) desiredDir += 2*M_PI;
	while (desiredDir > M_PI) desiredDir -= 2*M_PI;

	if (fabs(desiredDir) > (2*M_PI/32)*1.2) {
		turn(who, desiredDir > 0 ? 1 : -1);
		if (fabs(desiredDir) <= M_PI/2)
			thrust(who);
		return desiredDir > 0 ? 1 : -1;
	}
	thrust(who);
	return 0;
}

//The radius and velocity which this function takes aren't exact (especially for high v), but they give a sense of scale.
static char circle(entity *who, entity *target, double r, double v)
{
	int64_t dx = displacementX(who, target);
	int64_t dy = displacementY(who, target);
	double dist = sqrt(dx*dx + dy*dy);
	double vx = who->vx - target->vx + dy * v / dist;
	double vy = who->vy - target->vy - dx * v / dist;
	double scale = 1 - (r / dist);
	return gotoPt(who, dx * scale, dy * scale, vx, vy);
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
	int i = 0;
	for (; i < who->numModules; i++)
		(who->modules[i]->actFunc)(who, i, data->keys&(0x20<<i));
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
			if(runner->r < 64*5 || runner == who || runner->faction == 0){
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
	if(data->target->destroyFlag){
		data->target = NULL;
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
	if(who->targetLock == NULL){
		(who->modules[0]->actFunc)(who, 0, 0);
		//Martin: This is the one place I've changed your drone source. Hopefully it will cut down on jitters.
		circle(who, data->target, data->target->r+who->r+700, 30);
		/*if(x+(data->target->r+who->r+500) > 0){
			turn(who, 1);
		}
		if(x+(data->target->r+who->r+500) < 0){
			turn(who, -1);
		}*/
	}
	else{		
		//Addendum: Since the 'circle' method handles thrusting, I also moved this if statement into here.
		if(unvx + unvy < 0 || dvy * dvy + dvx *dvx < 62500){
			thrust(who);
		}
		(who->modules[0]->actFunc)(who, 0, 1);
		double y = dy*uny + dx*unx;
		if((x!=0 && y/fabs(x)<5) || y<0){
			turn(who, x>0?1:-1);
		}
	}
}

static void noCareCollision(entity* me, entity* him){}

static char crazyPursuit(entity *who, entity *target)
{
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
		if((x>0) ^ (vy>0)) spin = -1;
	}else{
		if(vx<0){
			vx *= -1;
			spin = -1;
			x *= -1;
		}

		if(x >= 0){
			double t = x/vx;
			if(vy*t-who->thrust/2*t*t < y) spin *= -1;
		}
	}
	turn(who, spin);
	if(dvx == 0 && dvy == 0) return spin;
	double dv = sqrt(dvx*dvx + dvy*dvy);
	unx = dvx/dv;
	uny = dvy/dv;
	y = dx*unx + dy*uny;
	x = dy*unx - dx*uny;
	int64_t r = who->r + target->r;
	if(r <= fabs(x)) return spin;
	double var = sqrt(r*r - x*x);
	y -= var;
	if(y >= dv || y < 0) return spin;
	who->x += dx+target->vx-who->vx - var/2*unx;
	who->y += dy+target->vy-who->vy - var/2*uny;
	return spin;
}

static void aiMissileAct(entity* who){
	uint16_t* ttl = who->aiFuncData;
	if(++(*ttl) == 40*6) who->shield = 0;
	if (who->targetLock == NULL)
		return;
	crazyPursuit(who, who->targetLock);
}

#define MISSILE_DMG 40

static void aiMissileCollision(entity* me, entity* him){
	if(him != me->targetLock) return;
	me->shield = 0;
	him->shield-=MISSILE_DMG;
}

static void aiAsteroidAct(entity* who){
	if(*(char*)who->aiFuncData){
		turn(who, *(char*)who->aiFuncData);
	}else thrust(who);
}

static void aiAsteroidCollision(entity* me, entity* him){
	if((him->type != 4 && him->type != 5) || *(char*)him->aiFuncData){//if it's not an asteroid; prevents formation of astroid fields. Feel free to delete if you want asteroid fields
		char* aRat = (char*)me->aiFuncData;
		if(*aRat) *aRat *= -1;
		else *aRat = 1-2*(random()%2);
	}
}

static void aiPacerAct(entity* who)
{
	if (who->vx < 500) thrust(who);
}

static void aiBulletAct(entity *who)
{
	if(++(*(uint16_t*)who->aiFuncData) == 40*6) who->shield = 0;
	thrust(who);
}

static void aiBulletCollision(entity *me, entity *him)
{
	me->shield = 0;
	if (me->lockSettings & 1 << him->faction)
		him->shield -= 20;
}

static void aiDestroyerAct(entity *who)
{
#define destroyerComputeShotsLeft() data->shotsLeft = who->targetLock->shield / MISSILE_DMG + 1;
	destroyerAiData *data = (destroyerAiData *)who->aiFuncData;
	if (who->targetLock) {
		circle(who, who->targetLock, 6400*2 /*twice lazor distance*/, 60);
		int e = who->energy;
		(who->modules[0]->actFunc)(who, 0, 1);
		(who->modules[1]->actFunc)(who, 1, 1);
		data->shotsLeft -= (e-who->energy) / MISSILE_E_COST;
		(who->modules[2]->actFunc)(who, 2, 1);
		if (data->shotsLeft <= 0) {
			lock(who);
			if (who->targetLock != NULL)
				destroyerComputeShotsLeft();
		}
	} else {
		(who->modules[0]->actFunc)(who, 0, 0);
		(who->modules[1]->actFunc)(who, 1, 0);
		(who->modules[2]->actFunc)(who, 2, 0);
		if (--(data->recheckTime) == 0) {
			linkNear(who, 64*6400);
			double bestScore = 64*6400;
			entity *target = NULL;
			entity* runner = who->mySector->firstentity;
			double d;
			int64_t dx, dy;
			while(runner){
				if(1<<runner->faction & who->lockSettings){
					dx = displacementX(who, runner);
					dy = displacementY(who, runner);
					d = sqrt(dx*dx+dy*dy)-runner->r;
					if(d < bestScore){
						bestScore = d;
						target = runner;
					}
				}
				runner = runner->next;
			}
			unlinkNear();
			if (target == NULL) {
				data->recheckTime = 200;
				data->shotsLeft = -100;
				return;
			}
			if (bestScore < LOCK_RANGE) {
				data->recheckTime = 1;
				who->targetLock = target;
				destroyerComputeShotsLeft();
				return;
			}
			if (gotoPt(who, displacementX(who, target), displacementY(who, target), who->vx-target->vx, who->vy-target->vy)) {
				data->recheckTime = 1;
			} else {
				data->recheckTime = 200*bestScore/(64*6400);
			}
		} else {
			if (data->shotsLeft != -100)
				thrust(who);
		}
	}
}

static void aiMinorMinerAct(entity *who)
{
	minorMinerAiData *data = (minorMinerAiData*)who->aiFuncData;
	if (data->home == NULL || data->home->destroyFlag) {
		who->shield = 0;
		return;
	}
	if (data->phase) {
		if (who->energy < 2 || who->targetLock == NULL) {
			data->phase = 0;
		} else {
			if (data->phase == 1) {
				int64_t dx = displacementX(who, who->targetLock);
				int64_t dy = displacementY(who, who->targetLock);
				if (sqrt(dx*dx + dy*dy) < miningRange/2*1.1 + who->r + who->targetLock->r)
					data->phase = 2;
			}
			if (data->phase == 2) {
				(who->modules[0]->actFunc)(who, 0, 1);
			}
			if (data->pleaseTurn && data->phase != 2) {
				turn(who, 1);
				thrust(who);
			} else {
				//Circle w/ a velocity of zero actually just pulls it alongside.
				circle(who, who->targetLock, miningRange/2 + who->r + who->targetLock->r, 0);
			}
			if (data->pleaseTurn)
				data->pleaseTurn--;
			return;
		}
	}
	if (data->pleaseTurn) {
		turn(who, 1);
		thrust(who);
		data->pleaseTurn--;
	} else {
		crazyPursuit(who, data->home);
	}
}

static void aiMinorMinerCollision(entity *who, entity *him)
{
	minorMinerAiData *data = (minorMinerAiData*)who->aiFuncData;
	if (/*data->phase == 0 && */data->home == him) {
		who->shield = 0;
		him->minerals += who->r*who->r + who->minerals;
		//So we don't knock our target about too much
		who->r = 1;
		//So my death doesn't cause an asteroid explosion
		who->minerals = -1;
	} else ((minorMinerAiData*)who->aiFuncData)->pleaseTurn = 4;
}

static void aiMajorMinerAct(entity *who){
	majorMinerAiData *data = (majorMinerAiData*)who->aiFuncData;
	char behavior = 2;
	if (data->target == NULL) behavior = -1;
	else if (data->target->destroyFlag) {
		data->target = NULL;
		behavior = -1;
	}
	//Note that the addtional 352*352*2 base minerals cancels with the below line, so that miners using the lazor will behave normally aside from carrying 352*352*2 extra wealth at all times. For the miners who use miners, it acts as a reserve to launch 2 at the beginning.
	if (who->minerals >= 1000000 + 352*352*2 || who->shield != who->maxShield) behavior = 1;
	if(data->homestation == NULL) behavior = 0;
	else if(data->homestation->destroyFlag){
		data->homestation = NULL;
		behavior = 0;
	}	
	if(behavior == 0){
		if (--(data->recheckTime) == 0) {
			data->recheckTime = 300;
			linkNear(who, 64*6400);
			entity* runner = who->mySector->firstentity;
			double r = 641;//must be bigger than a drone
			while(runner){
				if(runner->faction == who->faction && runner->r > r && runner != who){
					data->homestation = runner;
					r = runner->r;
				}
				runner = runner->next;
			}
			unlinkNear();
			if (data->homestation)
				data->recheckTime = 1;
		}
		return;
	}
	if(behavior == -1) {
		if (--(data->recheckTime) == 0) {
			data->recheckTime = 300;
			linkNear(who, 64*6400);
			double bestScore = 64*6400;
			entity *temptarget = NULL;
			entity* runner = who->mySector->firstentity;
			double d;
			int64_t dx, dy;
			while(runner){
				if(runner->faction == 0){
					dx = displacementX(who, runner);
					dy = displacementY(who, runner);
					d = sqrt(dx*dx+dy*dy)-runner->r;
					if(d < bestScore){
						bestScore = d;
						temptarget = runner;
					}
				}
				runner = runner->next;
			}
			unlinkNear();
			if ((data->target = temptarget) != NULL)
				data->recheckTime = 1;
			else if (who->minerals) {
				behavior = 1;
				data->recheckTime = 1;
			}
		}
	}
	if(who->energy == who->maxEnergy) data->phase = 0;
	if(who->energy < 2) data->phase = 1;
	if(data->phase == 0){
		(who->modules[0]->actFunc)(who, 0, 1);
	}
	if(behavior == 1){
		entity* homestation = data->homestation;
		circle(who, homestation, 1000 + who->r + homestation->r, 0);
		int64_t dx = displacementX(who, homestation);
		int64_t dy = displacementY(who, homestation);
		//The transfer now works in theoretically the opposite direction, if I'm wounded and he can give me enough for my 2 base miners.
		if (sqrt(dx*dx + dy*dy) < 2000 + who->r + homestation->r
		    && who->minerals != 352*352*2
		    && homestation->minerals + who->minerals >= 352*352*2){
			homestation->minerals += who->minerals - 352*352*2;
			who->minerals = 352*352*2;
			addTrail(who, homestation, 1);
		}
	}
	if(behavior == 2 && data->target != NULL){
		circle(who, data->target, miningRange/2 + who->r + data->target->r, 20);
	}
}

static void aiStationAct(entity *who)
{
	int count = *(int*)who->aiFuncData;
	turn(who, 1);
	int e = who->energy;
	int i = 0;
	for (; i < who->numModules; i++) {
		(who->modules[i]->actFunc)(who, i, i==count);
	}
	if (e != who->energy) {
		count++;
		if (count == who->numModules)
			count = 0;
	}
	*(int*)who->aiFuncData = count;
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
	aiAsteroid.loadSector = 0;
	aiAsteroid.act = aiAsteroidAct;
	aiAsteroid.handleCollision = aiAsteroidCollision;
	aiPacer.loadSector = 0;
	aiPacer.act = aiPacerAct;
	aiPacer.handleCollision = noCareCollision;
	aiBullet.loadSector = 0;
	aiBullet.act = aiBulletAct;
	aiBullet.handleCollision = aiBulletCollision;
	aiDestroyer.loadSector = 0;
	aiDestroyer.act = aiDestroyerAct;
	aiDestroyer.handleCollision = noCareCollision;
	aiMinorMiner.loadSector = 0;
	aiMinorMiner.act = aiMinorMinerAct;
	aiMinorMiner.handleCollision = aiMinorMinerCollision;
	aiMajorMiner.loadSector = 0;
	aiMajorMiner.act = aiMajorMinerAct;
	aiMajorMiner.handleCollision = noCareCollision;
	aiStation.loadSector = 1;
	aiStation.act = aiStationAct;
	aiStation.handleCollision = noCareCollision;
}
