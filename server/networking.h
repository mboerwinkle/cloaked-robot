#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct client {
	struct client* next;
	char keys[6];
	char name[20]; // For reloading after death
	struct sockaddr_in addr;
	entity ghostShip; // For when we're dead
	guarantee ghostGuarantee;
	entity *myShip;
	entity *spawnBase;
	char requestsSpawn;
} client;

typedef struct loadRequest{
	struct loadRequest *next;
	client *cli;
	char name[MAXNAMELEN];
} loadRequest;
extern loadRequest *firstLoadRequest;

extern client* clientList;
