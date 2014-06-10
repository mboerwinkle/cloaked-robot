#include "globals.h"
#include <stdlib.h>
#include <stdio.h>

entity *loadship(char name[MAXNAMELEN]){
	printf("loadship called (%s)\n", name);
	char path[MAXNAMELEN + 6];
	FILE *fp;
	long long int secx, secy;
	long int posx, posy;
	sector *conductor;
	sprintf(path, "ships/%s", name);
	if((fp = fopen(path, "r")) == NULL){
		return NULL;
	}
	fscanf(fp, "%llX_%llX\n%ld %ld", &secx, &secy, &posx, &posy);
	fclose(fp);
	appear(secx, secy);
	conductor = searchforsector(secx, secy);
	conductor->number++;
	return newEntity(0, conductor, posx, posy);
}
