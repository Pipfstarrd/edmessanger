#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>


#include "eventlist.h"


int initEventlist(Eventlist **eventlist) 
{
	*eventlist = NULL;
	return 0;
}

int addEvent(Eventlist **eventlist, const char *event, const char *msg, const char *sender)
{
	if (event == NULL || msg == NULL || sender == NULL) {
		printf("Passed NULL as argument to addEvent, check your code\n");
		return -1;
	}
	Eventlist *new = malloc(sizeof(Eventlist));

	new->event   = event;
	new->message = msg;
	new->sender  = sender;
	new->next    = NULL;

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
		/*
		 * (0: prev, prev->next == cur) ; (1: cur, cur->next == next) ; (2: next, next->next)
		 * (0: cur, cur->next)          ; (1: next, next->next)       ; (2: next->next)
		 */
		prev = cur;
		cur  = next;
		next = cur->next; // next->next
	}

	if (prev != NULL) {
		prev->next = next;
	} else { /* If prev == NULL, we need to delete the first element, 
	            we do that by updating the pointer */
		*eventlist = (*eventlist)->next;
	}
	return 0;
}
