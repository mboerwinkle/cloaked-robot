#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "globals.h"
#include <math.h>
#include <netinet/in.h>
#include "networking.h"

#define numDeathMessages 16
static char const *(deathMessages[numDeathMessages]) = {
	"should have stayed on Dagobah and finished their training!",
	"should have diverted power to shields!",
	"should have tried a crazy Ivan!",
	"was unceremoniously vaporized!",
	"was murderfied!",
	"was killified!",
	"looks moderately cadaverous!",
	"doesn't look very healthy...",
	"made modern art out of their ship!",
	"has an odd idea of fun...",
	"really should have installed escape pods!",
	"won't be missed as much as will be said in the obit!",
	"has to wait 3 friggin' seconds!",
	"let their ship explode!",
	"was removed from the gene pool!",
	"isn't here right now, please leave a message."};

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
	guarantee *g = poorSoul->me;
	double theta = ((double)random()/RAND_MAX)*(2*M_PI);
	int spd = random() % (int)(g->r / 6.4);
	entity* assRoid = newEntity(g, type, 3, 0, poorSoul->mySector, g->pos[0], g->pos[1], g->vel[0] + cos(theta)*spd, g->vel[1] + sin(theta)*spd);
	*(char*)assRoid->aiFuncData = 1-2*(random()%2); // It has been hit, tumble, plz.
	assRoid->theta = random()%16;
}

void run(sector *sec){
	entity *current = sec->firstentity;
	while(current){
		tick(current);
		current = current->next;
	}
	doStep(sec->firstentity);
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
				int size = (2.0/3)*(tmp->me->r*tmp->me->r + tmp->minerals);
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
			if (tmp->myAi->loadSector) {
				client *runner = clientList;
				while (runner) {
					if (runner->myShip == tmp) {
						printf("Oh No!!! Player %s %s\n", runner->name, deathMessages[random()%numDeathMessages]);
						memcpy(&runner->ghostGuarantee, tmp->me, sizeof(guarantee));
						break; // If it's a client, the disappear will be handled separately, 3 seconds later.
					}
					runner = runner->next;
				}
				if (runner == NULL)
					disappear(sec->x, sec->y);
			}
			destroyGuarantee(tmp->me);
		} else {
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
