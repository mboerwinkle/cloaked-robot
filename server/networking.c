#include <stdlib.h>
#include <stdio.h>
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

void* netListen(void* whoGivesADern){
	puts("So, yeah, threading.");
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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
	while(1){
		len = sizeof(bindAddr);
		recvfrom(sockfd, msg, 20, 0, (struct sockaddr*)&bindAddr, &len);
		client* current = clientList;
		while(current != NULL){
			if(current->addr.sin_addr.s_addr == bindAddr.sin_addr.s_addr){
				puts("Message from existing client.");
				break;
			}
		}
		if(current == NULL){//That is, he isn't joined yet
			if(*msg != ']') continue;//Our super-secret, "I'm a legitimate client" character
			printf("He requested ship %s\n", msg+1);
			client* new = malloc(sizeof(client));
			new->next = clientList;
			new->addr = bindAddr;
			new->myShip = loadship(msg+1);
			clientList = new;
		}
		printf("Message from %s\n", inet_ntoa(bindAddr.sin_addr));
	}
	return NULL;
}
