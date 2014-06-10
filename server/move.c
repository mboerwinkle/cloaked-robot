#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
void move(uint64_t xorigin, uint64_t yorigin, uint64_t xfinal, uint64_t yfinal){
	appear(xfinal, yfinal);
	disappear(xorigin, yorigin);
}
void appear(uint64_t x, uint64_t y){
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
void disappear(uint64_t x, uint64_t y){
	sector *target;
	short counterone, countertwo;
	for(counterone = -1; counterone <= 1; counterone++){
		for(countertwo = -1; countertwo <= 1; countertwo++){
			target = searchforsector((x+counterone), (y+countertwo));
			target->number--;
		}
	}
}
