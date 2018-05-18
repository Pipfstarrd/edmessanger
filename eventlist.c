#include <stddef.h>
#include <stdlib.h>


#include "eventlist.h"


int initEventlist(Eventlist *eventlist) 
{
	eventlist = NULL;
	return 0;
}

int addEvent(Eventlist **eventlist, char *event, char *msg)
{
	Eventlist *new = malloc(sizeof(Eventlist));

	new->event   = event;
	new->message = (uint8_t *) msg;

	if (*eventlist == NULL) {
		*eventlist = new;
		return 0;
	}

	while ((*eventlist)->next != NULL) {
		*eventlist = (*eventlist)->next;
	}

	(*eventlist)->next = new;
	return 0;
}

int removeEvent(Eventlist **eventlist, Eventlist *removable)
{
	Eventlist *prev, *cur,  *next;
	prev = NULL, cur = *eventlist, next = (*eventlist)->next;
	

	while(cur != removable) {
		prev = cur;
		cur  = next;
		next = prev->next;
	}

	if (prev != NULL) {
		prev->next = next;
	} else { /* If prev == NULL, we need to delete the first element, 
	            we do that by updating the pointer */
		*eventlist = (*eventlist)->next;
	}
	return 0;
}
