#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include "globals.h"

//PRIX64 is a nifty macro from inttypes.h.
static void gensector(uint64_t x, uint64_t y){//only called from loadsector
	printf("gensector called (%"PRId64", %"PRId64")\n", x, y);
	char name[42];
	sprintf(name, "sectors/%"PRIX64"_%"PRIX64, x, y);
	FILE *fp;
	fp = fopen(name, "w");
	//fprintf(fp, "sector\n");
	//other generaty thingys
	fclose(fp);
	fp = NULL;
	//printf("done generating (%s)\n", name);
}
static guarantee* readEntity(FILE* fp, sector* sec, guarantee* prev){
	char line[80];
	if (fgets(line, 80, fp) == NULL || strcmp(line, "entity\n")) {
		perror("Da fuq? Corrupt file in loadsector.c\n");
		return prev;
	}
	int type = atoi(fgets(line, 80, fp));
	int ai = atoi(fgets(line, 80, fp));
	int faction = atoi(fgets(line, 80, fp));
	int x = atoi(fgets(line, 80, fp));
	int y = atoi(fgets(line, 80, fp));
	if (fgets(line, 80, fp) == NULL || strcmp(line, "end\n")) {
		perror("Da fuq? Corrupt file in loadsector.c.\n");
		return prev;
	}
	return newEntity(prev, type, ai, faction, sec, x, y, 0, 0)->me;
}

void loadsector(uint64_t x, uint64_t y){
	printf("loadsector called (%"PRId64", %"PRId64")\n", x, y);
	sector *new = malloc(sizeof(sector));
	FILE *fp;
	char name[42];
	sprintf(name, "sectors/%"PRIX64"_%"PRIX64, x, y);//makes hexadecimal file name

	if((fp = fopen(name, "r")) == NULL){
		//printf("sector %s does not exist, generating\n", name);
		gensector(x, y);
	}

	new->nextsector = listrootsector;
	listrootsector = new;

	new->realnumber = 0;
	new->number = 1;
	new->firstentity = NULL;
	new->x = x;
	new->y = y;

	if(fp != NULL){
		guarantee *prev = NULL;
		int a;
		while((a = fgetc(fp)) != EOF){
			ungetc(a, fp); // Oops, not EOF, better put that back
			prev = readEntity(fp, new, prev);
		}
		fclose(fp);
		fp = NULL;
	}
}
