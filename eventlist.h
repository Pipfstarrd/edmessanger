#ifndef EVENTLIST_H
#define EVENTLIST_H
#include <stdint.h>


typedef struct Eventlist {
	const   char      *event;
	const   char      *message;
	const   char      *sender;
	struct  Eventlist *next;
} Eventlist;

int initEventlist(Eventlist *eventlist);
int addEvent(Eventlist **eventlist, const char *event, const char *msg, const char *sender);
int removeEvent(Eventlist **eventlist, Eventlist *removable);

#endif /* EVENTLIST_H */
