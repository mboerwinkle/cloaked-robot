#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
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

static void sendRadar(client* cli){
	int dataLen = 2;
	entity* who = cli->myShip;
	if (who->destroyFlag == 0) {// Because ghost ships don't behave w/ linkNear. The problem is that linkNear assumes the given ship is actually in the sector. I recognize this means you can't see out-of-sector things when dead; fix it if'n you dare.
		linkNear(who, 64*6400 * 16);
	} else if (who->destroyFlag == CANSPAWN) return; // Kinda hackish, but should wokr???
	entity* runner = who->mySector->firstentity;
	int64_t d;
	while(runner){
		if (runner == who) {
			runner = runner->next;
			continue;
		}
		d = divide(displacementX(who, runner)+3200*16, 6400*16);
		if(d > 63 || d < -63){
			runner = runner->next;
			continue;
		}
		data[dataLen+1] = d+63;
		d = divide(displacementY(who, runner)+3200*16, 6400*16);
		if(d > 63 || d < -63){
			runner = runner->next;
			continue;
		}
		if (dataLen + 3 > NETLEN) {
			puts("Dear Lord, even the radar packet is too big!");
			break;
		}
		data[dataLen+2] = d+63;
		data[dataLen] = runner->faction;
		dataLen+=3;
		runner = runner->next;
	}
	if (who->destroyFlag == 0)
		unlinkNear();
	if (dataLen < 3) {
		dataLen = 3;
		data[2] = 192;
	} else {
		data[2] += 192;
	}
	if(who->me->pos[0] < POS_MIN+6400*64*16) data[0] = (-who->me->pos[0]+POS_MIN+(6400*64*16))/(6400*16);
	else if(who->me->pos[0] > POS_MAX-6400*64*16) data[0] = (-who->me->pos[0]+POS_MAX+(6400*64*16))/(6400*16);
	else data[2] -= 128;
	if(who->me->pos[1] < POS_MIN+6400*64*16) data[1] = (-who->me->pos[1]+POS_MIN+(6400*64*16))/(6400*16);
	else if(who->me->pos[1] > POS_MAX-6400*64*16) data[1] = (-who->me->pos[1]+POS_MAX+(6400*64*16))/(6400*16);
	else data[2] -= 64;
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
			if (conductor->spawnBase) conductor->myShip = loadshipSpawner(conductor->name, conductor->spawnBase);
			else conductor->myShip = loadship(conductor->name);
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
		if(counter == 0 && me->destroyFlag != CANSPAWN) sendRadar(conductor);
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
			int count = 0;
			for (; count<runner->numTrails; count++){
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
	char msg[MAXNAMELEN+1];
	socklen_t len;
	int msgSize;
	while(1){
		len = sizeof(bindAddr);
		msgSize = recvfrom(sockfd, msg, MAXNAMELEN+1, 0, (struct sockaddr*)&bindAddr, &len);
		client* current = clientList;
		while(current != NULL){
			if(current->addr.sin_addr.s_addr == bindAddr.sin_addr.s_addr){
				if (current->myShip->destroyFlag == CANSPAWN && !current->requestsSpawn) {
					current->requestsSpawn = 1;
					printf("Player %s has respawned!\n", current->name);
					break;
				} else if (current->myShip->destroyFlag) {
					break;
				}
				if(msgSize == 1)
					((humanAiData*)current->myShip->aiFuncData)->keys = *msg;
				break;
			}
			current = current->next;
		}
		if(current == NULL){//That is, he isn't joined yet
			if(msgSize <= 1 || *msg != ']') continue;//Our super-secret, "I'm a legitimate client" character
			if (strnlen(msg, MAXNAMELEN + 1) > MAXNAMELEN) {
				printf("player name '%s' exceeds MAXNAMELEN of %d.\n", msg+1, MAXNAMELEN);
				continue;
			}
			client* new = malloc(sizeof(client));
			strcpy(new->name, msg+1);
			printf("He requested ship %s\n", msg+1);
			new->addr = bindAddr;
			new->spawnBase = NULL;
			new->requestsSpawn = 0;
			loadRequest *req = malloc(sizeof(loadRequest));
			req->next = firstLoadRequest;
			req->cli = new;
			strcpy(req->name, msg+1);
			firstLoadRequest = req;
		}
	}
	return NULL;
}
