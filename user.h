#ifndef USER_H /* Include guard */
#define USER_H


#include <unistd.h>
#include <stdint.h>

#include "eventlist.h"

typedef struct {
	uint8_t *username;
	uint8_t *password;
	uint8_t status;
	uint8_t token[100];
	int32_t *fd;
	uint64_t hash;
	Eventlist *eventlist;
} User;

typedef struct {
	uint16_t size;
	User     **users;
} Usertable;


Usertable* newUserTable(int count);
User* getUser(Usertable* usertable, const char *username);
int8_t addUser(Usertable* usertable, User *user);
uint64_t hashf(const char *str);

#endif /* USER_H */
