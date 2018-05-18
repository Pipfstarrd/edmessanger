#include <jansson.h>
#include <string.h>

#include "eventlist.h"
#include "user.h"
#include "api.h"
#include "clientdata.h"


typedef struct {
	Usertable *usertable;
	Eventlist *eventlist;
} APIData;


static APIData apiData;


int initApi(Usertable *usertable)
{
	apiData.usertable = usertable;
	initEventlist(apiData.eventlist);
	return 0;
}


int parse(char *message) 
{
	json_t       *root;
	json_t       *action;
	json_error_t error;

	root = json_loads(message, 0, &error);

	action = json_object_get(root, "action");

	if (!strcmp(json_string_value(action), "register")) {
		regUser(json_string_value(json_object_get(root, "username")),
		        json_string_value(json_object_get(root, "password")));
	} else if (!strcmp(json_string_value(action), "auth")) {
	} else if (!strcmp(json_string_value(action), "sendMsg")) {
	} else if (!strcmp(json_string_value(action), "getUpdates")) {
	}

	return 0;	
}


int regUser(const char *username, const char *password)
{
	User *user = malloc(sizeof(User));

	user->username = (uint8_t*) username;
	user->password = (uint8_t*) password;

	addUser(apiData.usertable, user);
	return 0;
}


int authUser(char *username, char *password)
{
	User *user = getUser(apiData.usertable, username);
	if (!strcmp(user->password, password)) {
		return 0;
	} else return -1;
}

int sendMsg(char* token, char* msg, char* recipient)
{
	return 0;
}


int getUpdates(char *username)
{
	return 0;
}
