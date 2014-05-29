#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"
/*struct*/void interpretsector(long long int x, long long int y){
	char name[35];
	sprintf(name, "sectors/%llX_%llX", x, y);
	printf("interpretsector called (%s)\n", name);
	FILE *fp;
	fp = fopen(name, "r");
	fclose(fp);
	fp = NULL;
}
