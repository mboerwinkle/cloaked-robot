#include "globals.h"
#include <stdlib.h>
#include <stdio.h>
int loadship(char name[MAXNAMELEN]){
	char path[MAXNAMELEN + 6];
	FILE *fp;
	long long int secx, secy;
	long int posx, posy;
	sector
	sprintf(path, "ships/%s", name);
	if((fp = fopen(path, "r")) == NULL){
		return(-1);
	}
	fscanf(fp, "%X_%X\n%ld %ld", secx, secy, posx, posy);
	
	return(0);
}
