#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "globals.h"
#include <math.h>

char globalActedFlag = 0;

struct moveRequest{
	struct moveRequest *next;
	entity *who;
	sector *from, *to;
};
static struct moveRequest *firstRequest = NULL;
static entity* thoseCondemnedToDeath = NULL;

void fileMoveRequest(entity *who, sector* from, sector* to){
	struct moveRequest *new = malloc(sizeof(struct moveRequest));
	new->who = who;
	new->from = from;
	new->to = to;
	new->next = firstRequest;
	firstRequest = new;
}

static void addAsteroid(entity* poorSoul, int type){
	entity* assRoid = newEntity(type, 3, 0, poorSoul->mySector, poorSoul->x, poorSoul->y);
	*(char*)assRoid->aiFuncData = 1-2*(random()%2); // It has been hit, tumble, plz.
	double theta = ((double)random()/RAND_MAX)*(2*M_PI);
	int spd = random() % (int)(poorSoul->r / 6.4);
	assRoid->vx = poorSoul->vx + cos(theta)*spd;
	assRoid->vy = poorSoul->vy + sin(theta)*spd;
	assRoid->theta = random()%16;
}

void run(sector *sec){
	entity *current = sec->firstentity;
	while(current){
		tick(current);
		current = current->next;
	}
}

void run2(sector *sec){
	entity *prev = NULL;
	entity *current = sec->firstentity;
	char result;
	while(current){
		if( (result=tick2(current)) ){
			entity *tmp = current;
			current = current->next;
			if(prev) prev->next = current;
			else sec->firstentity = current;
			tmp->destroyFlag = 3;
			tmp->next = thoseCondemnedToDeath;
			thoseCondemnedToDeath = tmp;
			if(result == 2){
				int size = (2.0/3)*(tmp->r*tmp->r + tmp->minerals);
				if(size>=320*320){
					while(size >= 1280*1280){
						size -= 1280*1280;
						addAsteroid(tmp, 12);
					}
					while(size >= 704*704){
						size -= 704*704;
						addAsteroid(tmp, 4);
					}
					while(size >= 320*320){
						size -= 320*320;
						addAsteroid(tmp, 5);
					}
					if(prev == NULL){ // ... Then it isn't anymore
						for(prev = sec->firstentity; prev->next != current; prev = prev->next);
					}
				}
			}
			if(tmp->myAi->loadSector)
				disappear(sec->x, sec->y);
		}else{
			prev = current;
			current = current->next;
		}
	}
}

void cleanup(){
	entity** current;
	struct moveRequest *tmp;
	while(firstRequest){
		if(firstRequest->who->myAi->loadSector) move(firstRequest->from->x, firstRequest->from->y, firstRequest->to->x, firstRequest->to->y);
		current = &firstRequest->from->firstentity;
		while(*current != firstRequest->who) current = &(*current)->next;
		*current = (*current)->next;
		firstRequest->who->next = firstRequest->to->firstentity;
		firstRequest->to->firstentity = firstRequest->who;

		tmp = firstRequest->next;
		free(firstRequest);
		firstRequest = tmp;
	}
	current = &thoseCondemnedToDeath;
	while(*current){
		if(--((*current)->destroyFlag) == 0){
			entity *tmp = *current;
			*current = (*current)->next;
			freeEntity(tmp);
		}else current = &(*current)->next;
	}
}
