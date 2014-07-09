#include <stdlib.h>
#include <stdio.h>
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
	if(who->y < POS_MIN+radius) u=1;
	else if(who->y > POS_MAX-radius) d=1;
	if(who->x < POS_MIN+radius) l=1;
	else if(who->x > POS_MAX-radius) r=1;

	numLinked = 0;
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
	fputc('.', stderr);
}

void unlinkNear(){
	for(numLinked--; numLinked>=0; numLinked--) linkedIn[numLinked]->next = NULL;
	fputc('\b', stderr);
}
