#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "globals.h"

void spawnstroids(sector *target){
	char combien = 1;
	double theta;
	for(; combien > 0; combien--){
		entity* stroidnewEntity = newEntity(4, 3, 0, target, 0, 0);
		*(char*)stroidnewEntity->aiFuncData = 0;
		long long int *randomnum = malloc(sizeof(long long int));
		*randomnum = random();
		stroidnewEntity->vx = (double)((unsigned char*)(randomnum))[0];
		stroidnewEntity->vy = (double)((unsigned char*)(randomnum))[1];
		theta = ((((unsigned char*)(randomnum))[2]%32)/2 + 0.5);
		stroidnewEntity->theta = (char)theta;
		if(theta > 8) stroidnewEntity->vy *= -1;
		if(theta > 4 && theta < 12.5) stroidnewEntity->vx *= -1;
	}
}

