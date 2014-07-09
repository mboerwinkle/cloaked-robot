#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"
int mkship(char name[MAXNAMELEN]){
	printf("mkship called (%s)\n", name);
	char path[MAXNAMELEN + 6];
	puts("1");
	FILE *fp;
	puts("1");
	sprintf(path, "ships/%s", name);
	puts("1");
	if((fp = fopen(path, "r")) != NULL){
	puts("1");
		fclose(fp);
	puts("1");
		return(-1);
	puts("1");
	}
	puts("1");
	fp = fopen(path, "w");
	puts("1");
	fprintf(fp, "0_0\n7000 7000");
	puts("1");
	fclose(fp);
	puts("1");
	return(0);
}
