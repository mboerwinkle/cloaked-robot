#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"
/*struct*/void interpretsector(uint64_t x, uint64_t y){
	char name[35];
	sprintf(name, "sectors/%lX_%lX", x, y);
	FILE *fp;
	fp = fopen(name, "r");
	fclose(fp);
	fp = NULL;
}
