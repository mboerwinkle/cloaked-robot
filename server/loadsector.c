#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"
//35 is the length of twouint64_ts in hex (with possible "-" signs) separated with an underscore
void gensector(uint64_t x, uint64_t y){//only called from loadsector
	printf("gensector called (%ld, %ld)\n", x, y);
	char name[35];
	sprintf(name, "sectors/%lX_%lX", x, y);
	FILE *fp;
	fp = fopen(name, "w");
	//fprintf(fp, "sector\n");
	//other generaty thingys
	fclose(fp);
	fp = NULL;
	printf("done generating (%s)\n", name);
}
void loadsector(uint64_t x, uint64_t y){
	printf("loadsector called (%ld, %ld)\n", x, y);
	sector *new = malloc(sizeof(sector));
	FILE *fp;
	char name[35];
	sprintf(name, "sectors/%lX_%lX", x, y);//makes hexadecimal file name

	if((fp = fopen(name, "r")) == NULL){
		printf("sector %s does not exist, generating\n", name);
		gensector(x, y);
	}

	if(fp != NULL){
		fclose(fp);
		fp = NULL;
	}

	new->nextsector = listrootsector;
	listrootsector = new;

	listrootsector->number = 1;
	listrootsector->firstentity = NULL;
	listrootsector->x = x;
	listrootsector->y = y;

	interpretsector(x, y);
}
