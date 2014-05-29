struct module;

typedef struct entity{
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

typedef struct module{
	void (*initFunc)(entity* who, int ix, double value);
	void (*actFunc)(entity* who, int ix, double energy);
	void (*cleanupFunc)(entity* who, int ix);
}module;

extern entity* newEntity(int type, double x, double y);

extern void tick(entity* who);

extern void drawEntity(entity* who, double x, double y, double zoom);

extern void thrust(entity* who, double amt);

extern void freeEntity(entity* who);
