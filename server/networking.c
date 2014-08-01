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

static int8_t data[600];

static void sendRadar(client* cli){
	int dataLen = 2;
	entity* who = cli->myShip;
	linkNear(who, 64*6400);
	entity* runner = who->mySector->firstentity;
	int64_t d;
	while(runner){
		d = simonDivide(displacementX(who, runner)+3200, 6400);
		if(d > 63 || d < -63){
			runner = runner->next;
			continue;
		}
		data[dataLen+1] = d+63;
		d = simonDivide(displacementY(who, runner)+3200, 6400);
		if(d > 63 || d < -63){
			runner = runner->next;
			continue;
		}
		data[dataLen+2] = d+63;
		data[dataLen] = runner->faction;
		dataLen+=3;
		runner = runner->next;
	}
	unlinkNear();
	data[2] += 192;
	if(who->x < POS_MIN+6400*64) data[0] = (-who->x+POS_MIN+(6400*64))/6400;
	else if(who->x > POS_MAX-6400*64) data[0] = (-who->x+POS_MAX+(6400*64))/6400;
	else data[2] -= 128;
	if(who->y < POS_MIN+6400*64) data[1] = (-who->y+POS_MIN+(6400*64))/6400;
	else if(who->y > POS_MAX-6400*64) data[1] = (-who->y+POS_MAX+(6400*64))/6400;
	else data[2] -= 64;
	data[0] |= 0x80; // It's a radar packet
	struct sockaddr_in sendAddr = {.sin_family=AF_INET, .sin_port=htons(3334), .sin_addr={.s_addr=cli->addr.sin_addr.s_addr}};
	sendto(sockfd, data, dataLen, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
}

void sendInfo(){
	static char counter = 0;
	if(++counter == 10) counter = 0;
	struct sockaddr_in sendAddr = {.sin_family=AF_INET, .sin_port=htons(3334)};
	//TODO: Decide if the above should be static
	int dataLen;
	int64_t d;
	client* prev = NULL;
	client* conductor = clientList;
	while(conductor){
		entity* me = conductor->myShip;
		if(me->destroyFlag){
			sendAddr.sin_addr.s_addr = conductor->addr.sin_addr.s_addr;
			*data = 0x41; // Lol, you died
			sendto(sockfd, (char*)data, 1, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
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
		if(counter == 0) sendRadar(conductor);
		linkNear(me, LOCK_RANGE);
		sector *sec = me->mySector;
		entity *runner = sec->firstentity;
		data[0] = me->faction;
		*(int16_t*)(data+1) = simonMod(((int64_t)sec->x%3000)*(464)-simonDivide(conductor->myShip->x,64), 3000)/2;
		*(int16_t*)(data+3) = simonMod(((int64_t)sec->y%3000)*(464)-simonDivide(conductor->myShip->y,64), 3000)/2;
		data[5] = me->shield*255/me->maxShield;
		data[6] = me->energy*255/me->maxEnergy;
		dataLen = 7;
		while(runner){
			data[dataLen+0] = 0x01*runner->theta+0x10*0/*flame or not*/+0x20*runner->faction;
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
				dataLen+=3;
				count++;
			}
			runner = runner->next;
		}
		unlinkNear();
		sendAddr.sin_addr.s_addr = conductor->addr.sin_addr.s_addr;
		sendto(sockfd, (char*)data, dataLen, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
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
					((humanAiData*)current->myShip->aiFuncData)->keys = *msg;
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
