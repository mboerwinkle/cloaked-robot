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

struct moveRequest{
	struct moveRequest *next;
	entity *who;
	sector *from, *to;
};
static struct moveRequest *firstRequest = NULL;

void fileMoveRequest(entity *who, sector* from, sector* to){
	struct moveRequest *new = malloc(sizeof(struct moveRequest));
	new->who = who;
	new->from = from;
	new->to = to;
	new->next = firstRequest;
	firstRequest = new;
}

void run(sector *sec){
	entity **current = &sec->firstentity;
	while(*current){
		if(tick(*current)){
			entity *tmp = *current;
			*current = (*current)->next;
			freeEntity(tmp);
			disappear(sec->x, sec->y); // TODO: Only do this if they were a player
		}else current = &(*current)->next;
	}
	struct moveRequest *tmp;
	while(firstRequest){
		move(firstRequest->from->x, firstRequest->from->y, firstRequest->to->x, firstRequest->to->y);
		current = &firstRequest->from->firstentity;
		while(*current != firstRequest->who) current = &(*current)->next;
		*current = (*current)->next;
		firstRequest->who->next = firstRequest->to->firstentity;
		firstRequest->to->firstentity = firstRequest->who;

		tmp = firstRequest->next;
		free(firstRequest);
		firstRequest = tmp;
	}
}
