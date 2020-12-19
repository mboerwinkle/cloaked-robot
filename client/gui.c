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
#include "font.h"

#ifdef WINDOWS
#include <windows.h>
#endif

#define width 500
#define height 500

#ifndef SCREEN_MULTIPLE
#warning Defaulting SCREEN_MULTIPLE to 1
#define SCREEN_MULTIPLE 1
#endif

#ifndef SIDEBAR_MULTIPLE
#warning Defaulting SIDEBAR_MULTIPLE to 2
#define SIDEBAR_MULTIPLE 2
#endif

static SDL_Window* window;
SDL_Renderer* render;
static SDL_Texture* minimapTex;

static int running = 1;
static unsigned char keys = 0;
static unsigned char twoPow[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#define numKeys 7
static int keyBindings[numKeys] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_a, SDLK_s, SDLK_d, SDLK_f};
static int thrustKey = SDLK_t;
#define numCommands 7
static int commandBindings[numCommands] = {SDLK_x, SDLK_z, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_r};

static int sockfd;
static struct sockaddr_in serverAddr;

spriteSheet* pictures = NULL;
SDL_Texture *lolyoudied, *plzcomeback;

int tagcount = 0;
char* tags = NULL;

static void paint(){
	SDL_RenderPresent(render);
}

static unsigned char teamReds[4]   = {0xff, 0xff,    0, 0xff};
static unsigned char teamGreens[4] = {0xff,    0,    0, 0xff};
static unsigned char teamBlues[4]  = {0xff,    0, 0xff,    0};

static void teamColor(unsigned char faction){
	SDL_SetRenderDrawColor(render, teamReds[faction], teamGreens[faction], teamBlues[faction], 255);
}

static void drawRadarBlip(const SDL_Rect *rect, unsigned char data) {
	unsigned char transponder = data & 0x30;
	unsigned char faction = data & 0x03;
	unsigned char r = teamReds[faction];
	unsigned char g = teamGreens[faction];
	unsigned char b = teamBlues[faction];
	if (transponder == 0x30) { // TM_NONE is dimmed
		r /= 2;
		g /= 2;
		b /= 2;
	} else if (transponder == 0x10) { // TM_MINE is faded close to asteroid color
		r |= 0x80;
		g |= 0x80;
		b |= 0x80;
	} else if (transponder == 0x20) { // TM_FEED is a checker of default and faded
		SDL_SetRenderDrawColor(render, r, g, b, 255);
		SDL_RenderDrawPoint(render, rect->x, rect->y);
		SDL_RenderDrawPoint(render, rect->x+1, rect->y+1);
		r |= 0x80;
		g |= 0x80;
		b |= 0x80;
		SDL_SetRenderDrawColor(render, r, g, b, 255);
		SDL_RenderDrawPoint(render, rect->x+1, rect->y);
		SDL_RenderDrawPoint(render, rect->x, rect->y+1);
		return;
	} // TM_DFND is default color*/
	SDL_SetRenderDrawColor(render, r, g, b, 255);
	SDL_RenderFillRect(render, rect);
}

static void drawRadar(int8_t* data, int len){
	SDL_SetRenderTarget(render, minimapTex);
	SDL_SetRenderDrawColor(render, 0, 0, 0, 0);
	SDL_RenderClear(render);
	SDL_Rect rect = {.x = 0, .y = 126, .w = 126, .h = 40};
	SDL_SetRenderDrawColor(render, 50, 50, 50, 0xFF);
	SDL_RenderFillRect(render, &rect);
	SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
	static char *modeStrings[4] = {"DFND", "MINE", "FEED", "NONE"};
	static char statusString[42];
	sprintf(statusString, "MINERALS:\n%13"PRId64"\nTRANSPONDER:\n%s", *(int64_t*)(data+2), modeStrings[(data[10]&12)/4]);
	drawText(render, 2, 126+2, 1, statusString);
	rect.w = rect.h = 2;
	SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
	
	tagcount = data[10];
	tags = realloc(tags, 4*tagcount);
	memcpy(tags, &(data[11]), 4*tagcount);
	int radarstart = 10+1+tagcount*4;

	char regularSize = 0;
	if (data[radarstart+1] & 128) {
		regularSize = 1;
		data[radarstart+1] &= 127;
	}
	if(data[radarstart]&128){
		int x = (data[0]&127)*2;
		SDL_RenderDrawLine(render, x, 0, x, 125);
		x++;
		SDL_RenderDrawLine(render, x, 0, x, 125);
	}
	if(data[radarstart]&64){
		int y = data[1]*2;
		SDL_RenderDrawLine(render, 0, y, 125, y);
		y++;
		SDL_RenderDrawLine(render, 0, y, 125, y);
	}
	int i;
	for(i = radarstart; i+2 < len; i += 3){
		rect.x = data[i+1]*2;
		rect.y = data[i+2]*2;
		drawRadarBlip(&rect, data[i]);
	}
	if (regularSize) { // This means it's a regular-sized radar scan, so we're definitely in the middle.
		rect.x = rect.y = 62;
		SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
		SDL_RenderFillRect(render, &rect);
	}

	SDL_SetRenderTarget(render, NULL);
}

/*static int getStarN(int x){
	int ret = 0;
	int i = 0;
	for(; i < 11; i++){
		ret = ret << 1;
		if(x&1) ret+=1;
		x = x >> 1;
	}
	return ret;
}*/

static double halton(int n, int p){
	double ret = 0;
	int dasNum = p;
	int dasOtherNum = 1;
	double dasNumAgain;
	while(dasOtherNum < 4096){
		dasNumAgain = 1.0/dasNum;
		while(n%dasNum){
			ret += dasNumAgain;
			n -= dasOtherNum;
		}
		dasOtherNum = dasNum;
		dasNum *= p;
	}
	return ret;
}

static SDL_Point starPoints[1000];

static void drawStars(int X, int Y){ // The stars are generated using the Halton sequence, which is a low-discrepancy sequence. If the purpose of the below code is hard to understand, that's why. Wikipedia it if you really care.
	int x, n, y;
	double z;
	for (n = 0; n < 1000; n++) {
		x = 4096*halton(n, 2)-X;
		y = 4096*halton(n, 3)-Y;
		z = 1+1.82842*halton(n, 5);
		z *= z;
		if (x >= 2048) x -= 4096;
		else if (x < -2048) x += 4096;
		if (y >= 2048) y -= 4096;
		else if (y < -2048) y += 4096;
		starPoints[n].x = 250*SCREEN_MULTIPLE+x*SCREEN_MULTIPLE/z;
		starPoints[n].y = 250*SCREEN_MULTIPLE+y*SCREEN_MULTIPLE/z;
	}
	SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
	SDL_RenderDrawPoints(render, starPoints, 1000);
}

static void handleNetwork(){
	static int8_t data[6000];
	struct sockaddr_in addr;
	socklen_t addrLen = sizeof(addr);
	int len;
	SDL_Rect rect;
	while(0<(len = recvfrom(sockfd, (char*)data, 6000, 0, (struct sockaddr*)&addr, &addrLen))){
		addrLen = sizeof(addr);
		if(addr.sin_addr.s_addr != serverAddr.sin_addr.s_addr) continue;
		if(*data & 0x80){
			drawRadar(data, len);
			continue;
		}
		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
		rect.x = rect.y = 0;
		rect.w = width*SCREEN_MULTIPLE;
		rect.h = (height+20)*SCREEN_MULTIPLE;
		SDL_RenderFillRect(render, &rect);
		int i = 0;
		if(*data == 0x41){
			i = 1;
			rect.x = (width/2-100)*SCREEN_MULTIPLE;
			rect.y = (height/2-50)*SCREEN_MULTIPLE;
			rect.w = 200*SCREEN_MULTIPLE;
			rect.h = 100*SCREEN_MULTIPLE;
			SDL_RenderCopy(render, lolyoudied, NULL, &rect);
			if (len == 1) { // Then clear the minimap too and paint
				rect.x = width*SCREEN_MULTIPLE + 1*SIDEBAR_MULTIPLE;
				rect.y = 1*SIDEBAR_MULTIPLE;
				rect.w = 128*SIDEBAR_MULTIPLE;
				rect.h = 128*SIDEBAR_MULTIPLE;
				SDL_RenderCopy(render, plzcomeback, NULL, &rect);
				paint();
				continue;
			}
		}
		int16_t bgx = *(int16_t*)(data+i+1);
		int16_t bgy = *(int16_t*)(data+i+3);
		drawStars(bgx, bgy);
		i += 7;
		while(i+6 < len){
			//unsigned char theta = 0x0F & data[i];
			//char flame = 0x10 & data[i];
			unsigned char sprite = (0x1F & data[i]) ^ 0x10;
			char faction = (0xE0 & data[i])/0x20;
			int ship = (uint8_t)data[++i];
			//if(faction == 1 && ship == 2) ship = 14;
			if (ship == 11 || ship == 13) ship = 0; // Freeze tag and creature players look like regular human ships
#ifdef PAUL
			if (faction == 1 && ship == 7) ship = 11;
			if (faction == 2 && ship == 2) ship = 13;
#endif
			int size = rect.w = rect.h = pictures[ship].size*SCREEN_MULTIPLE;
			int x = *(int16_t*)(data+(++i))*SCREEN_MULTIPLE;
			int y = *(int16_t*)(data+(i+=2))*SCREEN_MULTIPLE;
			rect.x =  width*SCREEN_MULTIPLE/2-size/2+x;
			rect.y = height*SCREEN_MULTIPLE/2-size/2+y;
			SDL_RenderCopy(render, pictures[ship].data[sprite], NULL, &rect);
			teamColor(faction);
			if(data[i+=2] & 0x20){ // Then we're drawing a target lock!!
				int index = abs(x)>abs(y) ? i-4 : i-2;
				if(abs(*(int16_t*)(data+index)) > width/2){
					double frac = (double)(width/2) / abs(*(int16_t*)(data+index));
					size *= frac;
					rect.x =  width*SCREEN_MULTIPLE/2-size/2+x*frac;
					rect.y = height*SCREEN_MULTIPLE/2-size/2+y*frac;
					rect.w = rect.h = size;
					size /= frac;
				}
				SDL_RenderDrawRect(render, &rect);
				rect.x = width*SCREEN_MULTIPLE/2-size/2+x;
				rect.w = size;
			}
			rect.y = height*SCREEN_MULTIPLE/2+size/2+y+2*SCREEN_MULTIPLE;
			rect.h = 3*SCREEN_MULTIPLE;
			rect.w = rect.w*(data[i]&0x1F)/31;
			SDL_RenderFillRect(render, &rect);
			x += width*SCREEN_MULTIPLE/2;
			y += height*SCREEN_MULTIPLE/2;
			while((data[i]&0x80) && i+3<len){
				int dx = ((uint8_t*)data)[i+1]*SCREEN_MULTIPLE;
				int dy = ((uint8_t*)data)[i+2]*SCREEN_MULTIPLE;
				if(data[i+3] & 0x40) dx-=256*SCREEN_MULTIPLE;
				if(data[i+3] & 0x20) dy-=256*SCREEN_MULTIPLE;
				addTrail(x, y, x+dx, y+dy, data[i+3]&0x1F);
				i+=3;
			}
			i++;
		}
		drawTrails(render);
		//Draw tags
		char t[4] = {0};
		SDL_SetRenderDrawColor(render, 255,255,255, 255);
		for(int tagidx = 0; tagidx < tagcount; tagidx++){
			memcpy(t, &(tags[tagidx*4]), 3);
			float angle = M_PI/127.0*(float)(tags[tagidx*4+3]);
			int x = width/2+width/4*cos(angle);
			int y = height/2+height/4*sin(angle);
			drawText(render, x, y, 1, t);
			//printf("%s\n", t);
		}

		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
		rect.y = height*SCREEN_MULTIPLE;
		rect.h = 20*SCREEN_MULTIPLE;
		rect.x = 0;
		rect.w = width*SCREEN_MULTIPLE;
		SDL_RenderFillRect(render, &rect);
		//Draw health and energy
		teamColor(data[0]);
		rect.x = width*SCREEN_MULTIPLE/2-width*SCREEN_MULTIPLE/2*(uint8_t)data[5]/255;
		rect.w = width*SCREEN_MULTIPLE*(uint8_t)data[5]/255;
		rect.h = 10*SCREEN_MULTIPLE;
		SDL_RenderFillRect(render, &rect);
		SDL_SetRenderDrawColor(render, 0, 0, 255, 255);
		rect.y += 10*SCREEN_MULTIPLE;
		rect.x = width*SCREEN_MULTIPLE/2-width*SCREEN_MULTIPLE/2*(uint8_t)data[6]/255;
		rect.w = width*SCREEN_MULTIPLE*(uint8_t)data[6]/255;
		SDL_RenderFillRect(render, &rect);
		//Draw sidebar grey
		SDL_SetRenderDrawColor(render, 50, 50, 50, 255);
		rect.x = width*SCREEN_MULTIPLE;
		rect.y = 0;
		rect.w = (126+2)*SIDEBAR_MULTIPLE;
		rect.h = (height+20)*SCREEN_MULTIPLE;
		SDL_RenderFillRect(render, &rect);

		rect.x = (width)*SCREEN_MULTIPLE + 1*SIDEBAR_MULTIPLE;
		rect.y = 1*SIDEBAR_MULTIPLE;
		rect.w = 126*SIDEBAR_MULTIPLE;
		rect.h = (126+40)*SIDEBAR_MULTIPLE;
		SDL_RenderCopy(render, minimapTex, NULL, &rect);



		paint();
	}
}

static void spKeyAction(int bit, char pressed){
	int i;
	unsigned char send;
	for (i = 0; i < numCommands && bit!=commandBindings[i]; i++);
	if (i < numCommands) {
		if (!pressed) return;
		send = 0x80 + i;
	} else {
		if (bit == thrustKey && pressed) {
			bit = keyBindings[2];
			pressed = !(keys & twoPow[2]);
		}
		for(i = 0; i < numKeys && bit!=keyBindings[i]; i++);
		if(i==numKeys) return;
		send = keys;
		if(pressed) send |= twoPow[i];
		else send &= 0xFF-twoPow[i];
		if(send == keys) return;
		keys = send;
	}
	sendto(sockfd, &send, 1, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
}

int main(int argc, char** argv){
	if(argc != 4){
		puts("USAGE: ./run <ip> <3-character tag> <shiptype>");
		return 5;
	}
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	window = SDL_CreateWindow(
		"Ship Game",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width*SCREEN_MULTIPLE + (126+2)*SIDEBAR_MULTIPLE,
		(height+20)*SCREEN_MULTIPLE,
		0/*SDL_WINDOW_FULLSCREEN*/
	);
	if(window == NULL){
		fputs("No SDL2 window.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_Quit();
		return 1;
	}
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	minimapTex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 126, 126+40);

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

	char tmp[20] = {']', '~', '~', '~', '\0'};
	for(int tagidx = strnlen(argv[2], 3)-1; tagidx >= 0; tagidx--){//copy the tag characters into the buffer
		tmp[1+tagidx] = argv[2][tagidx];
	}
	strncpy(&(tmp[4]), argv[3], 10);//kind of arbitrary length limiting
	sendto(sockfd, tmp, 20, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
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
	//if (lastExists)
		//pthread_join(lastId, NULL);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
