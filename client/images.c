#include <SDL2/SDL.h>
#include <Imlib2.h>
#include "gui.h"

static void rotationarray(uint32_t *oldpic, uint32_t *newpic, double degrees, int size){
	int bound = (size-1)/2;
	short newx, newy, x, y;
	for(x=-bound; x<=bound; x++){
		for(y=-bound; y<=bound; y++){
			newx = floor(x*cos(degrees)-y*sin(degrees)+.5);
			newy = floor(x*sin(degrees)+y*cos(degrees)+.5);
			//newx = x*cos(degrees)-y*sin(degrees);
			//newy = x*sin(degrees)+y*cos(degrees);
			if(newx<-bound || newx>bound || newy<-bound || newy>bound) continue;
			newpic[size*(bound+y)+bound+x] = oldpic[size*(bound+newy)+bound+newx];
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

static void loadPic(char *addr){
	static int numPics = 0;
	pictures = realloc(pictures, sizeof(spriteSheet)*(numPics+1));
	Imlib_Image img = imlib_load_image(addr);
	if (img == NULL) {
		puts("Unable to load this picture:");
		puts(addr);
		exit(5);
	}
	imlib_context_set_image(img);
	int mySize = imlib_image_get_width();
	pictures[numPics].size = mySize;
	pictures[numPics].data = malloc(sizeof(SDL_Surface*)*2*16);
	uint32_t *data = calloc(2*16*mySize*mySize, sizeof(uint32_t));
	uint32_t *myData = imlib_image_get_data();
	int o = 0; // "o" is for offset
	for (; o <= 16*mySize*mySize; o += 16*mySize*mySize) {
		//The "*2" in the following line is because we actually want the first and _third_ sprites.
		rotationarray(myData+o/16*2, data+o, -M_PI/2, mySize);
		rotationarray(data+o, data+o+1*mySize*mySize, -1*M_PI/8, mySize);
		rotationarray(data+o, data+o+2*mySize*mySize, -2*M_PI/8, mySize);
		rotationarray(data+o, data+o+3*mySize*mySize, -3*M_PI/8, mySize);
		rotate(data+o, mySize);
	}
	imlib_free_image();
	int i = 0;
	for(; i<2*16; i++){
		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data+i*mySize*mySize, mySize, mySize, 32, mySize*4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
		SDL_SetTextureBlendMode(
			pictures[numPics].data[i] = SDL_CreateTextureFromSurface(render, surface),
			SDL_BLENDMODE_BLEND);
		SDL_FreeSurface(surface);
	}
	free(data);
	numPics++;
}

static SDL_Texture* loadImage(char* str){
	Imlib_Image img = imlib_load_image(str);
	imlib_context_set_image(img);
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(imlib_image_get_data(), imlib_image_get_width(), imlib_image_get_height(), 32, imlib_image_get_width()*4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
	SDL_Texture* ret = SDL_CreateTextureFromSurface(render, surface);
	SDL_FreeSurface(surface);
	imlib_free_image();
	return ret;
}

void loadPics(){
	char picturename[80];
	FILE *fp = fopen("mdls/contents", "r");
	while(fgets(picturename, 80, fp)){
		picturename[strlen(picturename)-1] = '\0';
		loadPic(picturename);
	}
	lolyoudied = loadImage("mdls/death.png");
	plzcomeback = loadImage("mdls/respawn.png");
	SDL_SetTextureBlendMode(lolyoudied, SDL_BLENDMODE_BLEND);
}
