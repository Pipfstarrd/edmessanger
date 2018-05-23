#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <jansson.h>

#include "user.h"
#include "api.h"
#include "clientdata.h"


#define CLIENT_MAX 1000
#define BUFSIZE 65536      /* 64 * 1024 = 64 KiB */


const char help[] = "Usage:\n\
postman port\n";


void* clientHandler(void *args);


void* clientHandler(void *args) 
{
	ClientEnv *envHandler = (ClientEnv*) args;
	Client    *client     = &envHandler->clients[envHandler->clientId];

	uint8_t buffer[BUFSIZE]; // 64k is enough for everyone
	
	while (1) {
		bzero (buffer, BUFSIZE);
		int n = read(client->sockfd, buffer, BUFSIZE);

		if (n == -1 || n == 0) {
			perror("ERROR reading from socket");
			fflush(stderr);

			close(client->sockfd);

			client->sockfd    = 0;
			client->connected = 0;
			return NULL;
		}

//		const char jsonReply[] = "{\"status\": \"OK\", \"text\": \"Greetings from Equestria!\"}";

		char *jsonReply = parse(buffer);
		printf("REPLY: %s %lu\n", jsonReply, strlen(jsonReply));

		n = write(client->sockfd, jsonReply, strlen(jsonReply));
		free(jsonReply);

		if (n != -1 && n <= 0) {
			perror("ERROR writing to socket");
			fflush(stderr);
			 
			close(client->sockfd);

			client->sockfd    = 0;
			client->connected = 0;
			return NULL;
		}

	}
	
	// Shouldn't get here
	close(client->sockfd);
}


int main(int argc, char **argv)
{
	int32_t sockfd;

	Client clients[CLIENT_MAX];
	for (int i = 0; i < CLIENT_MAX; i++) { 
		clients[i].sockfd    = 0;
		clients[i].clilen    = sizeof(clients[i].client_addr);
		clients[i].connected = 0;
	}

	int16_t portno;
	int32_t clilen;
	
	Usertable *usertable = newUserTable(CLIENT_MAX);

	struct sockaddr_in serv_addr;


	if (argc != 2) {
		printf("%s", help);
		return -1;
	} else {
		portno = atoi(argv[1]);
		if (portno <= 0) {
			perror("Wrong port format");
			return -1;
		}
	}

	initApi(usertable);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		return -1;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

	bzero((char*) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port        = htons(portno);
	
	if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		return -1;
	}

	listen(sockfd, 100);

	uint16_t i = 0;

	while (1) {

		clients[i].sockfd = accept(sockfd, (struct sockaddr*)&clients[i].client_addr,
		                           &clients[i].clilen);

		if (clients[i].sockfd < 0) {
			perror("ERROR on accept");
			fflush(stderr);
		} else {
			clients[i].connected = 1;
		}

		
		ClientEnv *clientEnv = malloc(sizeof(ClientEnv));

		clientEnv->usertable = &usertable;
		clientEnv->clients   = clients;
		clientEnv->clientId  = i;

		pthread_create(&clients[i].threadHandler, NULL, clientHandler, 
		               (void*)clientEnv);

		while (clients[i].connected != 0) {
			i++;
			i %= CLIENT_MAX; 
		}

		printf("I: %d\n", i);
	}
}
