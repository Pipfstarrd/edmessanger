#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>


int main(int argc, char **argv)
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];

	if (argc != 3) {
		printf("Usage %s hostname port", argv[0]);
		return -1;
	}

	portno = atoi(argv[2]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		perror("ERROR creating socket");
		return -1;
	}

	server = gethostbyname(argv[1]);

	if (server == NULL) {
		printf("ERROR, no such host");
		return -1;
	}

	bzero((char*) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR connecting");
		return -1;
	}

	while(1) {
		printf("Please, print the message:\n");
		bzero(buffer, 256);
		fgets(buffer, 255, stdin);

		n = write(sockfd, buffer, strlen(buffer));

		if (n < 0) {
			perror("ERROR writing to socket");
			return -1;
		}

		bzero(buffer, 256);
		n = read(sockfd, buffer, 255);

		if (n < 0) {
			perror("ERROR reading from socket");
			return -1;
		}

		printf("%s\n", buffer);
	}
	return 0;
}
