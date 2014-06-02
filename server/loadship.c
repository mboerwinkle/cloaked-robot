#include "globals.h"
#include <stdlib.h>
#include <stdio.h>
int loadship(char name[MAXNAMELEN]){
	char path[MAXNAMELEN + 6];
	FILE *fp;
	long long int secx, secy;
	long int posx, posy;
	sector *conductor;
	entity *newship = malloc(sizeof(entity));
	sprintf(path, "ships/%s", name);
	if((fp = fopen(path, "r")) == NULL){
		return(-1);
	}
	fscanf(fp, "%llX_%llX\n%ld %ld", &secx, &secy, &posx, &posy);
	fclose(fp);
	conductor = listrootsector;
	while(conductor != NULL && (conductor->x != secx || conductor->y != secy)){
		conductor = conductor->nextsector;
	}
	if(conductor == NULL){
		loadsector(secx, secy);
		conductor = listrootsector;
	}
	newship->next = conductor->firstentity;
	conductor->firstentity = newship;
	
	return(0);
}
