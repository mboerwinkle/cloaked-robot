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
		if(runner->me->r < 64*5*16 || runner == who || !(who->lockSettings & (1<<runner->faction))){
			runner = runner->next;
			continue;
		}
		dx = displacementX(who, runner);
		if(abs(dx) > LOCK_RANGE){
			runner = runner->next;
			continue;
		}
		dy = displacementY(who, runner);
		if(abs(dy) > LOCK_RANGE){
			runner = runner->next;
			continue;
		}
		d = sqrt(dx*dx+dy*dy)-runner->me->r;
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
	double vx = who->me->vel[0] - target->me->vel[0] + dy * v / dist;
	double vy = who->me->vel[1] - target->me->vel[1] - dx * v / dist;
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
		(who->modules[i]->actFunc)(who, i, i<3 && (data->keys&(0x20<<i)));
}

static void aiHumanCollision(entity *who, entity *him)
{
	if (who->minerals == 0 || who->faction != him->faction || him->me->r < who->me->r)
		return;
	int i = who->numModules - 1;
	for (; i >= 0; i--) {
		if (who->modules[i] == &miningModule && ((humanAiData*)who->aiFuncData)->keys & (0x20 << i)) {
			him->minerals += who->minerals;
			who->minerals = 0;
			addTrail(who, him, 1);
		}
	}
}

static void aiDroneAct(entity* who){
	double vx = who->me->vel[0];
	double vy = who->me->vel[1];
	double dx, dy;
	droneAiData *data = who->aiFuncData;
	if(data->timer == 200){
		data->timer = 0;
	}
	data->timer++;
	if(data->timer == 1 && who->targetLock == NULL){
		if (data->repeats < 20)
			data->repeats++;
		linkNear(who, 64*6400*16 * data->repeats);
		double bestScore = 64*6400*4*16 * data->repeats;
		data->target = NULL;
		entity* runner = who->mySector->firstentity;
		int64_t d;
		while(runner){
			if(runner->me->r < 64*5*16 || runner == who || runner->faction == 0){
				runner = runner->next;
				continue;
			}
			dx = displacementX(who, runner);
			if(abs(dx) > 64*6400*16 * data->repeats){
				runner = runner->next;
				continue;
			}
			dy = displacementY(who, runner);
			if(abs(dy) > 64*6400*16 * data->repeats){
				runner = runner->next;
				continue;
			}
			d = sqrt(dx*dx+dy*dy);
			if(d > 64*6400*16 * data->repeats){
				runner = runner->next;
				continue;
			}
			double angle = atan2(dy, dx);
			if(angle < 0) angle += 2*M_PI;
			angle = fabs(angle - (M_PI_4/2)*who->theta);
			if(angle>M_PI) angle = 2*M_PI - angle;
			double score = d*(1+angle/M_PI);
			if(!(who->lockSettings & (1<<runner->faction))) score += 64*6400*16 * data->repeats;
			if(score < bestScore){
				bestScore = score;
				data->target = runner;
			}
			runner = runner->next;
		}
		unlinkNear();
		// If we have an enemy target, we don't need to look any farther next time.
		if (data->target && (who->lockSettings & (1<<data->target->faction)))
			data->repeats--;
	}
	if(data->target == NULL){	
		if(vx*vx+vy*vy < 62500*16*16){
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
	double dvx = who->me->vel[0] - data->target->me->vel[0];
	double dvy = who->me->vel[1] - data->target->me->vel[1];
	double unvx = unx*dvx;
	double unvy = uny*dvy;
	if(sqrt(dx * dx + dy * dy) <= LOCK_RANGE && (who->lockSettings & (1<<data->target->faction))) {
		who->targetLock = data->target;
		//So we can drop back to regular scanning range
		data->repeats = 0;
	}
	if(who->targetLock == NULL){
		(who->modules[0]->actFunc)(who, 0, 0);
		//Martin: This is the one place I've changed your drone source. Hopefully it will cut down on jitters.
		circle(who, data->target, data->target->me->r+who->me->r+700*16, 30*16);
		/*if(x+(data->target->me->r+who->me->r+500) > 0){
			turn(who, 1);
		}
		if(x+(data->target->me->r+who->me->r+500) < 0){
			turn(who, -1);
		}*/
	}
	else{		
		//Addendum: Since the 'circle' method handles thrusting, I also moved this if statement into here.
		if(unvx + unvy < 0 || dvy * dvy + dvx * dvx < 62500*16*16){
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

	double dvx = who->me->vel[0] - target->me->vel[0];
	double dvy = who->me->vel[1] - target->me->vel[1];

	double vy = dvx*unx + dvy*uny;
	double vx = dvy*unx - dvx*uny;

	char spin = 1;
	if (vx == 0) {
		if ((x>0) ^ (vy>0)) spin = -1;
	} else {
		if (vx<0) {
			vx *= -1;
			spin = -1;
			x *= -1;
		}

		if (x >= 0) {
			double t = x/vx;
			if (vy*t-who->thrust/2*t*t < y) spin *= -1;
		}
	}
	turn(who, spin);
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
	him->shield -= MISSILE_DMG;
}

static void aiAsteroidAct(entity* who){
	if(*(char*)who->aiFuncData){
		turn(who, *(char*)who->aiFuncData);
	}else thrust(who);
}

static void aiAsteroidCollision(entity* me, entity* him){
	if((him->faction != 0) || *(char*)him->aiFuncData){//if it's not an asteroid; prevents formation of astroid fields. Feel free to delete if you want asteroid fields
		char* aRat = (char*)me->aiFuncData;
		if(*aRat) *aRat *= -1;
		else *aRat = 1-2*(random()%2);
	}
}

static void aiPacerAct(entity* who)
{
	if (who->me->vel[0] < 500*16) thrust(who);
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
		circle(who, who->targetLock, 6400*2*16 /*twice lazor distance*/, 60*16);
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
			if (data->rechecks < 20)
				data->rechecks++;
			linkNear(who, 64*6400*16 * data->rechecks);
			double bestScore = 64*6400*16 * data->rechecks;
			entity *target = NULL;
			entity* runner = who->mySector->firstentity;
			double d;
			int64_t dx, dy;
			while(runner){
				if(1<<runner->faction & who->lockSettings){
					dx = displacementX(who, runner);
					if (abs(dx) >= bestScore) {
						runner = runner->next;
						continue;
					}
					dy = displacementY(who, runner);
					if (abs(dy) >= bestScore) {
						runner = runner->next;
						continue;
					}
					d = sqrt(dx*dx+dy*dy)-runner->me->r;
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
			data->rechecks = (bestScore + 6400*16) / (16*64*6400);
			data->shotsLeft = -1;
			if (bestScore < LOCK_RANGE) {
				data->recheckTime = 1;
				who->targetLock = target;
				destroyerComputeShotsLeft();
				return;
			}
			if (gotoPt(who, displacementX(who, target), displacementY(who, target), who->me->vel[0]-target->me->vel[0], who->me->vel[1]-target->me->vel[1])) {
				data->recheckTime = 1;
			} else {
				if (bestScore < 64*6400*16)
					data->recheckTime = 200*bestScore/(16*64*6400);
				else
					data->recheckTime = 200;
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
				if (sqrt(dx*dx + dy*dy) < miningRange/2*1.1 + who->me->r + who->targetLock->me->r)
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
				circle(who, who->targetLock, miningRange/2 + who->me->r + who->targetLock->me->r, 0);
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
	if (data->phase == 0 && data->home == him) {
		who->shield = 0;
		him->minerals += who->me->r/16*who->me->r/16 + who->minerals;
		//So we don't knock our target about too much
		who->me->r = 1;
		//So my death doesn't cause an asteroid explosion
		who->minerals = 0;
	} else ((minorMinerAiData*)who->aiFuncData)->pleaseTurn = 16;
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
			/*if (data->rechecks < 20)
				data->rechecks++;*/
			linkNear(who, 64*6400 * 20 * 16); // data->rechecks);
			entity* runner = who->mySector->firstentity;
			double bestDist = 64*6400 * 20 * 16; // data->rechecks;
			double r = 640*16 + 1;//must be bigger than a drone
			while(runner){
				if(runner->faction == who->faction && runner->me->r >= r && runner != who){
					int64_t dx = displacementX(who, runner);
					int64_t dy = displacementY(who, runner);
					if (runner->me->r == r && (abs(dx) >= bestDist || abs(dy) >= bestDist)) {
						runner = runner->next;
						continue;
					}
					double dist = sqrt(dx*dx + dy*dy);
					if (runner->me->r > r || dist < bestDist) {
						data->homestation = runner;
						r = runner->me->r;
						bestDist = dist;
					}
				}
				runner = runner->next;
			}
			unlinkNear();
			if (data->homestation) {
				data->recheckTime = 1;
				//data->rechecks = 0;
			}
		}
		(who->modules[0]->actFunc)(who, 0, 0);
		(who->modules[1]->actFunc)(who, 1, 0);
		return;
	}
	if(behavior == -1) {
		if (--(data->recheckTime) == 0) {
			data->recheckTime = 300;
			//if (data->rechecks < 20)
				//data->rechecks++;
			linkNear(who, 64*6400*16 * 20); // data->rechecks);
			double bestScore = 64*6400*16 * 20; // data->rechecks;
			entity *temptarget = NULL;
			entity* runner = who->mySector->firstentity;
			double d;
			int64_t dx, dy;
			while(runner){
				if(runner->faction == 0){
					dx = displacementX(who, runner);
					dy = displacementY(who, runner);
					d = sqrt(dx*dx+dy*dy)-runner->me->r;
					if(d < bestScore){
						bestScore = d;
						temptarget = runner;
					}
				}
				runner = runner->next;
			}
			unlinkNear();
			if ((data->target = temptarget) != NULL) {
				data->recheckTime = 1;
				//data->rechecks = 0;
			} else if (who->minerals != 352*352*2) {
				behavior = 1;
				data->recheckTime = 1;
				//data->rechecks = 0;
			}
		}
	}
	if(who->energy == who->maxEnergy) data->phase = 0;
	if(who->energy < 2) data->phase = 1;
	(who->modules[0]->actFunc)(who, 0, (data->phase == 0 && behavior > 0));
	(who->modules[1]->actFunc)(who, 1, 1);
	if(behavior == 1){
		entity* homestation = data->homestation;
		circle(who, homestation, 16*1000 + who->me->r + homestation->me->r, 0);
		int64_t dx = displacementX(who, homestation);
		int64_t dy = displacementY(who, homestation);
		//The transfer now works in theoretically the opposite direction, if I'm low on minerals and he can give me enough for my 2 base miners.
		if (sqrt(dx*dx + dy*dy) < 2000*16 + who->me->r + homestation->me->r
		    && who->minerals != 352*352*2
		    && homestation->minerals + who->minerals >= 352*352*2){
			homestation->minerals += who->minerals - 352*352*2;
			who->minerals = 352*352*2;
			addTrail(who, homestation, 1);
		}
	}
	if(behavior == 2 && data->target != NULL){
		circle(who, data->target, miningRange/2 + who->me->r + data->target->me->r, 30*16);
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
		*(int*)who->aiFuncData = count;
	}
}

void initAis(){
	aiHuman.loadSector = 1;
	aiHuman.act = aiHumanAct;
	aiHuman.handleCollision = aiHumanCollision;
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
