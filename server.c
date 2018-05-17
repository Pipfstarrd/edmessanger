#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <jansson.h>
#include "user.h"


#define CLIENT_MAX 1000


const char help[]="Usage:\n\
postman port\n";


void* clientHandler(void *args);


typedef struct {
	int32_t sockfd; // Socket descriptor
	struct sockaddr_in client_addr;
	int32_t clilen;
	pthread_t threadHandler; 
	int32_t connected;
} Client;



int main(int argc, char **argv)
{
	int32_t sockfd;

	Client clients[CLIENT_MAX];
	for (int i = 0; i < CLIENT_MAX; i++) {
		clients[i].sockfd = 0;
		clients[i].clilen = sizeof(clients[i].client_addr);
		clients[i].connected = 0;
	}
	

	int16_t portno;
	int32_t clilen;
	
	Usertable *usertable = newUserTable(CLIENT_MAX);

	User *test = (User*) malloc(sizeof(User));
	uint8_t* testusername = "pipfstarrd"; test->username = malloc(sizeof(testusername));
	test->username = testusername;
	test->hash = hashf(test->username);

	addUser(&usertable, test);	

	User *pointer = getUser(usertable, "pipfstarrd");
	if (pointer == NULL) {
		printf("Error on retrieving user form table!");
		return 1;
	}

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
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		return -1;
	}

	bzero((char*) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		return -1;
	}

	listen(sockfd, 100);
	int i = 0;

	while (1) {

		clients[i].sockfd = accept(sockfd, (struct sockaddr*)&clients[i].client_addr, &clients[i].clilen);

		if (clients[i].sockfd < 0) {
			perror("ERROR on accept");
			fflush(stderr);
		} else {
			clients[i].connected = 1;
		}

		pthread_create(&clients[i].threadHandler, NULL, clientHandler, (void*)&clients[i]);


		while (clients[i].connected != 0) {
			i++;
			i %= CLIENT_MAX; 
		}
		printf("I: %d\n", i);
	}
}


void* clientHandler(void *args) 
{
	Client *client = (Client*) args;
	uint8_t buffer[65536]; // 64k is enough for everyone
	
	while (1) {
		bzero (buffer, 65536);
		int n = read(client->sockfd, buffer, 65536);

		if (n != -1 && n <= 0) {
			perror("ERROR reading from socket");
			fflush(stderr);
			close(client->sockfd);
			client->sockfd = 0;
			client->connected = 0;
		}

		printf("Here is the message: %s", buffer);

		n = write(client->sockfd, "Gotcha!", 8);

		if (n != -1 && n <= 0) {
			perror("ERROR writing to socket");
			fflush(stderr);
			close(client->sockfd);
			client->sockfd = 0;
			client->connected = 0;
		}

	}
	
	// Shouldn't get here
	close(client->sockfd);
}
