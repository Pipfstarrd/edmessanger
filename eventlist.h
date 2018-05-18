#ifndef EVENTLIST_H
#define EVENTLIST_H
#include <stdint.h>


typedef struct Eventlist {
	uint8_t           *event;
	uint8_t           *message;
	struct  Eventlist *next;
} Eventlist;

int initEventlist(Eventlist *eventlist);
int addEvent(Eventlist **eventlist, char *event, char *msg);
int removeEvent(Eventlist **eventlist, Eventlist *removable);

#endif /* EVENTLIST_H */
