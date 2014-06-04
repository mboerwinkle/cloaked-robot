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

	return NULL;
}
