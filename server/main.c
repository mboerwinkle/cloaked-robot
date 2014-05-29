#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"
int main(){
	listrootsector = malloc(sizeof(sector));
	listrootsector->nextsector = NULL;
	//start network listening thread
	loadsector(1,1);
	loadsector(1,-2);
	loadsector(5,4);
	unloadsector(1,-2);
	unloadsector(5,4);
	unloadsector(1,1);
	
	return(0);
}
