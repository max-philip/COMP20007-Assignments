/* * * * * * *
 * Module for creating and manipulating stacks of integers
 *
 * created for COMP20007 Design of Algorithms 2017
 * by Matt Farrugia <matt.farrugia@unimelb.edu.au>
 */

typedef struct stack Stack;

// create a new stack and return its pointer
Stack *new_stack();

// destroy a stack and its associated memory
void free_stack(Stack *stack);

// push a new item of data onto the top of a stack. O(1).
void stack_push(Stack *stack, int data);

// remove and return the top item of data from a stack. O(1).
// error if the stack is empty (so first ensure stack_size() > 0)
int stack_pop(Stack *stack);

// return the number of items currently in a stack
int stack_size(Stack *stack);