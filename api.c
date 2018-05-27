#include <jansson.h>
#include <string.h>

#include "eventlist.h"
#include "user.h"
#include "api.h"
#include "clientdata.h"


typedef struct {
	Usertable *usertable;
} APIData;


static APIData apiData;


int initApi(Usertable *usertable)
{
	apiData.usertable = usertable;
	return 0;
}

char errorJson[] = "{\"status\": \"ERR\", \"text\": \"Wrong request format\" }";


char* parse(char *message) 
{
	json_t       *root;
	json_t       *action;
	json_error_t error;

	if (message != NULL) {
		root = json_loads(message, 0, &error);
	} else {
		return "";
	}

	action = json_object_get(root, "action");

	if (!strcmp(json_string_value(action), "register")) {
		const char *username = json_string_value(json_object_get(root, "username"));
		const char *password = json_string_value(json_object_get(root, "password"));
		if (regUser(username, password) == -1) {
			return formatError();
		} else if (username == NULL || password == NULL) {
			return formatError();
		} else {
			json_t *params = json_pack("{s:s}", "text", "New user added");
			return formatResponse("OK", params);
		}
	} else if (!strcmp(json_string_value(action), "auth")) {
		char *token;
		const char *username = json_string_value(json_object_get(root, "username"));
		const char *password = json_string_value(json_object_get(root, "password"));

		if ( (token = authUser(username, password)) == NULL) {
			return formatError();
		} else if (username == NULL || password == NULL) {
			return formatError();
		} else {
			json_t *params = json_pack("{s:s}", "token", token); 
			return formatResponse("OK", params);
		}
	} else if (!strcmp(json_string_value(action), "sendMsg")) {
		const char *username  = json_string_value(json_object_get(root, "username"));
		const char *token     = json_string_value(json_object_get(root, "token"));
		const char *msg       = json_string_value(json_object_get(root, "msg"));
		const char *recipient = json_string_value(json_object_get(root, "recipient"));
		if (username != NULL && token != NULL && msg != NULL && recipient != NULL) {
			return sendMsg(username, token, recipient, msg);
		} else {
			return formatError();
		}
	} else if (!strcmp(json_string_value(action), "getUpdates")) {
		const char *username = json_string_value(json_object_get(root, "username"));
		const char *token = json_string_value(json_object_get(root, "token"));
		if (username != NULL && token != NULL) {
			return getUpdates(username, token);
		} else {
			return formatError();
		}
	} else {
		formatError();
		return formatError();
	}

	return NULL;	
}


int regUser(const char *username, const char *password)
{
	FILE *fp;

	User *test = getUser(apiData.usertable, username);
	if (test != NULL) {
		return -1;
	}

	User *user = malloc(sizeof(User));

	user->username = (char*) username;
	user->password = (char*) password;

	addUser(apiData.usertable, user);

	char *v = dumpTable(apiData.usertable);
	fp = fopen("users.db", "w");

	fprintf(fp, "%s", v);

	fclose(fp);

	free (v);

	return 0;
}


char* authUser(const char *username, const char *password)
{
	User *user = getUser(apiData.usertable, username);
	if (!strcmp(user->password, password)) {
		user->token = "aaabckkkdjf31";
		return "aaabckkkdjf31";
	} else return NULL;
}

char* sendMsg(const char* username, const char* token, const char* recipient, const char* msg )
{
	User *user = getUser(apiData.usertable, username);	
	if (!user) {
		return formatError();
	} else if (strcmp(user->token, token)) {
		return formatError();
	} 

	User *recipnt = getUser(apiData.usertable, recipient);
	if (!recipnt) {
		return formatError();
	}

	addEvent(&recipnt->eventlist, "newMessage", msg, username);
	json_t *params = json_pack("{s:s}", "text", "Message sent");
	return formatResponse("OK", params);
}


char* getUpdates(const char *username, const char* token)
{
	User *user = getUser(apiData.usertable, username);
	
	if (user == NULL) {
		return formatError();
	}

	//if (strcmp(user->token, token)) {
//		return formatError();
//	}
	
	json_t *events = json_array();

	while (user->eventlist != NULL) {
		json_t *event  = json_object();
		json_object_set_new(event, "event", json_string(user->eventlist->event));
		json_object_set_new(event, "sender", json_string(user->eventlist->sender)); 
		json_object_set_new(event, "text", json_string(user->eventlist->message)); 
		json_array_append_new(events, event);

		removeEvent(&user->eventlist, user->eventlist);
	}
	
	return formatResponse("OK", events);
}

char* formatError()
{
	return errorJson;
}

char* formatResponse(char* status, json_t *params)
{
	json_t *object = json_object();
	json_object_set(object, "status", json_string(status));
	if (json_is_object(params)) {
		json_object_update(object, params);
	} else {
		json_object_set(object, "text", params);
	}

	return json_dumps(object, 0);
}
