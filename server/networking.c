#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include "globals.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct client{
	struct client* next;
	char keys[6];
	struct sockaddr_in addr;
	entity *myShip;
}client;

static client* clientList = NULL;

static int sockfd;

static int32_t simonDivide(int64_t a, int32_t b){
	if(a<0) a-=(b-1);
	return a/b;
}

static int32_t simonMod(int64_t a, int32_t b){
	int32_t result = a%b;
	if(result<0) return result+b;
	return result;
}

void sendInfo(){
	struct sockaddr_in sendAddr = {.sin_family=AF_INET, .sin_port=htons(3334)};
	static int8_t data[6*100];
	//TODO: Decide if the above should be static
	int dataLen;
	int64_t d;
	client* prev = NULL;
	client* conductor = clientList;
	while(conductor){
		entity* me = conductor->myShip;
		if(me->destroyFlag){
			if(prev){
				prev->next = conductor->next;
				free(conductor);
				conductor = prev->next;
				continue;
			}
			clientList = conductor->next;
			free(conductor);
			conductor = clientList;
			continue;
		}
		linkNear(me, LOCK_RANGE);
		sector *sec = me->mySector;
		entity *runner = sec->firstentity;
		((int16_t*)data)[0] = simonMod(((int64_t)sec->x%3000)*(464)-simonDivide(conductor->myShip->x,64), 3000)/2;
		*(int16_t*)(data+2) = simonMod(((int64_t)sec->y%3000)*(464)-simonDivide(conductor->myShip->y,64), 3000)/2;
		data[4] = 0x01*me->theta+0x10*0+0x20*0;
		data[5] = me->type;
		data[6] = me->shield*255/me->maxShield;
		data[7] = me->energy*255/me->maxEnergy;
		dataLen = 8;
		while(runner){
			if(runner == me){
				runner = runner->next;
				continue;
			}
			data[dataLen+0] = 0x01*runner->theta+0x10*0/*flame or not*/+0x20*0/*faction*/;
			data[dataLen+1] = runner->type;
			d = simonDivide(displacementX(conductor->myShip, runner)+32, 64);
			if(d < INT16_MIN || d > INT16_MAX){
				runner = runner->next;
				continue;
			}
			*(int16_t*)(data+dataLen+2) = d;
			d = simonDivide(displacementY(conductor->myShip, runner)+32, 64);
			if(d < INT16_MIN || d > INT16_MAX){
				runner = runner->next;
				continue;
			}
			*(int16_t*)(data+dataLen+4) = d;
			data[dataLen+6] = runner->shield*31/runner->maxShield;
			if(runner == conductor->myShip->targetLock){
				 data[dataLen+6] |= 0x20;
			}
			dataLen+=7;
			int count = 0;
			while(count<runner->numTrails){
				data[dataLen-1] |= 0x80;
				data[dataLen+2] = runner->trailTypes[count];
				d = simonDivide(displacementX(runner, runner->trailTargets[count])+32, 64);
				if(d<0){
					d+=256;
					data[dataLen+2] |= 0x40;
				}
				data[dataLen] = d;
				d = simonDivide(displacementY(runner, runner->trailTargets[count])+32, 64);
				if(d<0){
					d+=256;
					data[dataLen+2] |= 0x20;
				}
				data[dataLen+1] = d;
				count++;
			}
			runner = runner->next;
		}
		unlinkNear();
		sendAddr.sin_addr.s_addr = conductor->addr.sin_addr.s_addr;
		sendto(sockfd, (char*)data, dataLen*2, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
		prev = conductor;
		conductor = conductor->next;
	}
}

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
	char msg[20];
	socklen_t len;
	int msgSize;
	while(1){
		len = sizeof(bindAddr);
		msgSize = recvfrom(sockfd, msg, 20, 0, (struct sockaddr*)&bindAddr, &len);
		client* current = clientList;
		while(current != NULL){
			if(current->addr.sin_addr.s_addr == bindAddr.sin_addr.s_addr){
				if(msgSize == 1)
					*(char*)current->myShip->aiFuncData = *msg;
				break;
			}
			current = current->next;
		}
		if(current == NULL){//That is, he isn't joined yet
			if(msgSize <= 1 || *msg != ']') continue;//Our super-secret, "I'm a legitimate client" character
			printf("He requested ship %s\n", msg+1);
			client* new = malloc(sizeof(client));
			new->next = clientList;
			new->addr = bindAddr;
			new->myShip = loadship(msg+1);
			clientList = new;
		}
//		printf("Message from %s\n", inet_ntoa(bindAddr.sin_addr));
	}
	return NULL;
}
