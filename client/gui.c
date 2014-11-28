#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
//#include <Imlib2.h>
//#include <pthread.h>
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
#define SCREEN_MULTIPLE 1

static SDL_Window* window;
SDL_Renderer* render;
static SDL_Texture* minimapTex;

static int running = 1;
static unsigned char keys = 0;
static unsigned char twoPow[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#define numKeys 8
static int keyBindings[numKeys] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_x, SDLK_z, SDLK_a, SDLK_s, SDLK_d};

static int sockfd;
static struct sockaddr_in serverAddr;

spriteSheet* pictures = NULL;
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
/*
struct myPthreadData {
	uint32_t *imgData;
	int number;
	char join;
	pthread_t prev;
};
char lastExists = 0;
pthread_t lastId;

static int radarCount = 0;
static void *saveRadarImg(void *arg)
{
	struct myPthreadData *data = (struct myPthreadData*)arg;
	Imlib_Image img = imlib_create_image_using_data(128, 128, data->imgData);
	imlib_context_set_image(img);
	char file[100];
	sprintf(file, "radar%.4d.png", data->number);
	imlib_save_image(file);
	imlib_free_image();
	free(data->imgData);
	if (data->join)
		pthread_join(data->prev, NULL);
	free(data);
	return NULL;
}*/

static void drawRadar(int8_t* data, int len){
	SDL_SetRenderTarget(render, minimapTex);
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	SDL_RenderClear(render);
	SDL_Rect rect = {.w = 2, .h = 2};
	SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
	if(data[2]&128){
		SDL_RenderDrawLine(render, data[0]&0x7F, 0, data[0]&0x7F, 128);
	}
	if(data[2]&64){
		SDL_RenderDrawLine(render, 0, data[1], 128, data[1]);
	}
	/*uint32_t *imgData = calloc(128*128, 4);
	uint32_t color;
	int ix;*/
	int i = 2;
	while(i+2 < len){
		teamColor(data[i]&0x3F);
		rect.x = data[i+1];
		rect.y = data[i+2];
		/*SDL_GetRenderDrawColor(render, ((uint8_t*)&color)+2, ((uint8_t*)&color)+1, ((uint8_t*)&color), ((uint8_t*)&color)+3);
		ix = rect.x + 128*rect.y;
		if (rect.y >= 0) {
			if (rect.x >= 0)
			imgData[ix] = color;
			if (rect.x < 127)
			imgData[ix+1] = color;
		}
		if (rect.y < 127) {
			if (rect.x >= 0)
			imgData[ix+128] = color;
			if (rect.x < 127)
			imgData[ix+129] = color;
		}*/
		SDL_RenderFillRect(render, &rect);
		i+=3;
	}
	rect.x = rect.y = 63;
	SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
	SDL_RenderFillRect(render, &rect);
	/*struct myPthreadData *pData = malloc(sizeof(struct myPthreadData));
	pData->imgData = imgData;
	pData->number = radarCount++;
	if (lastExists) {
		pData->join = 1;
		pData->prev = lastId;
	} else {
		pData->join = 0;
		lastExists = 1;
	}
	pthread_create(&lastId, NULL, saveRadarImg, pData);*/

	SDL_SetRenderTarget(render, NULL);
}

static int getStarN(int x){
	int ret = 0;
	int i = 0;
	for(; i < 12; i++){
		ret = ret << 1;
		if(x&1) ret+=1;
		x = x >> 1;
	}
	return ret;
}

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
	int numPoints = 0;
	for(x = X-500; x<X+500; x++){
		if(x&0xFFF){
			n = getStarN(x);
			y = 4096*halton(n, 3);
			z = 1+halton(n, 5);
		}else{
			y = 0;
			z = 1;
		}
		starPoints[numPoints].x = 250*SCREEN_MULTIPLE+(x-X)*SCREEN_MULTIPLE/z;
		y -= Y;
		if(y >= 2048) y -= 4096;
		else if( y < -2048) y+= 4096;
		starPoints[numPoints++].y = 250*SCREEN_MULTIPLE+y*SCREEN_MULTIPLE/z;
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
		if(*data & 0x40){ // A control packet
			if(*data == 0x41){
				rect.x = (width/2-100)*SCREEN_MULTIPLE;
				rect.y = (height/2-50)*SCREEN_MULTIPLE;
				rect.w = 200*SCREEN_MULTIPLE;
				rect.h = 100*SCREEN_MULTIPLE;
				SDL_RenderCopy(render, lolyoudied, NULL, &rect);
				paint();
				continue;
			}
		}
		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
		rect.x = rect.y = 0;
		rect.w = width*SCREEN_MULTIPLE;
		rect.h = (height+20)*SCREEN_MULTIPLE;
		SDL_RenderFillRect(render, &rect);
		int16_t bgx = *(int16_t*)(data+1);
		int16_t bgy = *(int16_t*)(data+3);
		drawStars(bgx, bgy);
		int i = 7;
		while(i+6 < len){
			//unsigned char theta = 0x0F & data[i];
			//char flame = 0x10 & data[i];
			unsigned char sprite = (0x1F & data[i]) ^ 0x10;
			char faction = (0xE0 & data[i])/0x20;
			int ship = (uint8_t)data[++i];
			//if(faction == 1 && ship == 2) ship = 14;
			int size = rect.w = rect.h = pictures[ship].size*SCREEN_MULTIPLE;
			int x = *(int16_t*)(data+(++i))*SCREEN_MULTIPLE;
			int y = *(int16_t*)(data+(i+=2))*SCREEN_MULTIPLE;
			rect.x =  width*SCREEN_MULTIPLE/2-size/2+x;
			rect.y = height*SCREEN_MULTIPLE/2-size/2+y;
			SDL_RenderCopy(render, pictures[ship].data[sprite], NULL, &rect);
			if(data[i+=2] & 0x20){
				SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
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
			teamColor(faction);
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
		SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
		rect.y = height*SCREEN_MULTIPLE;
		rect.h = 20*SCREEN_MULTIPLE;
		rect.x = 0;
		rect.w = width*SCREEN_MULTIPLE;
		SDL_RenderFillRect(render, &rect);	
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

		SDL_SetRenderDrawColor(render, 50, 50, 50, 255);
		rect.x = width*SCREEN_MULTIPLE;
		rect.y = 0;
		rect.w = 130*SCREEN_MULTIPLE;
		rect.h = (height+20)*SCREEN_MULTIPLE;
		SDL_RenderFillRect(render, &rect);

		rect.x = (width+1)*SCREEN_MULTIPLE;
		rect.y = 1*SCREEN_MULTIPLE;
		rect.w = 128*SCREEN_MULTIPLE;
		rect.h = 128*SCREEN_MULTIPLE;
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
	window = SDL_CreateWindow("Ship Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (width+130)*SCREEN_MULTIPLE, (height+20)*SCREEN_MULTIPLE, 0);
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
	//if (lastExists)
		//pthread_join(lastId, NULL);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
