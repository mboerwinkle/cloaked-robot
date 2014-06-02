#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"
//35 is the length of two long long ints in hex (with possible "-" signs) separated with an underscore
void gensector(long long int x, long long int y){//only called from loadsector
	char name[35];
	sprintf(name, "sectors/%llX_%llX", x, y);
	printf("gensector called (%s)\n", name);
	FILE *fp;
	fp = fopen(name, "w");
	//fprintf(fp, "sector\n");
	//other generaty thingys
	fclose(fp);
	fp = NULL;
	printf("done generating (%s)\n", name);
}
void loadsector(long long int x, long long int y){
	sector *new = malloc(sizeof(sector));
	FILE *fp;
	char name[35];
	sprintf(name, "sectors/%llX_%llX", x, y);//makes hexadecimal file name
	printf("loadsector called (%s)\n", name);

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

	listrootsector->firstobj = NULL;
	listrootsector->firstentity = NULL;
	listrootsector->x = x;
	listrootsector->y = y;

	interpretsector(x, y);
}
