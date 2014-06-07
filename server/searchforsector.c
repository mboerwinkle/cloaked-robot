#include "globals.h"
#include <stdlib.h>
sector *searchforsector(long long int x, long long int y){
	sector *conductor;
	conductor = listrootsector;
 	while(conductor != NULL && (conductor->x != x || conductor->y != y)){
		conductor = conductor->nextsector;
	}
	return(conductor);
}
