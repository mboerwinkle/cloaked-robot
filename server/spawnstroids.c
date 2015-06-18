#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "globals.h"

void spawnstroids(sector *target){
	char combien = 1;
	double theta;
	for(; combien > 0; combien--){
		int32_t randomnum = random();
		int32_t vx = ((unsigned char*)&randomnum)[0]*16;
		int32_t vy = ((unsigned char*)&randomnum)[1]*16;
		theta = ((((unsigned char*)(&randomnum))[2]%32)/2 + 0.5);
		if(theta > 8) vy *= -1;
		if(theta > 4 && theta < 12.5) vx *= -1;
		entity* stroidnewEntity = newEntity(getCloseEntGuarantee(target, 0, 0), 12, 3, 0, target, 0, 0, vx, vy);
		stroidnewEntity->theta = (char)theta;
		*(char*)stroidnewEntity->aiFuncData = 0;
	}
}

