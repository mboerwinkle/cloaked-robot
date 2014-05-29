#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "globals.h"
#include <math.h>

double zoom;
sector mySector;

void initField(){
	zoom = 1.0/250;
	mySector.firstentity = NULL;
}

void stopField(){
	entity *current = mySector.firstentity;
	entity *tmp;
	while(current){
		tmp = current->next;
		freeEntity(current);
		current = tmp;
	}
}

void addEntity(entity* omgWtfLolBbq){
	int i = 0;
	for(; i < mySector.numEntities; i++){
		if(mySector.entities[i]) continue;
		mySector.entities[i] = omgWtfLolBbq;
		return;
	}
	mySector.entities = realloc(mySector.entities, sizeof(entity*)*(mySector.numEntities+=5));
	mySector.entities[i] = omgWtfLolBbq;
	for(i++; i<mySector.numEntities; i++) mySector.entities[i] = NULL;
	return;
}

void run(){
	int i = 0;
	for(; i < mySector.numEntities; i++){
		if(mySector.entities[i]) tick(mySector.entities[i]);
	}
}

void draw(){
	int i = 0;
	for(; i < mySector.numEntities; i++){
		if(mySector.entities[i]) drawEntity(mySector.entities[i], 0, 0, zoom);
	}
}
