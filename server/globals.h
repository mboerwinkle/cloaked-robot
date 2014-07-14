#include <stdint.h>
#define MAXNAMELEN 10
#define POS_MAX 2097151
#define POS_MIN (-2097152)
#define LOCK_RANGE (64*1000)

struct entity;
struct module;
struct sector;

extern void interpretsector(uint64_t x, uint64_t y);
extern int main();
extern void gensector(uint64_t x, uint64_t y);
extern void loadsector(uint64_t x, uint64_t y); 
extern int mkship(char *name);
extern struct entity* loadship(char *name);
extern void move(uint64_t xorigin, uint64_t yorigin, uint64_t xfinal, uint64_t yfinal);
extern void appear(uint64_t x, uint64_t y);
extern void disappear(uint64_t x,uint64_t y);

typedef struct ai{
	void (*act)(struct entity*);
	void (*handleCollision)(struct entity*, struct entity*);
}ai;

typedef struct droneAiData{
	short timer;
	char Attack;
}droneAiData;

extern ai aiHuman;
extern ai aiMissile;
extern ai aiDrone;
extern void initAis();

typedef struct entity{
	char actedFlag;
	char destroyFlag;
	int type;
	struct sector *mySector;
	struct entity *next;
	double vx, vy, r;
	int32_t x, y;

	double shield, maxShield;
	double energy, maxEnergy;
	double energyRegen;
	int turn, maxTurn;
	double thrust;
	struct entity* targetLock;

	char theta;
	double sinTheta, cosTheta;
	ai* myAi;
	void* aiFuncData;
	struct module** modules;
	void** moduleDatas;
	int numModules;

	struct entity** trailTargets;
	int* trailTypes;
	int numTrails, maxTrails;
}entity;

#define displacementX(a,b) ((POS_MAX-POS_MIN+1)*(int64_t)(b->mySector->x - a->mySector->x) + b->x - a->x)
#define displacementY(a,b) ((POS_MAX-POS_MIN+1)*(int64_t)(b->mySector->y - a->mySector->y) + b->y - a->y)

typedef struct sector {
	short number; //number of clients who need it open
	struct sector *nextsector;
	struct entity *firstentity;
	uint64_t x;
	uint64_t y;
} sector;

extern sector *searchforsector(uint64_t x, uint64_t y);
extern int unloadsector(sector *target);
extern int writesectortofile(sector *target);

sector *listrootsector;

typedef struct module{
	void (*initFunc)(entity* who, int ix, double value);
	void (*actFunc)(entity* who, int ix, char action);
	void (*cleanupFunc)(entity* who, int ix);
}module;

extern entity* newEntity(int type, int aiType, sector *where, int32_t x, int32_t y);
extern void addTrail(entity* from, entity* to, char type);
extern void tick(entity* who);
extern char tick2(entity* who);
//extern void drawEntity(entity* who, double x, double y, double zoom);
extern void thrust(entity* who);
extern void turn(entity* who, char dir);
extern void freeEntity(entity* who);

extern void fileMoveRequest(entity* who, sector* from, sector* to);
extern void run(sector *sec);
extern void run2(sector *sec);
extern void cleanup();
extern char globalActedFlag;

extern module missileModule;

extern void initModules();

extern void sendInfo();
extern void* netListen(void* arg);

extern void linkNear(entity* who, int32_t radius);
extern void unlinkNear();
