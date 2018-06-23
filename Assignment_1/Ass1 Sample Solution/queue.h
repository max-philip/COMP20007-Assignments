/* * * * * * *
 * Module for creating and manipulating queues of integers
 *
 * created for COMP20007 Design of Algorithms 2017
 * by Matt Farrugia <matt.farrugia@unimelb.edu.au>
 */

typedef struct queue Queue;

// create a new queue and return its pointer
Queue *new_queue();

// destroy a queue and its associated memory
void free_queue(Queue *queue);

// insert a new item of data at the back of a queue. O(1).
void queue_enqueue(Queue *queue, int data);

// remove and return the item of data at the front of a queue. O(1).
// error if the queue is empty (so first ensure queue_size() > 0)
int queue_dequeue(Queue *queue);

// return the number of items currently in a queue
int queue_size(Queue *queue);