/* * * * * * *
 * Module for creating and manipulating stacks of integers
 *
 * created for COMP20007 Design of Algorithms 2017
 * by Matt Farrugia <matt.farrugia@unimelb.edu.au>
 */

#include <stdlib.h>
#include <assert.h>
#include "list.h"	// gives us access to everything defined in list.h
#include "stack.h"	// and we'll also need the prototypes defined in queue.h

// a stack is just a wrapper for a list of integers
// we will use the front of the list as the top of the stack (to take advantage
// of O(1) insert and remove operations)
struct stack {
	List *items;
};

/* * * *
 * FUNCTION DEFINITIONS
 */

// create a new stack and return its pointer
Stack *new_stack() {
	Stack *stack = malloc(sizeof *stack);
	assert(stack);

	stack->items = new_list();
	
	return stack;
}

// destroy a stack and its associated memory
void free_stack(Stack *stack) {
	assert(stack != NULL);
	// free the list of items, and the stack itself
	free_list(stack->items);
	free(stack);
}

// push a new item of data onto the top of a stack. O(1).
void stack_push(Stack *stack, int data) {
	assert(stack != NULL);
	// the 'start' of the list will be our access point
	list_add_start(stack->items, data);
}

// remove and return the top item of data from a stack. O(1).
// error if the stack is empty (so first ensure stack_size() > 0)
int stack_pop(Stack *stack) {
	assert(stack != NULL);
	assert(stack_size(stack) > 0);

	// the 'start' of the list will be our access point
	return list_remove_start(stack->items);
}

// return the number of items currently in a stack
int stack_size(Stack* stack) {
	assert(stack != NULL);
	// delegate straight to list size function
	return list_size(stack->items);
}