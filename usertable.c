#include <stdlib.h>
#include <string.h>

#include <jansson.h>

#include "user.h"


Usertable* newUserTable(int count) {
	Usertable *usertable = (Usertable*) malloc(sizeof(Usertable));
	usertable->size = count;
	usertable->users = (User**) calloc(count, sizeof(User*));

	for (int i = 0; i < usertable->size; i++) {
		usertable->users[i] = NULL;
	}

	return usertable;
}

User* getUser(Usertable* usertable, const char *username) {
	uint64_t hashIndex = hashf(username), startIndex;
	startIndex = hashIndex = hashIndex % usertable->size;

	while (usertable->users[hashIndex] != NULL) {


		if(!strcmp(usertable->users[hashIndex]->username, username)) {
			return *(usertable->users + hashIndex);
		} 

		++hashIndex;
		hashIndex %= usertable->size;
	}

	return NULL;
}

int8_t addUser(Usertable* usertable, User *user) 
{
	uint64_t hashIndex = hashf(user->username), startIndex;
	startIndex = hashIndex = hashIndex % usertable->size;
	

	while (usertable->users[hashIndex] != NULL && hashIndex != startIndex - 1) {
		hashIndex++;
		hashIndex %= usertable->size;
	}

	if (hashIndex == startIndex - 1) {
		return 1; // Dictionary full
	} else {
		usertable->users[hashIndex] = user;
		initEventlist(usertable->users[hashIndex]->eventlist);
	}
		
	return 0;
}


/*
 * DESCR:
 * Dumps table contents in json format. 
 *
 * ARGS:
 * Takes Usertable pointer as argument
 */
char* dumpTable(Usertable *usertable)
{
	json_t *dump = json_array();
	for (int i = 0; i < usertable->size; i++) {
		if (usertable->users[i] != NULL) {
			json_t *userHandle = json_object();
			printf("Username: %s, password: %s\n", usertable->users[i]->username, usertable->users[i]->password);
			json_object_set_new(userHandle, "username", json_string(usertable->users[i]->username));
			json_object_set_new(userHandle, "password", json_string(usertable->users[i]->password));
			json_array_append_new(dump, userHandle);
		}
	}
	return json_dumps(dump, 0);
}

/*
 * DESCR:
 * Imports table from string in json format
 *
 * ARGS:
 * Usertable reference to be filled
 * String containing json data to be imported
 */
int importTable(Usertable *usertable, char *dump)
{
	json_t *parsedDump = json_array();
	parsedDump = json_loads(dump, 0, NULL);

	if (parsedDump == NULL) {
		fprintf(stderr, "Corrupted user database, failed to load\n");
		return -1;
	}

	size_t index;
	json_t *value;

	printf("%s\n", json_dumps(parsedDump, 0));

	json_array_foreach(parsedDump, index, value) {
		printf("VALUE: %s\n", json_dumps(value, 0));
		printf("VALUE username: %s\n", json_string_value(json_object_get(value, "username")));
		printf("VALUE password: %s\n", json_string_value(json_object_get(value, "password")));

		User *user = malloc(sizeof(User));
		user->username = (char*) json_string_value(json_object_get(value, "username"));
		user->password = (char*) json_string_value(json_object_get(value, "password"));
		addUser(usertable, user);
	}

	printTable(usertable);
	return 0;
}


int printTable(Usertable *usertable)
{
	printf("[");
	for (int i = 0; i < usertable->size; i++) {
		if (usertable->users[i] != NULL) {
			printf("{ \"username\": \"%s\", \"password\": \"%s\" }, \n", usertable->users[i]->username, usertable->users[i]->password);
		}
	}
	printf("]");
	return 0;
}

uint64_t hashf(const char *str) 
{
	uint64_t hash = 5381;
	int32_t c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}
