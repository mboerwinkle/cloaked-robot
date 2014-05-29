typedef struct{
	entity** entities;
	int numEntities;
}sector;

extern double zoom;

extern sector mySector;

extern void initField();

extern void stopField();

extern void addEntity(entity* who);

extern void run();

extern void draw();
