#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "entity.h"
#include "field.h"
#include <math.h>

double zoom;
sector mySector;

void initField(){
	zoom = 1.0/250;
	mySector.numEntities = 5;
	mySector.entities = calloc(sizeof(entity*), mySector.numEntities);
}

void stopField(){
	int i = 0;
	for(; i < mySector.numEntities; i++){
		if(mySector.entities[i]){
			freeEntity(mySector.entities[i]);
		}
	}
	free(mySector.entities);
	mySector.numEntities = 0;
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
