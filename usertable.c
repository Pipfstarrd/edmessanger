#include <stdlib.h>
#include <string.h>

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

User* getUser(Usertable* usertable, uint8_t *username) {
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


uint64_t hashf(uint8_t *str) 
{
	uint64_t hash = 5381;
	int32_t c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}
