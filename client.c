#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include <pthread.h>

#include <jansson.h>

#define BUFSIZE 65536 // 64 * 1024 = 64 KiB


const char usage[] = "Usage: /command [args]\n\
commands: /help /auth /register /send /update\n\
/auth USERNAME PASSWORD\n/register USERNAME PASSWORD \n\
/send RECIPIENT message\n/update — gets messages from the server\n";

const char bugMessage[] = "Wrong request, please, fill a bugreport";

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
	int32_t sockfd;
} User;


typedef struct {
	char    *status;
	uint8_t *text;
} Response;



Request*    parse(char *buffer, User *user);
Response*   runRequest(Request *req, int sockfd);
const char* decode(Response *response);
char*       getToken();
char*       setToken();

void* updateHandler(void* arg);


// Temporary handler for user data
User user;

int main(int argc, char **argv)
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;


	Response *response;

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
	
	user.sockfd = sockfd;

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
	
	pthread_t threadHandler;
	pthread_create(&threadHandler, NULL, updateHandler, NULL);

	while (1) {

		printf("\n[Status] [%s] ", user.loggedin? "Online" : "Offline");
		printf("Enter your command:\n");
		bzero(buffer, BUFSIZE);

		fgets(buffer, BUFSIZE - 1, stdin);

		Request *req = parse(buffer, &user);
		if (req != NULL) {
			response = runRequest(req, sockfd);
			if (response == NULL) {
				printf("Failed to process the request!");
			}
			free(req); 
			free (response);
		}

//		n = write(sockfd, buffer, strlen(buffer));

		/*if (n < 0) {
			perror("ERROR writing to socket");
			return -1;
		}*/

	}
	return 0;
}


Request* parse(char *buffer, User *user)
{
	Request *req = malloc(sizeof(Request)); 
	if (buffer[0] == '/') {
		char *argstring = strdup(buffer);
		char *command;
		command = strsep(&argstring, " \t\n");

		if (strcmp(command, "/help") == 0) {
			printf("%s", usage);
		} else if (strcmp(command, "/auth") == 0) {
			req->command  = AUTH;
			req->username = strsep(&argstring, " \t\n");
			req->password = strsep(&argstring, " \t\n");

			if (!req->username || !req->password) {
				printf("Wrong auth command format, plase refer /help for info.\n");
				return NULL;
			}
			return req;
		} else if (strcmp(command, "/register") == 0) {
			req->command  = REGISTER;
			req->username = strsep(&argstring, " \t\n");
			req->password = strsep(&argstring, " \t\n");
			return req;
		} else if (strcmp(command, "/update") == 0) {
			req->command  = GETUPDATES;
			req->token    = user->token;
			req->username = user->username;
			return req;
		} else if (strcmp(command, "/send") == 0) {
			req->command   = SENDMSG;
			req->token     = user->token;
			req->recipient = strsep(&argstring, " \t\n");
			req->msg       = argstring;
			req->username  = user->username;
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
	Response *resp = malloc(sizeof(Response));
	json_error_t *error;
	json_t *request, *response;
	bzero(buf, BUFSIZE);

	switch (req->command) {
		case AUTH:
			request = json_pack("{s:s, s:s, s:s}", "action", "auth", "username", 
			                    req->username, "password", req->password);

			v = json_dumps(request, 0);
			write(sockfd, v, strlen(v));

			read(sockfd, buf, BUFSIZE);

			response = json_loads(buf, 0, error);
			json_unpack(response, "{s:s, s:s}", "status", &resp->status, "token", &resp->text);
			if (!strcmp(resp->status, "OK")) {
				user.username  = req->username;
				user.loggedin  = 1;
				user.token     = resp->text;
			}

			break;

		case REGISTER:
			request = json_pack("{s:s, s:s, s:s}", "action", "register", "username", 
			                    req->username, "password", req->password);

			v = json_dumps(request, 0);
			write(sockfd, v, strlen(v));


			read(sockfd, buf, BUFSIZE);

			response = json_loads(buf, 0, error);
			json_unpack(response, "{s:s, s:s}", "status", &resp->status, "text", &resp->text);

			printf("%s\n", decode(resp));
			break;

		case GETUPDATES:
			request = json_pack("{s:s, s:s, s:s}", "action", "getUpdates", "token", 
			                    req->token, "username", req->username);

			v = json_dumps(request, 0);
			if (!v) {
				printf("You're not authorized!\n");
				return NULL;
			}

			write(sockfd, v, strlen(v));

			read(sockfd, buf, BUFSIZE);

			fflush(stdout);
			response = json_loads(buf, 0, error);
			json_unpack(response, "{s:s}", "status", &resp->status);
			if (!strcmp(resp->status, "OK")) {
				json_t *array;
				json_t *value;
				size_t index;

				array = json_object_get(response, "text");
				if (!array || json_array_size(array) == 0) {
					//printf("No updates yet!");
					return NULL;
				}

				json_array_foreach(array, index, value) {
					char *sender, *type, *msg;
					json_unpack(value, "{s:s, s:s, s:s}", "event", &type,
					            "sender", &sender, "text", &msg);
					if (!strcmp(type, "newMessage")) {
						printf("<%s> %s\n", sender, msg);
					}
				}
			}

			break;

		case SENDMSG:
			request = json_pack("{s:s, s:s, s:s, s:s, s:s}", "action", "sendMsg", "token",
			                    req->token, "recipient", req->recipient, "msg", req->msg,
			                    "username", req->username);

			v = json_dumps(request, 0);
			if (!v) {
				/* A dirty hack at the moment, we still lack error detection and 
				   handling at the server API */
				printf("You're not authorized!\n");
				return NULL;
			}
			write(sockfd, v, strlen(v));

			read(sockfd, buf, BUFSIZE);

			response = json_loads(buf, 0, error);
			json_unpack(response, "{s:s, s:s}", "status", &resp->status, "text", &resp->text);
			printf("%s\n", decode(resp));

			break;

		default: 
			resp = NULL;
			break;
	}

	free (v);
	free(request);
	return resp;
}

const char* decode(Response *response)
{
	if (!strcmp(response->status, "OK")) {
		return response->text;
	} else {
		return bugMessage;
	}
	
	// Shouldn't get here 
	return bugMessage;
}


void* updateHandler(void* arg) {
	Request *req = malloc(sizeof(Request));
	while(1) {
		if (user.loggedin) {
			req->command  = GETUPDATES;
			req->token    = user.token;
			req->username = user.username;
			runRequest(req, user.sockfd);
		}
		sleep(1);
	}
	return NULL;
}
