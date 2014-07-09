#include "globals.h"
#include <stdlib.h>
#include <stdio.h>

entity *loadship(char name[MAXNAMELEN]){
	printf("loadship called (%s)\n", name);
	char path[MAXNAMELEN + 6];
	FILE *fp;
uint64_t secx, secy;
	int32_t posx, posy;
	sector *conductor;
	sprintf(path, "ships/%s", name);
	if((fp = fopen(path, "r")) == NULL){
		return NULL;
	}
	fscanf(fp, "%lX_%lX\n%d %d", &secx, &secy, &posx, &posy);
	fclose(fp);
	appear(secx, secy);
	conductor = searchforsector(secx, secy);
	return newEntity(2, conductor, posx, posy);
}
