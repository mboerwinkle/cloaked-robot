typedef struct trail{
	int x1, y1, x2, y2;
	int type;
	int life;
}trail;

extern void addTrail(int x1, int y1, int x2, int y2, int type);
extern void drawTrails(SDL_Renderer* render);
