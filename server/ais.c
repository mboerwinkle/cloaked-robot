#include <stdio.h>
#include <math.h>
#include "globals.h"

void aiHuman(entity* who){
	char data = *(char*)who->aiFuncData;
	if(data & 0x01) turn(who, -1);
	if(data & 0x02) turn(who, 1);
	if(data & 0x04) thrust(who);
	(*who->modules[0]->actFunc)(who, 0, data&0x08);
}

void aiMissile(entity* who){
	entity* target = *((entity**)who->aiFuncData);
	if(target == NULL) return;

	thrust(who);
	double unx = -who->cosTheta;
	double uny = -who->sinTheta;

	int32_t dx = displacementX(who, target);
	int32_t dy = displacementY(who, target);

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
