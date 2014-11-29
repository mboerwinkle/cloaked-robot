#include "globals.h"
#include <stdio.h>
#include <stdlib.h>

int64_t xsectordisp(int64_t strt, int64_t end){
	strt -= end;
	if(strt > (0.5)*UNIVERSEX) strt -= UNIVERSEX;
	if(-strt > (0.5)*UNIVERSEX) strt += UNIVERSEX;
	return(strt);
}

int64_t ysectordisp(int64_t strt, int64_t end){
	strt -= end;
	if(strt > (0.5)*UNIVERSEY) strt -= UNIVERSEY;
	if(-strt > (0.5)*UNIVERSEY) strt += UNIVERSEY;
	return(strt);
}

uint64_t xcoord(uint64_t coord, char add){
	if(coord < -add){
		coord += UNIVERSEX+add;
	}
	else{
		coord += add;
	}
	while(coord >= UNIVERSEX){
		coord -= UNIVERSEX;
	}
	return(coord);
}

uint64_t ycoord(uint64_t coord, char add){
	if(coord < -add){
		coord += UNIVERSEY+add;
	}
	else{
		coord += add;
	}
	while(coord >= UNIVERSEY){
		coord -= UNIVERSEY;
	}
	return(coord);
}

void move(uint64_t xorigin, uint64_t yorigin, uint64_t xfinal, uint64_t yfinal){
	appear(xfinal, yfinal);
	disappear(xorigin, yorigin);
}
void appear(uint64_t x, uint64_t y){
	printf("appear called. %lld %lld\n", x, y);
	sector *target;
	short counterone, countertwo;
	for(counterone = -1; counterone <= 1; counterone++){
		for(countertwo = -1; countertwo <= 1; countertwo++){
			target = searchforsector(xcoord(x, counterone), ycoord(y,countertwo));
			if(target == NULL){
				loadsector(xcoord(x, counterone), ycoord(y, countertwo));
			}
			else{
				target->number++;
			}
		}
	}
}
void disappear(uint64_t x, uint64_t y){
	printf("disappear called. %lld %lld\n", x, y);
	sector *target;
	short counterone, countertwo;
	for(counterone = -1; counterone <= 1; counterone++){
		for(countertwo = -1; countertwo <= 1; countertwo++){
			target = searchforsector(xcoord(x, counterone), ycoord(y, countertwo));
			if(target == NULL) puts("that's an error...");
			target->number--;
		}
	}
}
