/* ***************************************************************************
 Program written by Max Philip for Assignment 1 of the Design of Algorithms
 (COMP20007) subject, Semester 1, 2017.

 Parts 1 and 2 perform a depth-first and breadth-first traversal from a source
 vertex on the input graph, respectively. Part 3 finds a simple (unique) path
 from a source vertex to a destination vertex, as well as the cumulative
 distance. Part 4 finds all possible unique paths, and part 5 determines the
 shortest path from a source vertex to a destination by total distance in km.

*************************************************************************** */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "traverse.h"
#include "list.h"

/* ************************************************************************ */

/* FUNCTION PROTOTYPES */

void print_dfs(Graph* graph, int source_id);
void print_bfs(Graph* graph, int source_id);
void detailed_path(Graph* graph, int source_id, int destination_id);
void all_paths(Graph* graph, int source_id, int destination_id);
void shortest_path(Graph* graph, int source_id, int destination_id);
int checkarray(int val, int *A, int size);
int path_recur_first(Graph* graph, int start, int end, int *stack,
			int stack_n, int *discovered, Edge* curr_edge, int cumul_dist,
									int is_first, int *dists, int dist_ind);
void path_recur(Graph* graph, int start, int end, int *stack, int stack_n,
															int *discovered);
int path_recur2(Graph* graph, int start, int end, int *stack, int stack_n,
		int *discovered, Edge* curr_edge, int cumul_dist, int min, int *array);

/* ************************************************************************ */

/* FUNCTION DEFINITIONS */

/* print part 1 output showing depth-first traversal of the input graph
   starting at the vertex indexed by source_id.
   (this implementation uses an explicit stack in place of recursion, as per
   the first Bonus Mark)*/
void print_dfs(Graph* graph, int source_id) {

	/* set the current edge to be the first edge of the source vertex */
	Edge* curr_edge = graph->vertices[source_id]->first_edge;

	int curr_vert = source_id;
	int total_n=0;

	/* no. of discovered vertices cannot exceed number of vertices in graph */
	int discovered[graph->n];

	/* initialise the stack in a singly-linked list representation */
	List *stack = new_list();

	/* do while loop is used as we need the current vertex to be updated once
	   after the condition is no longer satisfied */
	do {
		/* push the current vertex to the stack, then print it */
		list_add_end(stack, curr_vert);
		printf("%s\n", graph->vertices[curr_vert]->label);

		/* set current vertex as discovered*/
		discovered[total_n] = curr_vert;
		total_n ++;

		/* sets curr_vert to the next vertex is the next vertex is not yet in
		   the discovered array */
		if (!checkarray(curr_edge->v, discovered, total_n)){
			curr_vert = curr_edge->v;

			/* update the current edge for checking the following vertices */
			curr_edge = graph->vertices[curr_vert]->first_edge;
		} else {
			/* iterate over each adjacent vertex to the current vertex */
			while (curr_edge != NULL){

				/* when the current edge is the last, the current vertex is
				   popped from the stack, current vertex and edge are updated
				   to fit the last element of the stack */
				if ((curr_edge->next_edge == NULL)){
					list_remove_end(stack);
					curr_vert = stack->tail->data;
					curr_edge = graph->vertices[curr_vert]->first_edge;
				}

				/* if the vertex pointed to be the next edge of the current
				   is not visited, then it is the next vertex */
				if (!checkarray(curr_edge->next_edge->v, discovered, total_n)){
					curr_edge = curr_edge->next_edge;
					curr_vert = curr_edge->v;

					/* current edge is the first edge of the current vertex */
					curr_edge = graph->vertices[curr_vert]->first_edge;
					break;
				} else {
					/* set current edge to the adjacent edge of that vertex */
					curr_edge = curr_edge->next_edge;
				}
			}
		}
	/* ends loop when the number of vertices visted is one less than number of
	   vertices in graph, as the loop is entered once more time */
	} while (total_n != graph->n - 1);

	/* prints final vertex */
	printf("%s\n", graph->vertices[curr_vert]->label);

	/* destroy the stack and free its memory once finished*/
	free_list(stack);
}


/* ************************************************************************ */


/* print part 2 output showing breadth-first traversal of the input graph
   starting at the vertex indexed by source_id */
void print_bfs(Graph* graph, int source_id) {

	/* number of current visted vertices cannot exceed total number of vertices
	   in the graph */
	int discovered[graph->n];
	int i = 0, curr_vert = source_id;
	Edge* curr_edge = NULL;


	/* set all vertices to not discovered */
	for (i = 0; i <= graph->n; i++){
		discovered[i] = 0;
	}

	/* initialise the queue for bfs in a singly-linked list representation */
	List *queue = new_list();

	/* enqueue the source vertex, set it to discovered and print */
	list_add_end(queue, source_id);
	discovered[source_id] = 1;
	printf("%s\n", graph->vertices[source_id]->label);

	while (queue->head != NULL){

		/* dequeue the current vertex and set edge to the it's first edge */
		curr_vert = list_remove_start(queue);
		curr_edge = graph->vertices[curr_vert]->first_edge;

		/* iterate over the current vertex's adjacent vertices */
		while (curr_edge != NULL){

			/* if the adjacent vertex is not discovered, enqueue and set to
			   discovered */
			if (!discovered[curr_edge->v]){
				list_add_end(queue, curr_edge->v);
				discovered[curr_edge->v] = 1;
				/* print as it is now a discovered vertex */
				printf("%s\n", graph->vertices[curr_edge->v]->label);
			}
			/* go to the next edge of the vertex */
			curr_edge = curr_edge->next_edge;
		}
	}

	/*destroy the queue and free its memory once finished */
	free_list(queue);
}


/* ************************************************************************ */


/* print part 3 output, any simple path from the source_id vertex to the
   destination_id vertex including cumulative distance at each vertex */
void detailed_path(Graph* graph, int source_id, int destination_id) {

	int discovered[graph->n], stack[graph->n], stack_n = 0, i = 0;
	Edge* curr_edge = NULL;

	int cumul_dist=0;

	/* is updated one the current path is no longer the first */
	int is_first = 1;

	/* number of distances in current shortest path cannot exceed the total
	   number of vertices in the graph */
	int dists[graph->n];
	int dist_ind = 0;

	/* first distance is always 0km */
	dists[dist_ind] = 0;
	dist_ind++;

	/* set all vertices to not discovered */
	for (i = 0; i <= graph->n; i++){
		discovered[i] = 0;
	}

	/* whether the current path is the first gets updated within the recursive
	   helper function */
	is_first = path_recur_first(graph, source_id, destination_id, stack,
	stack_n, discovered, curr_edge, cumul_dist, is_first, dists, dist_ind);
}


/* ************************************************************************ */


/* print part 4 output of every possible unique path between the source_id and
   destination_id vertices, in the input graph */
void all_paths(Graph* graph, int source_id, int destination_id) {

	/* set number of possible to be discovered vertices to the number of
	   vertices in the graph */
	int discovered[graph->n];

	/* initialise stack in array representation */
	int stack[graph->n], stack_n = 0, i = 0;

	/* set all vertices to not discovered */
	for (i = 0; i <= graph->n; i++){
		discovered[i] = 0;
	}

	/* recursive function prints all paths */
	path_recur(graph, source_id, destination_id, stack, stack_n, discovered);
}


/* ************************************************************************ */


/* prints the path of shortest distance (km) between the source_id and
   destination_id vertices, in the input graph */
void shortest_path(Graph* graph, int source_id, int destination_id) {

	/* number of discovered vertices and vertices in stack cannot exceed the
	   total number of vertices in the graph */
	int discovered[graph->n], stack[graph->n], stack_n = 0, i = 0;
	Edge* curr_edge = NULL;

	int cumul_dist=0;

	/* set the minimum total path distance to the highest possible int,
	   ensuring that it is then first set to the first path distance later */
	int min = INT_MAX;

	/* number of vertices in current shortest path cannot exceed the total
	   number of vertices in the graph */
	int c_short[graph->n];

	/* set all vertices to not discovered */
	for (i = 0; i <= graph->n; i++){
		discovered[i] = 0;
	}

	/* min is updated at each recursive call */
	min = path_recur2(graph, source_id, destination_id, stack, stack_n,
								discovered, curr_edge, cumul_dist, min, c_short);

	/* prints the vertices of shortest path by km once every possible path has
	   been compared */
	for (i = 1; i <= c_short[0]+1; i++){
		if (i == c_short[0]+1){
			printf("%s ", graph->vertices[c_short[i]]->label);
			break;
		}
		printf("%s, ", graph->vertices[c_short[i]]->label);
	}

	/* prints final min value */
	printf("(%dkm)\n", min);
}


/* ************************************************************************ */


/* checks if the input val is an element of the input array, A */
int checkarray(int val, int *A, int size){
	int i;

	/* returns 1 if val is equal to any element of the input array */
	for (i=0; i < size; i++) {
		if (A[i] == val){
			return 1;
		}
	}
	return 0;
}


/* ************************************************************************ */


/* Used in the detailed_path (part 3) function. Is adapted from the path_recur2
   function below, used in all_paths (part 5). Takes the first simple path as
   the detailed path, instead of using dfs, avoiding any overlap */
int path_recur_first(Graph* graph, int start, int end, int *stack,
			int stack_n, int *discovered, Edge* curr_edge, int cumul_dist,
									int is_first, int *dists, int dist_ind){

	int i, next_vert;

	/* push current vertex to the stack, and set it to discovered status */
	stack[stack_n] = start;
	stack_n ++;
	discovered[start] = 1;

	/* update the array of distances only if it is in the first simple path */
	if ((curr_edge)&&(is_first)){
		cumul_dist += curr_edge->weight;
		dists[dist_ind] = cumul_dist;
		dist_ind++;
	}

	/* when current path is the first path and the current vertex is equal to
	   the destination, start printing each vertex in first path along with
	   cumalative distance from the dists array */
	if ((start == end)&&(is_first)){
		for(i = 0; i < stack_n; i++){
			if (stack[i] == end){
				printf("%s (%dkm)\n", graph->vertices[stack[i]]->label, cumul_dist);
				is_first = 0;
				break;
			}
			printf("%s (%dkm)\n", graph->vertices[stack[i]]->label, dists[i]);
		}
	} else {
		/* set the current edge to the first edge of the current vertex, to
		   allow for accessing all adjacent vertices */
		curr_edge = graph->vertices[start]->first_edge;

		/* run recursively for each vertex adjacent to the current vertex */
		while (curr_edge != NULL){
			next_vert = curr_edge->v;
			if (!discovered[next_vert]){
				is_first = path_recur_first(graph, next_vert, end, stack,
	   stack_n, discovered, curr_edge, cumul_dist, is_first, dists, dist_ind);
			}
			/* go to next edge for next adjacent vertex */
			curr_edge = curr_edge->next_edge;
		}
	}
	/* set the current vertex back to unvisited */
	discovered[start] = 0;

	/* decrement the stack index, effectively popping the current vertex from
	   the stack */
	stack_n --;

	/* returns if current path is the first path, as we only want the first */
	return is_first;
}


/* ************************************************************************ */


/* Recursive helper function used in the all_paths (part 4) function. Prints
   all paths from the initial vertex (start) to the destination (end). Stores
   the already visited vertices in an array, and current path in a stack */
void path_recur(Graph* graph, int start, int end, int *stack, int stack_n,
															int *discovered){
	int i, next_vert;
	Edge* curr_edge = graph->vertices[start]->first_edge;

	/* push current vertex to the stack, and set it to discovered status */
	stack[stack_n] = start;
	stack_n ++;
	discovered[start] = 1;

	/* compare current path to current shortest when the current vertex is the
	   destination */
	if (start == end){
		for(i = 0; i < stack_n; i++){
			if (stack[i] == end){

				/* newline after the last vertex in current path */
				printf("%s\n", graph->vertices[stack[i]]->label);
				break;
			}
			printf("%s, ", graph->vertices[stack[i]]->label);
		}
	} else {

		/* set the current edge to the first edge of the current vertex, to
		   allow for accessing all adjacent vertices */
		curr_edge = graph->vertices[start]->first_edge;

		/* run recursively for each vertex adjacent to the current vertex */
		while (curr_edge != NULL){
			next_vert = curr_edge->v;
			if (!discovered[next_vert]){
				path_recur(graph, next_vert, end, stack, stack_n, discovered);
			}
			/* go to next edge for next adjacent vertex */
			curr_edge = curr_edge->next_edge;
		}
	}
	/* set the current vertex back to unvisited */
	discovered[start] = 0;

	/* decrement the stack index, effectively popping the current vertex from
	   the stack */
	stack_n --;
}


/* ************************************************************************ */


/* Used in the shortest_path (part 5) function. Adapted from the path_recur
   function above. Finds the shortest path by brute force iterating over all
   possible unique paths until shortest path by total km is found */
int path_recur2(Graph* graph, int start, int end, int *stack, int stack_n,
	int *discovered, Edge* curr_edge, int cumul_dist, int min, int *c_short){
	int i, next_vert, j;

	/* push current vertex to the stack, and set it to discovered status */
	stack[stack_n] = start;
	stack_n ++;
	discovered[start] = 1;

	/* add to the current cumalative distance when the current edge is not
	   NULL */
	if (curr_edge){
		cumul_dist += curr_edge->weight;
	}

	/* print the current path when the current vertex is the destination */
	if (start == end){
		for(i = 0; i < stack_n; i++){
			if (stack[i] == end){

				/* update the minimum total distance when the current distance
				   is less than the current minimum */
				if (cumul_dist <= min){
				 	min = cumul_dist;

					/* set first element of array to the number of vertices in
					   the current shortest path, for printing at the end */
					c_short[0] = i;

					/* update array of current shortest path vertices */
					for (j = 1; j <= i+1; j++){
						c_short[j] = stack[j-1];
					}
				}
				break;
			}
		}
	} else {
		/* set the current edge to the first edge of the current vertex, to
		   allow for accessing all adjacent vertices */
		curr_edge = graph->vertices[start]->first_edge;

		/* run recursively for each vertex adjacent to the current vertex */
		while (curr_edge != NULL){
			next_vert = curr_edge->v;
			if (!discovered[next_vert]){
				min = path_recur2(graph, next_vert, end, stack, stack_n,
								discovered, curr_edge, cumul_dist, min, c_short);
			}
			/* go to next edge for next adjacent vertex */
			curr_edge = curr_edge->next_edge;
		}
	}
	/* set the current vertex back to unvisited */
	discovered[start] = 0;

	/* decrement the stack index, effectively popping the current vertex from
	   the stack */
	stack_n --;

	/* return the current minimum distance (km), for use in shortest_path */
	return min;
}


/* ************************************************************************ */
