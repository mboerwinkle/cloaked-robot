#include <stdlib.h>
#include <stdio.h>
#include <math.h>
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
static SDL_Texture* texture;
static SDL_Renderer* render;

static int running = 1;

static int sockfd;
static struct sockaddr_in serverAddr;

void paint(){
	static int x = width/2-6;
	uint32_t picture[9] = {0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0xFF0000FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF};
	SDL_Rect r = {x+=5, .y=height/2-1, .w=3, .h=3};
	SDL_UpdateTexture(texture, &r, picture, 12);
	SDL_RenderCopy(render, texture, NULL, NULL);
	SDL_RenderPresent(render);
	SDL_RenderClear(render);
	SDL_DestroyTexture(texture);
	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	//TODO: There's got to be a better way to do the above. All I want to do is clear the texture.
}

void myDrawScreen(){
//	SDL_GL_SwapWindow(window);
//	glClear(GL_COLOR_BUFFER_BIT);
}

static void spKeyAction(int bit, char pressed){
	char* tmp = "]yo";
	sendto(sockfd, &tmp, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
}

int main(int argc, char** argv){
	if(argc < 2){
		puts("Please specify an ip.");
		return 5;
	}
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	window = SDL_CreateWindow("Bounce Brawl", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	if(window == NULL){
		fputs("No SDL2 window.\n", stderr);
		fputs(SDL_GetError(), stderr);
		SDL_Quit();
		return 1;
	}
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
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
	serverAddr.sin_port=htons(3333);
	inet_aton(argv[1], &serverAddr.sin_addr);

	while(running){
		SDL_Event evnt;
		SDL_WaitEvent(&evnt);
		if(evnt.type == SDL_QUIT)		running = 0;
		else if(evnt.type == SDL_KEYDOWN){
			paint();
			spKeyAction(evnt.key.keysym.sym, 1);
		}
		else if (evnt.type == SDL_KEYUP)	spKeyAction(evnt.key.keysym.sym, 0);
	}
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
