#ifndef CLIENTDATA_H
#define CLIENTDATA_H

#include <netinet/in.h>
#include <pthread.h>

typedef struct {
	int32_t               sockfd; // Socket descriptor
	struct    sockaddr_in client_addr;
	int32_t               clilen;
	pthread_t             threadHandler; 
	uint8_t               connected;
} Client;

typedef struct {
	Usertable **usertable;
	Client    *clients;
	uint16_t  clientId;
} ClientEnv;

#endif      /* CLIENTDATA_H */
