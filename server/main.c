#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <SDL2/SDL.h>

#include "globals.h"
#include "networking.h"

//#define SPEEDRUN
loadRequest *lastLoadRequest = NULL;
sector *listrootsector = NULL;

static void teamColor(SDL_Renderer *render, unsigned char faction, unsigned char tm){
	unsigned char r, g, b;
	if(faction == 0) {        //white, unaligned/inanimate
		r = g = b = 0xFF;
	} else if(faction == 1) { //red, pirates
		r = 0xFF;
		g = b = 0;
	} else if(faction == 2) { //blue, imperial
		r = g = 0;
		b = 0xFF;
	} else {                  //yellow, independent/traders
		r = g = 0xFF;
		b = 0;
	}
	if (tm == TM_NONE) {
		r /= 2;
		g /= 2;
		b /= 2;
	} else if (tm == TM_MINE) {
		r |= 0x80;
		g |= 0x80;
		b |= 0x80;
	} else if (tm == TM_FEED) {
		r |= 0x40;
		g |= 0x40;
		b |= 0x40;
	} // TM_DFND is default color
	SDL_SetRenderDrawColor(render, r, g, b, 0xFF);
}

int main(int argc, char **argv){
	//srandom(time(NULL));
	char showMap = argc > 1;
	SDL_Window *window = NULL;
	SDL_Renderer *render = NULL;
	if (showMap) {
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
		window = SDL_CreateWindow("Sector Overview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, POS_RANGE/(6400*16), POS_RANGE/(6400*16), 0);
		if(window == NULL){
			fputs("No SDL2 window.\n", stderr);
			fputs(SDL_GetError(), stderr);
			SDL_Quit();
			return 1;
		}
		render = SDL_CreateRenderer(window, -1, 0);
	}
#ifndef SPEEDRUN
	struct timespec t = {.tv_sec=0};
	struct timespec lastTime = {.tv_sec = 0, .tv_nsec = 0};
	struct timespec otherTime = {.tv_sec = 0, .tv_nsec = 0};
#endif

	sector *conductor, *tmp, **runner;
	listrootsector = NULL;
	initModules();
	initAis();
#ifdef SPEEDRUN
	loadsector(0, 0);
#endif
	//start network listening thread
	pthread_t id;
	pthread_create(&id, NULL, netListen, NULL);
	pthread_detach(id);

	short adventuretime = 300;//nope, make that "char timetillpotentialtospawnastroidssothatwedontcheckeachtickandslowshitdown"
	//nope, make that "char timeTillPotentialToSpawnAstroidsSoThatWeDontCheckEachTickAndSlowShitDown"
	while(1){
		if (firstLoadRequest != lastLoadRequest) {
			puts("Request detected");
			loadRequest *first = firstLoadRequest;
			loadRequest *runner = first;
			while (runner != lastLoadRequest) {
				loadRequest *next = runner->next;
				client *cli = runner->cli;
				cli->myShip = loadship(runner->name);
				if (cli->myShip == NULL) {
					printf("Failed to load ship \"%s\"\n", runner->name);
					free(cli);
				} else {
					cli->next = clientList;
					clientList = cli;
				}
				if (runner != first) free(runner); // Leave the first one allocated, so the next value of first is something new.
				runner = next;
			}
			if (runner) free(runner); // Clean up the still-allocated first one from last time.
			lastLoadRequest = first;
		}
		if(adventuretime-- == 0) {
			adventuretime = 300;
			int min = INT_MAX;
			for (conductor = listrootsector; conductor; conductor = conductor->nextsector) {
				if (conductor->realnumber == 0) {
					//spawnstroids(conductor);
				}
				if (conductor->topGuarantee == NULL) continue;
				if (conductor->topGuarantee->pto < min)
					min = conductor->topGuarantee->pto;
			}
			if (min) {
				for (conductor = listrootsector; conductor; conductor = conductor->nextsector) {
					conductor->topGuarantee->pto -= min;
				}
			}
			if (showMap) {
				if ((conductor = searchforsector(0, 0))) {
					SDL_SetRenderDrawColor(render, 0, 0, 0, 0);
					SDL_RenderClear(render);
					entity *runner;
					SDL_Rect r = {.w=2, .h=2};
					for (runner = conductor->firstentity; runner; runner = runner->next) {
						teamColor(render, runner->faction, runner->transponderMode);
						r.x = (runner->me->pos[0]-POS_MIN)/(6400*16);
						r.y = (runner->me->pos[1]-POS_MIN)/(6400*16);
						SDL_RenderFillRect(render, &r);
					}
					SDL_RenderPresent(render);
				}
				SDL_Event e;
				while (SDL_PollEvent(&e)) {
					if (e.type == SDL_QUIT) {
						SDL_DestroyRenderer(render);
						SDL_DestroyWindow(window);
						SDL_Quit();
						showMap = 0;
					}
				}
			}
		}
		conductor = listrootsector;
		while(conductor != NULL){
			run(conductor);
			conductor = conductor->nextsector;
		}
		doStep();
		conductor = listrootsector;
		while(conductor != NULL){
			run2(conductor);
			conductor = conductor->nextsector;
		}
		cleanup();
		runner = &listrootsector;
		while(*runner != NULL){
			if((*runner)->number == 0){
				tmp = *runner;
				*runner = (*runner)->nextsector;
				unloadsector(tmp);
			}else runner = &(*runner)->nextsector;
		}
		sendInfo();
		globalActedFlag ^= 1;
#ifndef SPEEDRUN
		clock_gettime(CLOCK_MONOTONIC, &otherTime);
		int32_t sleep = (int32_t)25000000 - (otherTime.tv_nsec-lastTime.tv_nsec) - 1000000000l*(otherTime.tv_sec-lastTime.tv_sec);
		if(sleep > 0){
			t.tv_nsec = sleep;
			nanosleep(&t, NULL);
		}
		clock_gettime(CLOCK_MONOTONIC, &lastTime);
#endif
	}
	return(0);
}
