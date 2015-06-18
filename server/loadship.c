#include "globals.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

entity *loadship(char name[MAXNAMELEN]){
	printf("loadship called (%s)\n", name);
	char path[MAXNAMELEN + 6], faction, shipType, aiType;
	FILE *fp;
	uint64_t secx, secy;
	int32_t posx, posy;
	sector *conductor;
	sprintf(path, "ships/%s", name);
	if((fp = fopen(path, "r")) == NULL){
		return NULL;
	}
	if (4 > fscanf(fp, "%"PRId64" %"PRId64"\n%"PRId32" %"PRId32"\n", &secx, &secy, &posx, &posy)) {
		puts("Looks like a corrupt file in loadship.c");
		return NULL;
	}
	if (3 > fscanf(fp, "%hhd %hhd %hhd", &shipType, &faction, &aiType)) {
		puts("Looks like a corrupt file in loadship.c. "); // Bogus space at the end of one message so the discerning debugger can deduce which line the error came from
		return NULL;
	}
	fclose(fp);
	//printf("%"PRId64" %" PRId64 "\n", secx, secy);
	appear(secx, secy);
	conductor = searchforsector(secx, secy);
	entity *ret = newEntity(getCloseEntGuarantee(conductor, posx, posy), shipType, aiType, faction, conductor, posx, posy, 0, 0);
	disappear(secx, secy); // Why would I do such a thing? Because newEntity has to do an appear itself, for paladin spawning & such. However, I can't get rid of this function's appear call either, because the sector needs to be loaded when we spawn the player.
	return ret;
}
