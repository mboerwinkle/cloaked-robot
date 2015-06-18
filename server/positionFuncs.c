#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "globals.h"

static entity *(linkedIn[3]);
static int numLinked;
static entity* what;

static void addLinkedSector(sector* whom){
	if(whom && whom->firstentity){
		while(what->next) what = what->next;
		linkedIn[numLinked++] = what;
		what->next = whom->firstentity;
	}
}

//Assumes who is actually listed in who->mySector's list
void linkNear(entity* who, int32_t radius){
	uint64_t x = who->mySector->x;
	uint64_t y = who->mySector->y;
	char u=0, d=0, l=0, r=0;
	if(who->me->pos[1] < POS_MIN+radius) u=1;
	else if(who->me->pos[1] > POS_MAX-radius) d=1;
	if(who->me->pos[0] < POS_MIN+radius) l=1;
	else if(who->me->pos[0] > POS_MAX-radius) r=1;

	if(numLinked != 0) {
		puts("I think we may have (yet another) linkNear issue...");
		numLinked = 0;
	}
	what = who;
	if(l){
		addLinkedSector(searchforsector(x-1, y));
		if(u) addLinkedSector(searchforsector(x-1, y-1));
		else if(d) addLinkedSector(searchforsector(x-1, y+1));
	}else if(r){
		addLinkedSector(searchforsector(x+1, y));
		if(u) addLinkedSector(searchforsector(x+1, y-1));
		else if(d) addLinkedSector(searchforsector(x+1, y+1));
	}
	if(u) addLinkedSector(searchforsector(x, y-1));
	else if(d) addLinkedSector(searchforsector(x, y+1));
}

void unlinkNear(){
	if(numLinked == 0) return;
	for(numLinked--; numLinked>=0; numLinked--) linkedIn[numLinked]->next = NULL;
	numLinked = 0;
}

guarantee* getCloseEntGuarantee(sector *sec, int32_t x, int32_t y) {
	guarantee *ret = sec->topGuarantee;
	if (ret == NULL) return NULL;
	while (ret->ent == NULL) {
		guarantee *bestFit = NULL;
		int32_t score = INT32_MAX;
		int i;
		for (i = ret->numKids - 1; i >= 0; i--) {
			int32_t dx = abs(ret->kids[i]->pos[0] - x);
			int32_t dy = abs(ret->kids[i]->pos[1] - y);
			if (dy > dx) dx = dy;
			if (dx < score) {
				score = dx;
				bestFit = ret->kids[i];
			}
		}
		if (bestFit == NULL) {
			puts("Strange brew!");
			return NULL;
		}
		ret = bestFit;
	}
	return ret;
}
