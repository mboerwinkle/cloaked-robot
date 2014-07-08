#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <SDL2/SDL.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "gui.h"
#include "images.h"

#ifdef WINDOWS
#include <windows.h>
#endif

#define width 500
#define height 500

static SDL_Window* window;
SDL_Renderer* render;

static int running = 1;
static unsigned char keys = 0;
static unsigned char twoPow[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static int keyBindings[6] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_SPACE, SDLK_x, SDLK_z};

static int sockfd;
static struct sockaddr_in serverAddr;

spriteSheet* pictures;
SDL_Texture* background1;

static void paint(){
	SDL_RenderPresent(render);
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	SDL_RenderClear(render);
}

static void handleNetwork(){
	static int16_t data[3*100];
	struct sockaddr_in addr;
	socklen_t addrLen = sizeof(addr);
	int len;
	SDL_Rect rect;
	while(0<(len = recvfrom(sockfd, (char*)data, 600, 0, (struct sockaddr*)&addr, &addrLen))){
		addrLen = sizeof(addr);
		if(addr.sin_addr.s_addr != serverAddr.sin_addr.s_addr) continue;
		len/=2;
		rect.w = rect.h = 1500;
		rect.x = data[0]-1500;
		rect.y = data[1]-1500;
		SDL_RenderCopy(render, background1, NULL, &rect);
		if(data[0]<500){
			rect.x = data[0];
			SDL_RenderCopy(render, background1, NULL, &rect);
			if(data[1]<500){
				rect.y = data[1];
				SDL_RenderCopy(render, background1, NULL, &rect);
				rect.x = data[0]-1500;
				SDL_RenderCopy(render, background1, NULL, &rect);
			}
		}else if(data[1]<500){
			rect.y = data[1];
			SDL_RenderCopy(render, background1, NULL, &rect);
		}
		unsigned char theta = 0x0F & data[2];
//		char flame = 0x10 & data[i];
//		char faction = (0x60 & data[i])/0x20;
		int ship = (0xFF80 & data[2]) / 0x80;
		int size = rect.w = rect.h = pictures[ship].size;
		rect.x =  width/2-size/2;
		rect.y = height/2-size/2;
		SDL_RenderCopy(render, pictures[ship].data[theta], NULL, &rect);
		int i = 4;
		SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
		while(i+3 < len){
			theta = 0x0F & data[i];
//			char flame = 0x10 & data[i];
//			char faction = (0x60 & data[i])/0x20;
			ship = (0xFF80 & data[i]) / 0x80;
			size = rect.w = rect.h = pictures[ship].size;
			int x = data[++i];
			int y = data[++i];
			rect.x =  width/2-size/2+x;
			rect.y = height/2-size/2+y;
			SDL_RenderCopy(render, pictures[ship].data[theta], NULL, &rect);
			if(data[++i] & 0x20){
				int index = abs(x)>abs(y) ? i-2 : i-1;
				if(fabs(data[index]) > width/2){
					double frac = (double)(width/2) / abs(data[index]);
					size *= frac;
					rect.x =  width/2-size/2+x*frac;
					rect.y = height/2-size/2+y*frac;
					rect.w = rect.h = size;
				}
				SDL_RenderDrawRect(render, &rect);
				rect.x = width/2-size/2+x;
				rect.w = pictures[ship].size;
			}
			rect.y = height/2+size/2+y+2;
			rect.h = 3;
			rect.w = rect.w*(data[i]&0x1F)/31;
			SDL_RenderFillRect(render, &rect);
			i++;
		}
		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
		rect.y = height;
		rect.h = 20;
		rect.x = 0;
		rect.w = width;
		SDL_RenderFillRect(render, &rect);
		SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
		rect.x = width/2-width/2*(data[3]&0xFF)/255;
		rect.w = width*(data[3]&0xFF)/255;
		rect.h = 10;
		SDL_RenderFillRect(render, &rect);
		SDL_SetRenderDrawColor(render, 0, 0, 255, 255);
		rect.y += 10;
		rect.x = width/2-width/2*((data[3]&0xFF00)/0x100)/255;
		rect.w = width*((data[3]&0xFF00)/0x100)/255;
		SDL_RenderFillRect(render, &rect);
		paint();
	}
}

static void spKeyAction(int bit, char pressed){
	int i = 0;
	for(; i < 6 && bit!=keyBindings[i]; i++);
	if(i==6) return;
	unsigned char old = keys;
	if(pressed) keys |= twoPow[i];
	else keys &= 0xFF-twoPow[i];
	if(keys == old) return;
	sendto(sockfd, &keys, 1, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
}

int main(int argc, char** argv){
	if(argc < 2){
		puts("Please specify an ip.");
		return 5;
	}
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	window = SDL_CreateWindow("Ship Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height+20, 0);
	if(window == NULL){
		fputs("No SDL2 window.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_Quit();
		return 1;
	}
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	loadPics();

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		puts("When in danger,\nOr in doubt,\nRun in circles!\nScream and shout!");
		return 1;
	}
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(3334);
	if(0 > bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr))){
		puts("When in danger,\nOr in doubt,\nRun in circles!\nScream and shout!!!");
		return 2;
	}
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
	serverAddr.sin_port=htons(3333);
	inet_aton(argv[1], &serverAddr.sin_addr);

	char* tmp = "]yo";
	sendto(sockfd, tmp, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	struct timespec t = {.tv_sec = 0, .tv_nsec = 10000000};
	while(running){
		SDL_Event evnt;
		while(SDL_PollEvent(&evnt)){
			if(evnt.type == SDL_QUIT)		running = 0;
			else if(evnt.type == SDL_KEYDOWN){
				spKeyAction(evnt.key.keysym.sym, 1);
			}
			else if (evnt.type == SDL_KEYUP)	spKeyAction(evnt.key.keysym.sym, 0);
		}
		handleNetwork();
		nanosleep(&t, NULL);
	}
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
