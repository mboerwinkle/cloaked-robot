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

#define MISSILESPIN 0.01
void aiMissile(entity* who){
	entity* target = *((entity**)who->aiFuncData);
	if(target == NULL) return;

	thrust(who, .06);
	double expectedFinalAngle;
	if(who->omega > 0) expectedFinalAngle = who->theta + 0.5*who->omega*who->omega/MISSILESPIN*1.2;
	else  expectedFinalAngle = who->theta - 0.5*who->omega*who->omega/MISSILESPIN*1.2;
	double unx = -cos(expectedFinalAngle);
	double uny = -sin(expectedFinalAngle);

	double dy = target->y - who->y;
	double dx = target->x - who->x;

	double y = dx*unx + dy*uny;
	double x = dy*unx - dx*uny;

	double dvx = who->vx - target->vx;
	double dvy = who->vy - target->vy;

	double vy = dvx*unx + dvy*uny;
	double vx = dvy*unx - dvx*uny;

	double spin = MISSILESPIN;
	if(vx == 0){
		if((x>0) ^ (vy>0)) who->omega -= spin;
		else who->omega += spin;
		return;
	}
	if(vx<0){
		vx *= -1;
		spin *= -1;
		x *= -1;
	}

	if(x<0){
		who->omega += spin;
		return;
	}
	double t = x/vx;
	if(vy*t-0.03*t*t < y) who->omega -= spin;
	else who->omega += spin;
	return;
}
