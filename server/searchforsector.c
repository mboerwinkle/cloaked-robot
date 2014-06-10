#include "globals.h"
#include <stdlib.h>
sector *searchforsector(uint64_t x, uint64_t y){
	sector *conductor;
	conductor = listrootsector;
 	while(conductor != NULL && (conductor->x != x || conductor->y != y)){
		conductor = conductor->nextsector;
	}
	return(conductor);
}
