#ifndef USER_H /* Include guard */
#define USER_H


#include <unistd.h>

typedef struct {
	uint8_t *username;
	uint8_t *password;
	uint8_t status;
	uint8_t token[100];
	int32_t *fd;
	uint64_t hash;
} User;

typedef struct {
	uint16_t size;
	User     **users;
} Usertable;


Usertable* newUserTable(int count);
User* getUser(Usertable* usertable, uint8_t *username);
int8_t addUser(Usertable* usertable, User *user);
uint64_t hashf(uint8_t *str);

#endif /* USER_H */
