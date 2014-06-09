#define MAXNAMELEN 10
#define NUMKEYS 5

struct entity;
struct module;
struct sector;

extern void interpretsector(long long int x, long long int y);
extern int main();
extern void gensector(long long int x, long long int y);
extern void loadsector(long long int x, long long int y); 
extern int writesectortofile(long long int x, long long int y);
extern int mkship(char *name);
extern struct entity* loadship(char *name);
extern void move(long long int xorigin, long long int yorigin, long long int xfinal, long long int yfinal);
extern void appear(long long int x, long long int y);
extern void disappear(long long int x, long long int y);

typedef struct entity{
	struct sector *mySector;
	struct entity *next;
	double vx, vy, r;
	long int x, y;

	double shield, maxShield;
	double energy, maxEnergy;
	int turn, maxTurn;
	double thrust;

	char theta;
	double sinTheta, cosTheta;
	void (*aiFunc)(struct entity*);
	void* aiFuncData;
	struct module** modules;
	void** moduleDatas;
	int numModules;
}entity;

#define displacementX(a,b) ((((long long int)ULONG_MAX)+1)*(b->mySector->x - a->mySector->x) + b->x - a->x)
#define displacementY(a,b) ((((long long int)ULONG_MAX)+1)*(b->mySector->y - a->mySector->y) + b->y - a->y)

typedef struct obj {
	struct obj *next;
	//other object data
} obj;
typedef struct sector {
	short number; //number of clients who need it open
	struct sector *nextsector;
	struct obj *firstobj;
	struct entity *firstentity;
	long long int x;
	long long int y;
} sector;

extern sector *searchforsector(long long int x, long long int y);
extern int unloadsector(sector *target);

sector *listrootsector;

extern void aiHuman(entity* who);
extern void aiMissile(entity* who);

typedef struct module{
	void (*initFunc)(entity* who, int ix, double value);
	void (*actFunc)(entity* who, int ix, double energy, char action);
	void (*cleanupFunc)(entity* who, int ix);
}module;

extern entity* newEntity(int type, long int x, long int y);
extern char tick(entity* who);
//extern void drawEntity(entity* who, double x, double y, double zoom);
extern void thrust(entity* who);
extern void turn(entity* who, char dir);
extern void freeEntity(entity* who);

extern double zoom;
extern sector mySector;

extern void initField();
extern void stopField();
extern void addEntity(entity* who);
extern void fileMoveRequest(entity* who, sector* from, sector* to);
extern void run(sector *sec);

extern module missileModule;

extern void initModules();

extern void sendInfo();
extern void* netListen(void* arg);
