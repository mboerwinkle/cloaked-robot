#include <stdlib.h>
#include <SDL2/SDL.h>
#include "trails.h"

static trail* trails = NULL;
static int numTrails = 0, maxTrails = 0;

void addTrail(int x1, int y1, int x2, int y2, int type){
	if(numTrails == maxTrails){
		trails = realloc(trails, sizeof(trail)*(maxTrails+=2));
	}
	trails[numTrails].x1 = x1;
	trails[numTrails].y1 = y1;
	trails[numTrails].x2 = x2;
	trails[numTrails].y2 = y2;
	trails[numTrails].type = type;
	trails[numTrails].life = 20;
	numTrails++;
}

void drawTrails(SDL_Renderer* render){
	SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);
	int i = 0;
	for(; i < numTrails; i++){
		if(trails[i].type == 0){
			SDL_SetRenderDrawColor(render, 255, 0, 0, 12*trails[i].life);
			SDL_RenderDrawLine(render, trails[i].x1, trails[i].y1, trails[i].x2, trails[i].y2);
		}
		if(--trails[i].life == 0){
			trails[i] = trails[--numTrails];
			i--;
		}
	}
	SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_NONE);
}
