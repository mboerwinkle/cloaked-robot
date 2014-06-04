#include <stdlib.h>
#include <stdio.h>
#include "globals.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct client{
	struct client* next;
	char keys[6];
	struct sockaddr_in addr;
}client;

client* clientList = NULL;

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
		printf("Message from %s\n", inet_ntoa(bindAddr.sin_addr));
	}
	return NULL;
}
