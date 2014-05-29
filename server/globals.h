extern int unloadsector(long long int x, long long int y);
extern void interpretsector(long long int x, long long int y);
extern int main();
extern void gensector(long long int x, long long int y);
extern void loadsector(long long int x, long long int y); 
extern int writesectortofile(long long int x, long long int y);

typedef struct obj {
	struct obj *next;
	//other object data
} obj;
typedef struct ship {
	struct ship *next;
	//other ship data
} ship;
typedef struct entity {
	struct entity *next;
	//other entity data
} entity;
typedef struct sector {
	struct sector *nextsector;
	struct obj *firstobj;
	struct ship *firstship;
	struct entity *firstentity;
	long long int x;
	long long int y;
} sector;
sector *listrootsector;
