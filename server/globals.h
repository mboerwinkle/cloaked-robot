extern int unloadsector(long long int x, long long int y);
extern void interpretsector(long long int x, long long int y);
extern int main();
extern void gensector(long long int x, long long int y);
extern void loadsector(long long int x, long long int y); 
extern int writesectortofile(long long int x, long long int y);

struct module;

typedef struct entity{
	struct entity *next;
	double x, y, vx, vy, r;
	double shield, maxShield;
	double energy, maxEnergy;
	double theta, omega;
	double sinTheta, cosTheta;
	void (*aiFunc)(struct entity*);
	void* aiFuncData;
	struct module** modules;
	void** moduleDatas;
	int numModules;
}entity;

typedef struct obj {
	struct obj *next;
	//other object data
} obj;
typedef struct sector {
	struct sector *nextsector;
	struct obj *firstobj;
	struct entity *firstentity;
	long long int x;
	long long int y;
} sector;
sector *listrootsector;

struct aiHumanData{
	int player;
};
extern void aiHuman(entity* who);
extern void aiMissile(entity* who);

typedef struct module{
	void (*initFunc)(entity* who, int ix, double value);
	void (*actFunc)(entity* who, int ix, double energy);
	void (*cleanupFunc)(entity* who, int ix);
}module;

extern entity* newEntity(int type, double x, double y);
extern void tick(entity* who);
//extern void drawEntity(entity* who, double x, double y, double zoom);
extern void thrust(entity* who, double amt);
extern void freeEntity(entity* who);

extern double zoom;
extern sector mySector;

extern void initField();
extern void stopField();
extern void addEntity(entity* who);
extern void run(sector *sec);
extern void draw();

extern module turnModule;
extern module thrustModule;
extern module missileModule;

extern void initModules();
