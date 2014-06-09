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

#ifdef WINDOWS
#include <windows.h>
#endif

#define width 500
#define height 500

static SDL_Window* window;
static SDL_Renderer* render;

static int running = 1;
static unsigned char keys = 0;
static unsigned char twoPow[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static int keyBindings[3] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP};

static int sockfd;
static struct sockaddr_in serverAddr;

typedef struct{
	SDL_Texture** data;
	int size;
}spriteSheet;
spriteSheet* pictures;

#define R 0xFF0000FF
#define G 0x00FF00FF
#define B 0x0000FFFF

void rotationarray(uint32_t oldpic[21][21],uint32_t newpic[21][21], short degrees){
	short newx, newy, x, y;
	for(x=-10; x<=10; x++){
		for(y=-10; y<=10; y++){
			newx = x*cos(degrees)-y*sin(degrees);
			newy = x*sin(degrees)+y*cos(degrees);
			newpic[10+newx][10+newy] = oldpic[10+x][10+y];
		}
	}
}

void rotate(uint32_t* oldData, int size){
	int area = size*size*4;
	int dim = size-1;
	int k = 0, i, j;
	uint32_t point;
	uint32_t *data;
	for(; k<4; k++){
		data = oldData+k*size*size;
		for(i=0; i<size; i++){
			for(j=0; j<size; j++){
				point = data[i+size*j];
				data[1*area+(dim-j)+size*   i   ] = point;
				data[2*area+(dim-i)+size*(dim-j)] = point;
				data[3*area+   j   +size*(dim-i)] = point;
			}
		}
	}
}

void loadPics(){
	pictures = malloc(sizeof(spriteSheet));
	pictures[0].size = 3;
	pictures[0].data = malloc(sizeof(SDL_Surface*)*16);
	uint32_t *data = malloc(sizeof(uint32_t)*16*9);
	uint32_t myData[36] =  {G, G, G, G, B, R, G, G, G,
				G, G, G, G, B, R, G, G, R,
				G, G, G, G, B, G, G, G, R,
				G, G, G, G, B, G, G, R, R};
	memcpy(data, myData, sizeof(uint32_t)*4*9);
	rotate(data, 3);
	int i = 0;
	for(; i<16; i++){
		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data+i*9, 3, 3, 32, 12, 0xFF000000, 0x00FF0000, 0x0000FF00, 0xFF);
		pictures[0].data[i] = SDL_CreateTextureFromSurface(render, surface);
		SDL_FreeSurface(surface);
	}
	free(data);
}

void paint(){
	SDL_RenderPresent(render);
	SDL_RenderClear(render);
}

void handleNetwork(){
	static int16_t data[3*100];
	struct sockaddr_in addr;
	socklen_t addrLen = sizeof(addr);
	int len;
	SDL_Rect rect;
	while(0<(len = recvfrom(sockfd, (char*)data, 600, 0, (struct sockaddr*)&addr, &addrLen))){
		addrLen = sizeof(addr);
		if(addr.sin_addr.s_addr != serverAddr.sin_addr.s_addr) continue;
		len/=2;
		int i = 0;
		while(i+2 < len){
			unsigned char theta = 0x0F & data[i];
//			char flame = 0x10 & data[i];
//			char faction = (0x60 & data[i])/0x20;
			int ship = (0xFF80 & data[i]) / 0x80;
			int size = rect.w = rect.h = pictures[ship].size;
			rect.x =  width/2-size/2+data[++i];
			rect.y = height/2-size/2+data[++i];
			SDL_RenderCopy(render, pictures[ship].data[theta], NULL, &rect);
			i++;
		}
		paint();
	}
}

static void spKeyAction(int bit, char pressed){
	int i = 0;
	for(; i < 3 && bit!=keyBindings[i]; i++);
	if(i==3) return;
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
	window = SDL_CreateWindow("Ship Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
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
