#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"
int mkship(char name[MAXNAMELEN]){
	char path[MAXNAMELEN + 6];
	FILE *fp;
	sprintf(path, "ships/%s", name);
	if((fp = fopen(path, "r")) != NULL){
		fclose(fp);
		return(-1);
	}
	fp = fopen(path, "w");
	printf("%s\n", path);
	fprintf(fp, "0_0\n7000 7000");
	fclose(fp);
	return(0);
}