#include <SDL2/SDL.h>
#include "fontData.h"

void drawText(SDL_Renderer *render, int x, int y, float size, char* string){
	SDL_Rect r;
	int j = -1;
	int i = -1;
	int y2;
	int x2;
	Uint8 row;
	Uint8 test;
	float tempx, tempy;
	while(string[++j] != '\0'){
		i++;
		if(string[j] == '\n'){
			i = -1;
			y+=size*9;
			continue;
		}
		tempy = y;
		for(y2 = 0; y2<8; y2++){
			row = gfxPrimitivesFontdata[8*string[j]+y2];
			test = 128;
			tempx = x+i*9*size;
			for(x2 = 0; x2<8; x2++){
				tempx+=size;
				if(!(row & test)){
					test/=2;
					continue;
				}
				r.x = (int)(tempx-size);
				r.y = (int)tempy;
				r.w = (int)tempx-r.x;
				r.h = (int)(tempy+size)-r.y;
				SDL_RenderFillRect(render, &r);
				test/=2;
			}
			tempy += size;
		}
	}
}
