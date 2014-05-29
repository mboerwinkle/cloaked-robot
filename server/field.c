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
	omgWtfLolBbq->next = mySector.firstentity;
	mySector.firstentity = omgWtfLolBbq;
}

void run(sector *sec){
	entity *current = sec.firstentity;
	entity *tmp;
	while(current){
		tmp = current->next;
		tick(current);
		current = tmp;
	}
}

void draw(){
	entity *current = mySector.firstentity;
	entity *tmp;
	while(current){
		tmp = current->next;
		drawEntity(current, 0, 0, zoom);
		current = tmp;
	}
}
