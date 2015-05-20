
typedef struct client {
	struct client* next;
	char keys[6];
	char name[20]; // For reloading after death
	struct sockaddr_in addr;
	entity ghostShip; // For when we're dead
	guarantee ghostGuarantee;
	int32_t ghostPos[2];
	entity *myShip;
} client;

extern client* clientList;
