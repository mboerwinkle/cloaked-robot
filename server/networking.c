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
	static int16_t data[3*100];
	//TODO: Decide if the above should be static
	int dataLen;
	int64_t d;
	client* conductor = clientList;
	while(conductor){
		linkNear(conductor->myShip, LOCK_RANGE);
		sector *sec = conductor->myShip->mySector;
		entity *runner = sec->firstentity;
		data[0] = simonMod(((int64_t)sec->x%3000)*(464)-simonDivide(conductor->myShip->x,64), 3000)/2;
		data[1] = simonMod(((int64_t)sec->y%3000)*(464)-simonDivide(conductor->myShip->y,64), 3000)/2;
		dataLen = 2;
		while(runner){
			data[dataLen+0] = 0x01*runner->theta+0x10*0/*flame or not*/+0x20*0/*faction*/+0x80*runner->type;
			d = simonDivide(displacementX(conductor->myShip, runner)+32, 64);
			if(d < INT16_MIN || d > INT16_MAX){
				runner = runner->next;
				continue;
			}
			data[dataLen+1] = d;
			d = simonDivide(displacementY(conductor->myShip, runner)+32, 64);
			if(d < INT16_MIN || d > INT16_MAX){
				runner = runner->next;
				continue;
			}
			data[dataLen+2] = d;
			data[dataLen+3] = 0;
			if(runner == conductor->myShip->targetLock){
				 data[dataLen+3] |= 1;
			}
			dataLen+=4;
			runner = runner->next;
		}
		unlinkNear();
		sendAddr.sin_addr.s_addr = conductor->addr.sin_addr.s_addr;
		sendto(sockfd, (char*)data, dataLen*2, 0, (struct sockaddr*)&sendAddr, sizeof(sendAddr));
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
