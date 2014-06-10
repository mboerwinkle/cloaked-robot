#include <SDL2/SDL.h>
#include <Imlib2.h>
#include "gui.h"

#define R 0xFF0000FF
#define G 0x00FF00FF
#define B 0x0000FF00

static void rotationarray(uint32_t *oldpic, uint32_t *newpic, double degrees){
	short newx, newy, x, y;
	for(x=-10; x<=10; x++){
		for(y=-10; y<=10; y++){
			newx = floor(x*cos(degrees)-y*sin(degrees)+.5);
			newy = floor(x*sin(degrees)+y*cos(degrees)+.5);
			//newx = x*cos(degrees)-y*sin(degrees);
			//newy = x*sin(degrees)+y*cos(degrees);
			if(newx<-10 || newx>10 || newy<-10 || newy>10) continue;
			newpic[21*(10+y)+10+x] = oldpic[21*(10+newy)+10+newx];
		}
	}
}

static void rotate(uint32_t* oldData, int size){
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
	int mySize = 21;
	Imlib_Image img = imlib_load_image("mdls/ship1.png");
	imlib_context_set_image(img);
	pictures = malloc(sizeof(spriteSheet));
	pictures[0].size = mySize;
	pictures[0].data = malloc(sizeof(SDL_Surface*)*16);
	uint32_t *data = calloc(sizeof(uint32_t), 16*mySize*mySize);
/*	uint32_t myData[36] =  {G, G, G, G, B, R, G, G, G,
				G, G, G, G, B, R, G, G, R,
				G, G, G, G, B, G, G, G, R,
				G, G, G, G, B, G, G, R, R};*/
	uint32_t *myData = imlib_image_get_data();
	rotationarray(myData, data, -M_PI/2);
//	memcpy(data, myData, 4*mySize*mySize);
	imlib_free_image();
	rotationarray(data, data+1*mySize*mySize, -1*M_PI/8);
	rotationarray(data, data+2*mySize*mySize, -2*M_PI/8);
	rotationarray(data, data+3*mySize*mySize, -3*M_PI/8);
	rotate(data, mySize);
	int i = 0;
	for(; i<16; i++){
		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data+i*mySize*mySize, mySize, mySize, 32, mySize*4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
		SDL_SetTextureBlendMode(
			pictures[0].data[i] = SDL_CreateTextureFromSurface(render, surface),
			SDL_BLENDMODE_ADD);
		SDL_FreeSurface(surface);
	}
	free(data);
}
