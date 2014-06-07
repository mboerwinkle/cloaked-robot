#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
void move(long long int xorigin, long long int yorigin, long long int xfinal, long long int yfinal){
	appear(xfinal, yfinal);
	disappear(xorigin, yorigin);
}
void appear(long long int x, long long int y){
	sector *target;
	short counterone, countertwo;
	for(counterone = -1; counterone <= 1; counterone++){
		for(countertwo = -1; countertwo <= 1; countertwo++){
			target = searchforsector((x+counterone), (y+countertwo));
			if(target == NULL){
				loadsector((x+counterone),(y+countertwo));
			}
			else{
				target->number++;
			}
		}
	}
}
void disappear(long long int x, long long int y){
	sector *target;
	short counterone, countertwo;
	for(counterone = -1; counterone <= 1; counterone++){
		for(countertwo = -1; countertwo <= 1; countertwo++){
			target = searchforsector((x+counterone), (y+countertwo));
			target->number--;
		}
	}
}
