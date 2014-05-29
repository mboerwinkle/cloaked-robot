#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "gfx.h"
#include "font.h"
#include "entity.h"
#include "field.h"
#include "gui.h"
#include "ais.h"
#include "module.h"
#include <SDL2/SDL.h>

#ifdef WINDOWS
#include <windows.h>
#endif

static SDL_Window* window;

static int pKeys[2][NUMKEYS] = {{SDLK_w, SDLK_d, SDLK_a, SDLK_x, SDLK_z},\
	{SDLK_UP, SDLK_RIGHT, SDLK_LEFT, SDLK_RCTRL, SDLK_RSHIFT}};
char myKeys[2][NUMKEYS] = {{0}, {0}};

char mode = 1;
static int running = 1;

int width, height;

FILE* logFile;

void myDrawScreen(){
	SDL_GL_SwapWindow(window);
	glClear(GL_COLOR_BUFFER_BIT);
}

static inline void simpleDrawText(int line, char* text){
	drawText(20-width/2, 20+TEXTSIZE*9*line+height/2, TEXTSIZE, text);
}

static void paint(){
	GLenum error = glGetError();
	if(error){
		fputs((const char*)gluErrorString(error), logFile);
		fputc('\n', logFile);
		fflush(logFile);
	}
	if(mode == 0){
		fputs("Well, thomething theemth to be amith\n", stderr);
	}else{
		run();
		draw();
		myDrawScreen();
	}
}

static void spKeyAction(int bit, char pressed){
	if(bit == SDLK_ESCAPE){
		if(!pressed) return;
		stopField();
		running = 0;
		return;
	}
	int i, j;
	for(j=0; j < 2; j++){
		for(i=0; i<NUMKEYS; i++){
			if(pKeys[j][i]==bit){
				myKeys[j][i] = pressed;
				return;
			}
		}
	}
}

int main(int argc, char** argv){
	logFile = fopen("log.txt", "w");
	fputs("Everything looks good from here\n", logFile);

	srand(time(NULL));
#ifndef WINDOWS
	struct timespec t;
	t.tv_sec = 0;
	struct timespec lastTime = {.tv_sec = 0, .tv_nsec = 0};
	struct timespec otherTime = {.tv_sec = 0, .tv_nsec = 0};
#else
	HANDLE hTimer = CreateWaitableTimer(NULL, 1, NULL);
	FILETIME lastTime = {.dwLowDateTime = 0, .dwHighDateTime = 0};
	LARGE_INTEGER largeInt;
#endif
	fputs("Timing initialized\n", logFile);
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	fputs(SDL_GetError(), logFile);
	fputc('\n', logFile);
	SDL_ClearError();
	width = 500;
	height = 500;
	window = SDL_CreateWindow("Bounce Brawl", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
	fputs(SDL_GetError(), logFile);
	fputc('\n', logFile);
	SDL_ClearError();
	if(window == NULL){
		fputs("No SDL2 window.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_Quit();
		return 1;
	}
	fputs("Created Window\n", logFile);
	SDL_GLContext context = SDL_GL_CreateContext(window);
	fputs(SDL_GetError(), logFile);
	fputc('\n', logFile);
	SDL_ClearError();
	fputs("Created GL Context\n", logFile);
	initGfx(logFile);
	glClearColor(0, 0, 0, 1);
	fputs(SDL_GetError(), logFile);
	fputc('\n', logFile);
	SDL_ClearError();
	fputs("Intialized Graphics, Entering Main Loop\n", logFile);
	fflush(logFile);

	initModules();
	//Probably ought to move setup to elsewhere, but oh well
	initField();
	entity* me = newEntity(0, 0, 0);
	((struct aiHumanData*)me->aiFuncData)->player = 1;
	addEntity(me);
	me = newEntity(0, 0, -50);
	((struct aiHumanData*)me->aiFuncData)->player = 0;
	addEntity(me);

	while(running){
		paint();///Also runs the thing if relevant.
#ifndef WINDOWS
		clock_gettime(CLOCK_MONOTONIC, &otherTime);
		long int sleep = (long int)(25000000) - (otherTime.tv_nsec-lastTime.tv_nsec+1000000000l*(otherTime.tv_sec-lastTime.tv_sec));
		if(sleep > 0){
			t.tv_nsec = sleep;
			nanosleep(&t, NULL);
		}
		clock_gettime(CLOCK_MONOTONIC, &lastTime);
#else
		largeInt.LowPart = lastTime.dwLowDateTime;
		largeInt.HighPart = lastTime.dwHighDateTime;
		largeInt.QuadPart += 250000;
		SetWaitableTimer(hTimer, &largeInt, 0, NULL, NULL, 0);
		WaitForSingleObject(hTimer, INFINITE);
		GetSystemTimeAsFileTime(&lastTime);
#endif

		SDL_Event evnt;
		while(SDL_PollEvent(&evnt)){
			if(evnt.type == SDL_QUIT)		running = 0;
			else if(evnt.type == SDL_KEYDOWN)	spKeyAction(evnt.key.keysym.sym, 1);
			else if (evnt.type == SDL_KEYUP)	spKeyAction(evnt.key.keysym.sym, 0);
		}
	}
	fclose(logFile);
	SDL_DestroyWindow(window);
#ifdef WINDOWS
	CloseHandle(hTimer);
#endif
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}
