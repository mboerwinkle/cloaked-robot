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
#include "trails.h"

#ifdef WINDOWS
#include <windows.h>
#endif

#define width 500
#define height 500

static SDL_Window* window;
SDL_Renderer* render;
static SDL_Texture* minimapTex;

static int running = 1;
static unsigned char keys = 0;
static unsigned char twoPow[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#define numKeys 7
static int keyBindings[numKeys] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_x, SDLK_z, SDLK_a, SDLK_s};

static int sockfd;
static struct sockaddr_in serverAddr;

spriteSheet* pictures;
SDL_Texture* background1;
SDL_Texture* lolyoudied;

static void paint(){
	SDL_RenderPresent(render);
}

static void teamColor(char faction){
	if(faction == 0) SDL_SetRenderDrawColor(render, 255, 255, 255, 255);//white, unaligned
	if(faction == 1) SDL_SetRenderDrawColor(render, 255, 0, 0, 255);//red, pirates
	if(faction == 2) SDL_SetRenderDrawColor(render, 0, 0, 255, 255);//blue, imperial
	if(faction == 3) SDL_SetRenderDrawColor(render, 255, 255, 0, 255);//yellow, independent, traders
}

static void drawRadar(int8_t* data, int len){
	SDL_SetRenderTarget(render, minimapTex);
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	SDL_RenderClear(render);
	SDL_Rect rect = {.w = 2, .h = 2};
	SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
	if(data[2]&128){
		puts("column");
		SDL_RenderDrawLine(render, data[0]-128, 0, data[0]-128, 128);
	}
	if(data[2]&64){
		puts("row");
		SDL_RenderDrawLine(render, 0, data[1], 128, data[1]);
	}
	int i = 2;
	while(i+2 < len){
		teamColor(data[i]&0x3F);
		rect.x = data[i+1];
		rect.y = data[i+2];
		SDL_RenderFillRect(render, &rect);
		i+=3;
	}
	SDL_SetRenderTarget(render, NULL);
}

static void handleNetwork(){
	static int8_t data[6000];
	struct sockaddr_in addr;
	socklen_t addrLen = sizeof(addr);
	int len;
	SDL_Rect rect;
	while(0<(len = recvfrom(sockfd, (char*)data, 600, 0, (struct sockaddr*)&addr, &addrLen))){
		addrLen = sizeof(addr);
		if(addr.sin_addr.s_addr != serverAddr.sin_addr.s_addr) continue;
		if(*data & 0x80){
			drawRadar(data, len);
			continue;
		}
		if(*data & 0x40){ // A control packet
			if(*data == 0x41){
				rect.x = width/2-100;
				rect.y = height/2-50;
				rect.w = 200;
				rect.h = 100;
				SDL_RenderCopy(render, lolyoudied, NULL, &rect);
				paint();
				continue;
			}
		}
		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
		rect.x = rect.y = 0;
		rect.w = width;
		rect.h = height+20;
		SDL_RenderFillRect(render, &rect);
		rect.w = rect.h = 1500;
		int16_t bgx = *(int16_t*)(data+1);
		int16_t bgy = *(int16_t*)(data+3);
		rect.x = bgx-1500;
		rect.y = bgy-1500;
		SDL_RenderCopy(render, background1, NULL, &rect);
		if(bgx<500){
			rect.x = bgx;
			SDL_RenderCopy(render, background1, NULL, &rect);
			if(bgy<500){
				rect.y = bgy;
				SDL_RenderCopy(render, background1, NULL, &rect);
				rect.x = bgx-1500;
				SDL_RenderCopy(render, background1, NULL, &rect);
			}
		}else if(bgy<500){
			rect.y = bgy;
			SDL_RenderCopy(render, background1, NULL, &rect);
		}
		int i = 7;
		while(i+6 < len){
			unsigned char theta = 0x0F & data[i];
//			char flame = 0x10 & data[i];
			char faction = (0xE0 & data[i])/0x20;
			int ship = (uint8_t)data[++i];
			int size = rect.w = rect.h = pictures[ship].size;
			int x = *(int16_t*)(data+(++i));
			int y = *(int16_t*)(data+(i+=2));
			rect.x =  width/2-size/2+x;
			rect.y = height/2-size/2+y;
			SDL_RenderCopy(render, pictures[ship].data[theta], NULL, &rect);
			if(data[i+=2] & 0x20){
				SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
				int index = abs(x)>abs(y) ? i-4 : i-2;
				if(abs(*(int16_t*)(data+index)) > width/2){
					double frac = (double)(width/2) / abs(*(int16_t*)(data+index));
					size *= frac;
					rect.x =  width/2-size/2+x*frac;
					rect.y = height/2-size/2+y*frac;
					rect.w = rect.h = size;
				}
				SDL_RenderDrawRect(render, &rect);
				rect.x = width/2-size/2+x;
				rect.w = pictures[ship].size;
			}
			teamColor(faction);
			rect.y = height/2+size/2+y+2;
			rect.h = 3;
			rect.w = rect.w*(data[i]&0x1F)/31;
			SDL_RenderFillRect(render, &rect);
			x += width/2;
			y += height/2;
			while((data[i]&0x80) && i+3<len){
				int dx = ((uint8_t*)data)[i+1];
				int dy = ((uint8_t*)data)[i+2];
				if(data[i+3] & 0x40) dx-=256;
				if(data[i+3] & 0x20) dy-=256;
				addTrail(x, y, x+dx, y+dy, data[i+3]&0x1F);
				i+=3;
			}
			i++;
		}
		drawTrails(render);
		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
		rect.y = height;
		rect.h = 20;
		rect.x = 0;
		rect.w = width;
		SDL_RenderFillRect(render, &rect);	
		teamColor(data[0]);
		rect.x = width/2-width/2*(uint8_t)data[5]/255;
		rect.w = width*(uint8_t)data[5]/255;
		rect.h = 10;
		SDL_RenderFillRect(render, &rect);
		SDL_SetRenderDrawColor(render, 0, 0, 255, 255);
		rect.y += 10;
		rect.x = width/2-width/2*(uint8_t)data[6]/255;
		rect.w = width*(uint8_t)data[6]/255;
		SDL_RenderFillRect(render, &rect);

		SDL_SetRenderDrawColor(render, 50, 50, 50, 255);
		rect.x = width;
		rect.y = 0;
		rect.w = 130;
		rect.h = height+20;
		SDL_RenderFillRect(render, &rect);

		rect.x = width+1;
		rect.y = 1;
		rect.w = 128;
		rect.h = 128;
		SDL_RenderCopy(render, minimapTex, NULL, &rect);

		paint();
	}
}

static void spKeyAction(int bit, char pressed){
	int i = 0;
	for(; i < numKeys && bit!=keyBindings[i]; i++);
	if(i==numKeys) return;
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
	window = SDL_CreateWindow("Ship Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width+130, height+20, 0);
	if(window == NULL){
		fputs("No SDL2 window.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_Quit();
		return 1;
	}
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	minimapTex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 128, 128);

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
