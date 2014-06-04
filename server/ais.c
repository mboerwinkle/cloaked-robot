#include <stdio.h>
#include <math.h>
#include "globals.h"

void aiHuman(entity* who){
	struct aiHumanData* data = (struct aiHumanData*)who->aiFuncData;
	int p = data->player;
	if(p==255) return;
/*	if(myKeys[p][0]) (*who->modules[0]->actFunc)(who, 0, 1);
	if(myKeys[p][1]) (*who->modules[1]->actFunc)(who, 1, 1);
	if(myKeys[p][2]) (*who->modules[2]->actFunc)(who, 2, 1);
	if(myKeys[p][4]) (*who->modules[3]->actFunc)(who, 3, 1);*/
}

void aiMissile(entity* who){
	entity* target = *((entity**)who->aiFuncData);
	if(target == NULL) return;

	thrust(who, who->thrust);
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
