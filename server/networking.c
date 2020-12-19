#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include "globals.h"

#include "networking.h"

client* clientList = NULL;

loadRequest *firstLoadRequest = NULL;

static int sockfd;

static int32_t divide(int64_t a, int32_t b){
	if(a<0) a-=(b-1);
	return a/b;
}

static int32_t mod(int64_t a, int32_t b){
	int32_t result = a%b;
	if(result<0) return result+b;
	return result;
}

// -120 / 40 Hz = 3 seconds until respawn
#define CANSPAWN (-120)

#define NETLEN 6000
static int8_t data[NETLEN];

#define LG_GRID_SIZE (-POS_MIN/RADAR_GRID_RADIUS)
#define SM_GRID_SIZE (RADAR_RANGE/RADAR_GRID_RADIUS)
#define RADAR_GRID_RADIUS 32
#define RADAR_GRID_WIDTH (RADAR_GRID_RADIUS*2-1)

static int transform(int32_t x, int32_t gridSize) {
	int32_t ret = divide(x+gridSize/2, gridSize);
	if (ret >= RADAR_GRID_RADIUS || ret <= -RADAR_GRID_RADIUS) return -1;
	return ret + RADAR_GRID_RADIUS - 1;
}

static void sendRadar(client* cli, char large){
	int dataLen = 10;
	entity* who = cli->myShip;
	if (!large && who->destroyFlag == 0) {// Because ghost ships don't behave w/ linkNear. The problem is that linkNear assumes the given ship is actually in the sector. I recognize this means you can't see out-of-sector things when dead; fix it if'n you dare.
		linkNear(who, RADAR_RANGE);
	}
	entity* runner = who->mySector->firstentity;
	int32_t x, y;
	char tagcount = 0;
	char tags[80];//20 sets of 4 bytes. First three bytes are the tag characters, last byte is angle/255
	uint8_t *radar = calloc(RADAR_GRID_WIDTH, RADAR_GRID_WIDTH);
	while(runner){
		if (runner == who) {
			runner = runner->next;
			continue;
		}
		if (large) {
			x = transform(runner->me->pos[0], LG_GRID_SIZE);
		} else {
			x = transform(displacementX(who, runner), SM_GRID_SIZE);
		}
		if(x == -1) {
			runner = runner->next;
			continue;
		}
		if (large) {
			y = transform(runner->me->pos[1], LG_GRID_SIZE);
		} else {
			y = transform(displacementY(who, runner), SM_GRID_SIZE);
		}
		if(y == -1) {
			runner = runner->next;
			continue;
		}
		if(runner->myAi == &aiHuman){
			memcpy(&(tags[4*tagcount]), ((humanAiData*)(runner->aiFuncData))->owner->tag, 3);
			tags[4*tagcount+3] = (char)(127.0*atan2f(displacementY(who, runner), displacementX(who, runner))/M_PI);
			tagcount++;
		}
		x += y * RADAR_GRID_WIDTH;
		radar[x] = 128+runner->faction;
		if (runner->faction == who->faction) radar[x] |= runner->transponderMode<<4;
		runner = runner->next;
	}
	if(dataLen+1+tagcount*4 > NETLEN){
		printf("Too many tags. Excluding\n");
		tagcount = 0;
	}
	data[dataLen] = tagcount;
	dataLen++;
	memcpy(&(data[dataLen]), tags, 4*tagcount);
	dataLen+=4*tagcount;

	int radarstart = dataLen;
	uint8_t *r2 = radar;
	for (y = 0; y < RADAR_GRID_WIDTH; y++) {
		for (x = RADAR_GRID_WIDTH-1; x >= 0; x--) {
			if (dataLen + 3 > NETLEN) {
				puts("Dear Lord, even the radar packet is too big!");
				break;
			}
			if (radar[x] == 0) continue;
			data[dataLen] = radar[x] & 127;
			data[dataLen+1] = x;
			data[dataLen+2] = y;
			dataLen += 3;
		}
		if (x >= 0) break;
		radar += RADAR_GRID_WIDTH;
	}
	free(r2);
	if (who->destroyFlag == 0)
		unlinkNear();
	if (dataLen-radarstart < 2) { //This will only happen if the radar is empty, but we've got to be sure.
		dataLen = radarstart+2;
		data[radarstart] = 0;
		data[radarstart+1] = 0;
	}
	data[radarstart] += who->transponderMode<<2;
	if (large) {
		x = transform(who->me->pos[0], LG_GRID_SIZE);
		y = transform(who->me->pos[1], LG_GRID_SIZE);
	} else {
		data[radarstart+1] += 128; // From Hell's heart I stab at thee
		if(who->me->pos[0] < 0) {
			x = transform(-who->me->pos[0]+POS_MIN, SM_GRID_SIZE);
		} else {
			x = transform(-who->me->pos[0]+POS_MAX, SM_GRID_SIZE);
		}
		if(who->me->pos[1] < 0) {
			y = transform(-who->me->pos[1]+POS_MIN, SM_GRID_SIZE);
		} else {
			y = transform(-who->me->pos[1]+POS_MAX, SM_GRID_SIZE);
		}
	}
	if (-1 != x) {
		data[radarstart] += 128;
		data[0] = x;
	}
	if (-1 != y) {
		data[radarstart] += 64;
		data[1] = y;
	}
	memcpy(data+2, &who->minerals, 8);
	data[0] |= 0x80; // It's a radar packet
	struct sockaddr_in sendAddr = {.sin_family=AF_INET, .sin_port=htons(3334), .sin_addr={.s_addr=cli->addr.sin_addr.s_addr}};
	if(dataLen > NETLEN) puts("Radar packet too large, not sending!");
	else sendto(sockfd, data, dataLen, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
}

void sendInfo(){
	static char counter = 0;
	if(++counter == 10) counter = 0;
	struct sockaddr_in sendAddr = {.sin_family=AF_INET, .sin_port=htons(3334)};
	int dataLen;
	int64_t d;
	client* conductor = clientList;
	while(conductor){
		dataLen = 0;
		if (conductor->spawnBase && conductor->spawnBase->destroyFlag) conductor->spawnBase = NULL;
		if (conductor->requestsSpawn) {
			entity *newShip;

			if (conductor->spawnBase) newShip = loadshipSpawner(conductor->shiptype, conductor->spawnBase);
			else newShip = loadship(conductor->shiptype);
			// Handle failures gracefully
			if (newShip != NULL) {
				conductor->myShip = newShip;
				puts("setting owner");
				((humanAiData*)newShip->aiFuncData)->owner = conductor;
			}
			conductor->requestsSpawn = 0;
		}
		entity* me = conductor->myShip;
		if(me->destroyFlag){
			if (me->destroyFlag > 0) {
				memcpy(&conductor->ghostShip, me, sizeof(entity)); // We don't care about all the modules, pointers, etc.
				conductor->ghostShip.me = &conductor->ghostGuarantee;
				conductor->ghostShip.destroyFlag = -1;
				me = conductor->myShip = &conductor->ghostShip;
			} else if (me->destroyFlag > CANSPAWN) {
				me->destroyFlag--;
				if (me->destroyFlag == CANSPAWN)
					disappear(me->mySector->x, me->mySector->y);
			}
		}
		if(counter == 0 && me->destroyFlag != CANSPAWN) {
			humanAiData *data = (humanAiData*)me->aiFuncData;
			if (data->largeRadar == 2) {
				sendRadar(conductor, 1);
				data->largeRadar = 3;
			} else if (data->largeRadar != 3) {
				sendRadar(conductor, 0);
			}
		}
		if (me->destroyFlag) {
			*data = 0x41; // Lol, you died
			if (me->destroyFlag == CANSPAWN) {
				sendAddr.sin_addr.s_addr = conductor->addr.sin_addr.s_addr;
				sendto(sockfd, (char*)data, 1, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
				conductor = conductor->next;
				continue;
			} else {
				dataLen = 1;
			}
		} else {
			linkNear(me, LOCK_RANGE);
		}
		sector *sec = me->mySector;
		entity *runner = sec->firstentity;
		data[dataLen] = me->faction;
		*(int16_t*)(data+dataLen+1) = mod(divide(me->me->pos[0], 64*16), 4096);
		*(int16_t*)(data+dataLen+3) = mod(divide(me->me->pos[1], 64*16), 4096);
		data[dataLen+5] = me->shield*255/me->maxShield;
		data[dataLen+6] = me->energy*255/me->maxEnergy;
		dataLen += 7;
		while(runner){
			if (dataLen + 7 > NETLEN) {
				puts("Nope, too long!");
				break;
			}
			data[dataLen+0] = 0x01*runner->theta+0x10*runner->thrustFlag+0x20*runner->faction;
			data[dataLen+1] = runner->type;
			d = divide(displacementX(conductor->myShip, runner)+32*16, 64*16);
		#define NET_VIEW_RANGE ((LOCK_RANGE/(64*16))+1)
			if(d < -NET_VIEW_RANGE || d > NET_VIEW_RANGE){
				runner = runner->next;
				continue;
			}
			*(int16_t*)(data+dataLen+2) = d;
			d = divide(displacementY(conductor->myShip, runner)+32*16, 64*16);
			if(d < -NET_VIEW_RANGE || d > NET_VIEW_RANGE){
				runner = runner->next;
				continue;
			}
			*(int16_t*)(data+dataLen+4) = d;
			if (runner->shield <= 0)
				data[dataLen+6] = 0;
			else if (runner->shield > runner->maxShield)
				data[dataLen+6] = 31;
			else
				data[dataLen+6] = runner->shield*31/runner->maxShield;
			if(runner == conductor->myShip->targetLock){
				 data[dataLen+6] |= 0x20;
			}
			dataLen+=7;
			for (int count = 0; count<runner->numTrails; count++){
				if (dataLen + 3 > NETLEN) {
					puts("Just too long.");
					break;
				}
				data[dataLen-1] |= 0x80;
				data[dataLen+2] = runner->trailTypes[count];
				d = divide(runner->trailXs[count]+32*16, 64*16);
				if(d<0){
					d+=256;
					data[dataLen+2] |= 0x40;
				}
				data[dataLen] = d;
				d = divide(runner->trailYs[count]+32*16, 64*16);
				if(d<0){
					d+=256;
					data[dataLen+2] |= 0x20;
				}
				data[dataLen+1] = d;
				dataLen+=3;
			}
			runner = runner->next;
		}
		if (me->destroyFlag == 0)
			unlinkNear();
		sendAddr.sin_addr.s_addr = conductor->addr.sin_addr.s_addr;
		if(dataLen > NETLEN) puts("Network packet too large, not sending!");
		else sendto(sockfd, (char*)data, dataLen, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
		conductor = conductor->next;
	}
}

//It's relatively important that this thread doesn't actually modify clientList. That's the other thread's job.
void* netListen(void* whoGivesADern){
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		puts("When in danger,\nOr in doubt,\nRun in circles!\nScream and shout!");
		return NULL;
	}
	struct sockaddr_in bindAddr = {.sin_family=AF_INET, .sin_addr={.s_addr=htonl(INADDR_ANY)}, .sin_port=htons(3333)};
	if(0 > bind(sockfd, (struct sockaddr*)&bindAddr, sizeof(bindAddr))){
		puts("When in danger,\nOr in doubt,\nRun in circles!\nScream and shout!!!");
		return NULL;
	}
	//Judging from loadship.c, MAXNAMELEN includes the null byte in the count.
	//The +1 here is for the ']' character
	char msg[20];
	socklen_t len;
	while(1){
		len = sizeof(bindAddr);
		int msgSize = recvfrom(sockfd, msg, 20, 0, (struct sockaddr*)&bindAddr, &len);
		client* current = clientList;
		while(current != NULL){
			if(current->addr.sin_addr.s_addr == bindAddr.sin_addr.s_addr){
				if (msgSize == 1) {
					if (current->myShip->destroyFlag == CANSPAWN && !current->requestsSpawn) {
						current->requestsSpawn = 1;
						printf("Player %s,%s has respawned!\n", current->tag,current->shiptype);
						break;
					} else if (current->myShip->destroyFlag) {
						break;
					}
					unsigned char m = *msg;
					humanAiData *data = (humanAiData*)current->myShip->aiFuncData;
					if (m & 0x80) {
						if (m == 0x80) data->getLock = 1;
						else if (m == 0x81) data->clearLock = 1;
						else if (m <= 0x85) data->setTM = m - 0x82;
						else if (m == 0x86) {
							if (data->largeRadar == 3) {
								data->largeRadar = 0;
							} else if (data->largeRadar == 0) {
								data->largeRadar = 1;
							}
						}
					} else {
						data->keys = m;
					}
				} else if (msgSize == 20 && *msg == ']') {
					current->tag[3] = 0;
					current->shiptype[MAXNAMELEN] = 0;
					memcpy(current->tag, msg+1, 3);
					memcpy(current->shiptype, msg+4, MAXNAMELEN);
					printf("Player updated %s, %s\n", current->tag, current->shiptype);
				} else {
					printf("Got malformed data.\n");
				}
				break;
			}
			current = current->next;
		}
		if(current == NULL){//That is, he isn't joined yet
			if(msgSize != 20 || *msg != ']') continue;//Our super-secret, "I'm a legitimate client" character
			client* new = malloc(sizeof(client));
			new->tag[3] = 0;
			new->shiptype[MAXNAMELEN] = 0;
			memcpy(new->tag, msg+1, 3);
			memcpy(new->shiptype, msg+4, MAXNAMELEN);
			new->addr = bindAddr;
			new->spawnBase = NULL;
			new->requestsSpawn = 0;
			loadRequest *req = malloc(sizeof(loadRequest));
			req->next = firstLoadRequest;
			req->cli = new;
			strcpy(req->name, new->shiptype);
			firstLoadRequest = req;
			printf("Player joined %s, %s\n", new->tag, new->shiptype);
		}
	}
	return NULL;
}
