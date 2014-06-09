#include <stdio.h>
#include <math.h>
#include "globals.h"

void aiHuman(entity* who){
	char data = *(char*)who->aiFuncData;
	if(data & 0x01) turn(who, -1);
	if(data & 0x02) turn(who, 1);
	if(data & 0x04) thrust(who);
}

void aiMissile(entity* who){
	entity* target = *((entity**)who->aiFuncData);
	if(target == NULL) return;

	thrust(who);
	double unx = -cos(who->theta*(2*M_PI/16));
	double uny = -sin(who->theta*(2*M_PI/16));

	double dy = target->y - who->y;
	double dx = target->x - who->x;

	double y = dx*unx + dy*uny;
	double x = dy*unx - dx*uny;

	double dvx = who->vx - target->vx;
	double dvy = who->vy - target->vy;

	double vy = dvx*unx + dvy*uny;
	double vx = dvy*unx - dvx*uny;

	char spin = 1;
	if(vx == 0){
		if((x>0) ^ (vy>0)) (*who->modules[0]->actFunc)(who, 0, 1, -spin);
		else (*who->modules[0]->actFunc)(who, 0, 1, spin);
		return;
	}
	if(vx<0){
		vx *= -1;
		spin *= -1;
		x *= -1;
	}

	if(x<0){
		(*who->modules[0]->actFunc)(who, 0, 1, spin);
		return;
	}
	double t = x/vx;
	if(vy*t-0.03*t*t < y) (*who->modules[0]->actFunc)(who, 0, 1, -spin);
	else (*who->modules[0]->actFunc)(who, 0, 1, spin);
	return;
}
