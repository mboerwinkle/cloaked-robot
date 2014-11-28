#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"

void gensector(uint64_t x, uint64_t y){//only called from loadsector
	printf("gensector called (%lld, %lld)\n", x, y);
	char name[35];
	sprintf(name, "sectors/%lld_%lld", x, y);
	FILE *fp;
	fp = fopen(name, "w");
	//fprintf(fp, "sector\n");
	//other generaty thingys
	fclose(fp);
	fp = NULL;
	printf("done generating (%s)\n", name);
}
static void readEntity(FILE* fp, sector* sec){
	char line[80];
	fgets(line, 80, fp);
	if(strcmp(line, "entity\n")){
		perror("Da fuq? Corrupt file in loadsector.c\n");
		return;
	}
	int type = atoi(fgets(line, 80, fp));
	int ai = atoi(fgets(line, 80, fp));
	int faction = atoi(fgets(line, 80, fp));
	int x = atoi(fgets(line, 80, fp));
	int y = atoi(fgets(line, 80, fp));
	fgets(line, 80, fp);
	if(strcmp(line, "end\n")){
		perror("Da fuq? Corrupt file in loadsector.c.\n");
		return;
	}
	newEntity(type, ai, faction, sec, x, y);
}

void loadsector(uint64_t x, uint64_t y){
	printf("loadsector called (%lld, %lld)\n", x, y);
	sector *new = malloc(sizeof(sector));
	FILE *fp;
	char name[35];
	sprintf(name, "sectors/%lld_%lld", x, y);//makes hexadecimal file name

	if((fp = fopen(name, "r")) == NULL){
		printf("sector %s does not exist, generating\n", name);
		gensector(x, y);
	}

	new->nextsector = listrootsector;
	listrootsector = new;

	listrootsector->number = 1;
	listrootsector->firstentity = NULL;
	listrootsector->x = x;
	listrootsector->y = y;

	if(fp != NULL){
		int a;
		while((a = fgetc(fp)) != EOF){
			ungetc(a, fp); // Oops, not EOF, better put that back
			readEntity(fp, listrootsector);
		}
		fclose(fp);
		fp = NULL;
	}
}
