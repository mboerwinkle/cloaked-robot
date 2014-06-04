#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"
/*struct*/void interpretsector(long long int x, long long int y){
	printf("interpretsector called (%lld, %lld)\n", x, y);
	char name[35];
	sprintf(name, "sectors/%llX_%llX", x, y);
	FILE *fp;
	fp = fopen(name, "r");
	fclose(fp);
	fp = NULL;
}
