///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT List μέσω συνδεδεμένης λίστας.
//
///////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "ADTList.h"


// Ενα List είναι pointer σε αυτό το struct
struct list {
	ListNode dummy;				// χρησιμοποιούμε dummy κόμβο, ώστε ακόμα και η κενή λίστα να έχει έναν κόμβο.
	ListNode last;				// δείκτης στον τελευταίο κόμβο, ή στον dummy (αν η λίστα είναι κενή)
	int size;					// μέγεθος, ώστε η list_size να είναι Ο(1)
	DestroyFunc destroy_value;	// Συνάρτηση που καταστρέφει ένα στοιχείο της λίστας.
};

struct list_node {
	ListNode next;		// Δείκτης στον επόμενο
	Pointer value;		// Η τιμή που αποθηκεύουμε στον κόμβο
};


List list_create(DestroyFunc destroy_value) {
	// Πρώτα δημιουργούμε το stuct
	List list = malloc(sizeof(*list));
	list->size = 0;
	list->destroy_value = destroy_value;

	// Χρησιμοποιούμε dummy κόμβο, ώστε ακόμα και μια άδεια λίστα να έχει ένα κόμβο
	// (απλοποιεί τους αλγορίθμους). Οπότε πρέπει να τον δημιουργήσουμε.
	//
	list->dummy = malloc(sizeof(*list->dummy));
	list->dummy->next = NULL;		// άδεια λίστα, ο dummy δεν έχει επόμενο

	// Σε μια κενή λίστα, τελευταίος κόμβος είναι επίσης ο dummy
	list->last = list->dummy;

	return list;
}

int list_size(List list) {
	return list->size;
}

void list_insert_next(List list, ListNode node, Pointer value) {
	// Αν το node είναι NULL απλά εισάγουμε μετά τον dummy κόμβο!
	// Αυτή ακριβώς είναι η αξία του dummy, δε χρειαζόμαστε ξεχωριστή υλοποίηση.
	if (node == NULL)
		node = list->dummy;

	// Δημιουργία του νέου κόμβου
	ListNode new = malloc(sizeof(*new));
	new->value = value;

	// Σύνδεση του new ανάμεσα στο node και το node->next
	new->next = node->next;
	node->next = new;

	// Ενημέρωση των size & last
	list->size++;
	if (list->last == node)
		list->last = new;
}

void list_remove_next(List list, ListNode node) {
	// Αν το node είναι NULL απλά διαγράφουμε μετά τον dummy κόμβο!
	// Αυτή ακριβώς είναι η αξία του dummy, δε χρειαζόμαστε ξεχωριστή υλοποίηση.
	if (node == NULL)
		node = list->dummy;

	// Ο κόμβος προς διαγραφή είναι ο επόμενος του node, ο οποίος πρέπει να υπάρχει
	ListNode removed = node->next;
	assert(removed != NULL);		// LCOV_EXCL_LINE

	if (list->destroy_value != NULL)
		list->destroy_value(removed->value);

	// Σύνδεση του node με τον επόμενο του removed
	node->next = removed->next;		// πριν το free!

	free(removed);

	// Ενημέρωση των size & last
	list->size--;
	if (list->last == removed)
		list->last = node;
}

Pointer list_find(List list, Pointer value, CompareFunc compare) {
	ListNode node = list_find_node(list, value, compare);
	return node == NULL ? NULL : node->value;
}

DestroyFunc list_set_destroy_value(List list, DestroyFunc destroy_value) {
	DestroyFunc old = list->destroy_value;
	list->destroy_value = destroy_value;
	return old;
}

void list_destroy(List list) {
	// Διασχίζουμε όλη τη λίστα και κάνουμε free όλους τους κόμβους,
	// συμπεριλαμβανομένου και του dummy!
	//
	ListNode node = list->dummy;
	while (node != NULL) {				// while αντί για for, γιατί θέλουμε να διαβάσουμε
		ListNode next = node->next;		// το node->next _πριν_ κάνουμε free!

		// Καλούμε τη destroy_value, αν υπάρχει (προσοχή, όχι στον dummy!)
		if (node != list->dummy && list->destroy_value != NULL)
			list->destroy_value(node->value);

		free(node);
		node = next;
	}

	// Τέλος free το ίδιο το struct
	free(list);
}


// Διάσχιση της λίστας /////////////////////////////////////////////

void print_list(List list) {
	for(ListNode lNode=list_first(list);
    lNode != LIST_EOF;
    lNode = list_next(list , lNode)
    ) {
        fprintf(stderr,"->%s\n",(char *)list_node_value(list, lNode));
    }
}

void print_args(List argList) {
	printf("\n");
	for(ListNode lNode=list_first(argList);
    lNode != LIST_EOF;
    lNode = list_next(argList , lNode)
    ) {
        print_list((List)list_node_value(argList, lNode));
		printf("\n");
    }
}

ListNode insert_alias_in_lists(List comList, ListNode prevComNode, List argList, char *alias) {
	// Take the first word of the separate command(separated with semicolumn)
    int aliasLength = strlen(alias);
    char *aliasCopy = (char *)calloc(aliasLength+1, sizeof(char));
	char *save=aliasCopy;
    strncpy(aliasCopy, alias, aliasLength);
	// char *restSC;
	char *token;
	int count=0;
	ListNode returnNode;

	// First remove the node next to previous command node 
	// print_list(comList);
	while((token = strtok_r(aliasCopy, " ", &aliasCopy) )) {
		if(count==0) {
			list_insert_next(comList, prevComNode, strdup(token));
			if(prevComNode==LIST_BOF)
				prevComNode=list_first(comList);
			else
				prevComNode=list_next(comList, prevComNode);
			list_remove_next(comList, prevComNode);
			returnNode=list_next(comList, prevComNode);
		}
		else {
			list_insert_next(argList, LIST_BOF, strdup(token));
		}
		count++;
	}

	free(save);
	return returnNode;
}

ListNode list_first(List list) {
	// Ο πρώτος κόμβος είναι ο επόμενος του dummy.
	//
	return list->dummy->next;
}

ListNode list_last(List list) {
	// Προσοχή, αν η λίστα είναι κενή το last δείχνει στον dummy, εμείς όμως θέλουμε να επιστρέψουμε NULL, όχι τον dummy!
	//
	if (list->last == list->dummy)
		return LIST_EOF;		// κενή λίστα
	else
		return list->last;
}

ListNode list_next(List list, ListNode node) {
	// assert(node != NULL);	// LCOV_EXCL_LINE (αγνοούμε το branch από τα coverage reports, είναι δύσκολο να τεστάρουμε το false γιατί θα κρασάρει το test)
	return (node != NULL) ? node->next : NULL;
	// return node->next;
}

Pointer list_node_value(List list, ListNode node) {
	// assert(node != NULL);	// LCOV_EXCL_LINE
	return (node != NULL) ? node->value : NULL;
}

ListNode list_find_node(List list, Pointer value, CompareFunc compare) {
	// διάσχιση όλης της λίστας, καλούμε την compare μέχρι να επιστρέψει 0
	//
	for (ListNode node = list->dummy->next; node != NULL; node = node->next)
		if (compare(value, node->value) == 0)
			return node;		// βρέθηκε

	return NULL;	// δεν υπάρχει
}