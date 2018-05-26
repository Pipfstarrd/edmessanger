#ifndef USER_H /* Include guard */
#define USER_H


#include <unistd.h>
#include <stdint.h>

#include "eventlist.h"

typedef struct {
	char      *username;
	char      *password;
//	uint8_t status;
	char      *token;
	int32_t   *fd;
	uint64_t  hash;
	Eventlist *eventlist;
} User;

typedef struct {
	uint16_t size;
	User     **users;
} Usertable;


Usertable* newUserTable (int);
User*      getUser      (Usertable*, const char*);
int8_t     addUser      (Usertable*, User*);
char*      dumpTable    (Usertable*);
int        importTable  (Usertable*, char*);
int        printTable   (Usertable*);
uint64_t   hashf        (const char*);

#endif /* USER_H */
