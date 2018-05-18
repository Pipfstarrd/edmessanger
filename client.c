#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include <jansson.h>

#define BUFSIZE 65536 // 64 * 1024 = 64 KiB


typedef enum { HELP, SENDMSG, GETUPDATES, AUTH, REGISTER } Command;

typedef struct {
	Command command;
	char    *username;
	char    *password;
	char    *token;
	char    *msg;
	char    *recipient;
} Request;


typedef struct {
	char    *username;
	char    *password;
	uint8_t loggedin;
	char    *token;
} User;

typedef enum { OK, ERR } Status;

typedef struct {
	Status  status;
	uint8_t *text;
} Response;



Request*  parse(char *buffer, User *user);
Response* runRequest(Request *req, int sockfd);
char*    decode(Response *response);
char*    getToken();
char*    setToken();



int main(int argc, char **argv)
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	User user;
	user.loggedin = 0;

	char buffer[BUFSIZE];

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

	printf("Connected to %s:%s\n", argv[1], argv[2]);

	while(1) {

		printf("[Status] [%s] ", user.loggedin?"Online":"Offline");
		printf("Enter your command:\n");
		bzero(buffer, BUFSIZE);

		fgets(buffer, BUFSIZE - 1, stdin);

		Request *req = parse(buffer, &user);
		if (req != NULL) {
			Response *response = runRequest(req, sockfd);
			printf("%s", decode(response));
		}

		n = write(sockfd, buffer, strlen(buffer));

		if (n < 0) {
			perror("ERROR writing to socket");
			return -1;
		}

		bzero(buffer, BUFSIZE);
//		n = read(sockfd, buffer, 255);

		if (n < 0) {
			perror("ERROR reading from socket");
			return -1;
		}

		printf("%s\n", buffer);
	}
	return 0;
}


Request* parse(char *buffer, User *user)
{
	Request *req; 
	if (buffer || buffer[0] != '/') {
		char *argstring = buffer;
		char *command;
		command = strsep(&argstring, " \t\n");
		if (strcmp(command, "/help") == 0) {
			printf("Usage: /command [args]\n\
commands: /help /auth /register /send /update\n\
/auth USERNAME PASSWORD\n/register USERNAME PASSWORD \n\
/send RECIPIENT message\n/update — gets messages from the server\n");

		} else if (strcmp(command, "/auth") == 0) {
			req->command  = AUTH;
			req->username = strsep(&argstring, " \t\n");
			req->password = strsep(&argstring, " \t\n");
			return req;
		} else if (strcmp(command, "/register") == 0) {
			req->command  = REGISTER;
			req->username = strsep(&argstring, " \t\n");
			req->password = strsep(&argstring, " \t\n");
			return req;
		} else if (strcmp(command, "/update") == 0) {
			req->command = GETUPDATES;
			req->token   = user->token;
			return req;
		} else if (strcmp(command, "/send") == 0) {
			req->command   = SENDMSG;
			req->token     = user->token;
			req->recipient = strsep(&argstring, " \t\n");
			req->msg       = argstring;
			return req;
		} else {
			printf("Please, enter a valid command. Refer /help for information\n");
			return NULL;
		}
		
	} else {
		printf("Please, enter a valid command. Refer /help for information\n");
		return NULL;
	}
	return NULL;
}

Response* runRequest(Request *req, int sockfd)
{
	char buf[BUFSIZE];
	char *v;
	Response *resp;
	json_error_t *error;
	json_t *request, *response;

	switch (req->command) {
		case AUTH:
			printf("%s %s\n", req->username, req->password);
			fflush(stdout);
			request = json_pack("{s:s, s:s, s:s}", "action", "auth", "username", 
			                    req->username, "password", req->password);

			v = json_dumps(request, 0);
			write(sockfd, v, strlen(v));

			read(sockfd, buf, BUFSIZE);

			response = json_loads(buf, 0, error);
			json_unpack(response, "{s:s, s:s}", "status", &resp->status, "token", &resp->text);

			break;

		case REGISTER:
			request = json_pack("{s:s, s:s, s:s}", "action", "register", "username", 
			                    req->username, "password", req->password);

			v = json_dumps(request, 0);
			write(sockfd, v, strlen(v));


			read(sockfd, buf, BUFSIZE);

			response = json_loads(buf, 0, error);
			json_unpack(response, "{s:s, s:s}", "status", &resp->status, "text", &resp->text);

			break;

		case GETUPDATES:
			request = json_pack("{s:s, s:s, s:s}", "action", "getupdates", "token", 
			                    req->token);

			v = json_dumps(request, 0);
			write(sockfd, v, strlen(v));

			read(sockfd, buf, BUFSIZE);

			response = json_loads(buf, 0, error);
			json_unpack(response, "{s:s, s:s}", "status", &resp->status, "text", &resp->text);

			break;

		case SENDMSG:
			request = json_pack("{s:s, s:s, s:s, s:s}", "action", "sendMsg", "token", 
			                    req->token, "recipient", req->recipient, "msg", req->msg);

			v = json_dumps(request, 0);
			write(sockfd, v, strlen(v));

			read(sockfd, buf, BUFSIZE);

			response = json_loads(buf, 0, error);
			json_unpack(response, "{s:s, s:s}", "status", &resp->status, "text", &resp->text);

			break;

		default: 
			resp = NULL;
			break;
	}
	return resp;
}

char* decode(Response *response)
{
	return "";
}
