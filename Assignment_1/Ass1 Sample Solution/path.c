/* * * * * * *
 * Module for storing, manipulating, and printing simple paths (sequences of 
 * towns, separated by distances)
 * 
 * It was entirely possible to do this assignment without making additional
 * data structures like this (just using arrays/stacks/queues) but I found
 * having a dedicated path module helped me make my solution a lot simpler
 *
 * created for COMP20007 Design of Algorithms - Assignment 1, 2017
 * by Matt Farrugia <matt.farrugia@unimelb.edu.au>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "stack.h"

#include "path.h"

// a path stores towns and the distances between them (in parallel stacks)
// it also stores the total distance of the towns in the sequence
// 
// it also stores a boolean array to track which towns are in the path,
// and the number of towns in the graph the paths are from (for bounds checking)
struct path {
	Stack *towns; // stack of towns: (bottom) start, town, ..., destination
	Stack *dists; // stack of distances: (bottom) 0, dist1, ..., total_dist
	int    total; // the current total distance

	bool *inpath; // which towns are in path right now?
	int   ntowns; // how many towns total (used for bounds checking)
};


// create a new path starting from starting_town, for a graph with ntowns total
// towns (i.e. the highest permissible town id is ntowns-1)
// return the address of the new town
Path *new_path(int starting_town, int ntowns) {
	Path *path = malloc(sizeof *path);
	assert(path);

	path->towns = new_stack();
	path->dists = new_stack();
	path->total = 0;

	path->ntowns = ntowns;
	path->inpath = malloc(ntowns * sizeof (bool));
	assert(path->inpath);
	for (int i = 0; i < ntowns; i++) {
		path->inpath[i] = false;
	}
	
	path_add(path, starting_town, 0);

	return path;
}

// create a new path object which is a copy of 'path'. That is, it contains
// the same towns and distances. return the address of this new path.
Path *path_clone(Path *path) {
	assert(path != NULL);

	// reverse stacks first, to copy in forwards order
	Stack *towns = new_stack();
	Stack *dists = new_stack();
	while (stack_size(path->towns) > 0) {
		stack_push(towns, stack_pop(path->towns));
		stack_push(dists, stack_pop(path->dists));
	}

	// copy over the starting town
	int town = stack_pop(towns);
	int dist = stack_pop(dists);
	Path *copy = new_path(town, path->ntowns);

	// and put them back into the original path's stacks
	stack_push(path->towns, town);
	stack_push(path->dists, dist);

	// now add the rest of the towns and their dists to the new stack and
	// also rebuild the original path stack
	while (stack_size(towns) > 0) {
		town = stack_pop(towns);
		dist = stack_pop(dists);

		path_add(copy, town, dist);

		// and rebuild the path stacks
		stack_push(path->dists, dist);
		stack_push(path->towns, town);
	}

	// free the temp stacks, we are finished with them
	free_stack(towns);
	free_stack(dists);

	// finally, return the new path we built
	return copy;
}
Path *new_path(int starting_town, int ntowns);

// delete a path and all of its memory
void free_path(Path *path) {
	assert(path != NULL);
	free_stack(path->towns);
	free_stack(path->dists);
	free(path->inpath);
	free(path);
}


// add a new step to the end of a path: a step to 'town' with distance 'dist'
void path_add(Path *path, int town, int dist) {
	assert(path != NULL);
	assert(0 <= town && town < path->ntowns);

	stack_push(path->dists, dist);
	stack_push(path->towns, town);
	path->total += dist;
	path->inpath[town] = true;
}

// remove the trailing step from the end of a path
void path_pop(Path *path) {
	assert(path != NULL);
	assert(stack_size(path->towns) > 0);

	path->total -= stack_pop(path->dists);
	path->inpath[stack_pop(path->towns)] = false;
}

// is this town currently in the path 'path'? this operation is O(1)
// (made possible by maintaining a boolean array of which towns are in the path)
bool path_has(Path *path, int town) {
	assert(path != NULL);
	assert(0 <= town && town < path->ntowns);

	return path->inpath[town];
}

// how long is this path currently?
int path_distance(Path *path) {
	assert(path != NULL);
	return path->total;
}


// print a path, one town per line, including cumulative distances (in km)
// e.g.:
// Cooma (0km)
// Bega (110km)
// Bairnsdale (435km)
// 
void path_print_detailed(Path *path, Graph *graph) {
	assert(path != NULL);

	// reverse stacks first, to print in forwards order
	Stack *towns = new_stack();
	while (stack_size(path->towns) > 0) {
		stack_push(towns, stack_pop(path->towns));
	}
	Stack *dists = new_stack();
	while (stack_size(path->dists) > 0) {
		stack_push(dists, stack_pop(path->dists));
	}

	// print the path
	int cumulative_dist = 0;
	while (stack_size(towns) > 0) {
		int dist = stack_pop(dists);
		int town = stack_pop(towns);

		// print the next town
		cumulative_dist += dist; // will start at 0
		printf("%s (%dkm)\n", graph->vertices[town]->label, cumulative_dist);

		// rebuild the path stacks
		stack_push(path->dists, dist);
		stack_push(path->towns, town);
	}

	// finished! free everything we allocated
	free_stack(towns);
	free_stack(dists);
}


// print a path, all on one line, separated by command and spaces
// e.g.:
// Cooma, Bega, Bairnsdale
// 
void path_print(Path *path, Graph *graph) {
	assert(path != NULL);

	// reverse town stack first, to print in forwards order
	Stack *towns = new_stack();
	while (stack_size(path->towns) > 0) {
		stack_push(towns, stack_pop(path->towns));
	}

	// print the first town
	int town = stack_pop(towns);
	printf("%s", graph->vertices[town]->label);
	stack_push(path->towns, town);

	// print the remaining towns
	while (stack_size(towns) > 0) {
		int town = stack_pop(towns);
		printf(", %s", graph->vertices[town]->label);

		// rebuild the path town stack
		stack_push(path->towns, town);
	}
	printf("\n");

	// finished! free everything we allocated
	free_stack(towns);
}

// print a path, all on one line, including its total distance (in km)
// e.g.:
// Cooma, Bega, Bairnsdale (435km)
// 
void path_print_distance(Path *path, Graph *graph) {
	assert(path != NULL);

	// reverse town stack first, to print in forwards order
	Stack *towns = new_stack();
	while (stack_size(path->towns) > 0) {
		stack_push(towns, stack_pop(path->towns));
	}

	// print the first town
	int town = stack_pop(towns);
	printf("%s", graph->vertices[town]->label);
	stack_push(path->towns, town);

	// print the remaining towns
	while (stack_size(towns) > 0) {
		int town = stack_pop(towns);
		printf(", %s", graph->vertices[town]->label);

		// rebuild the path town stack
		stack_push(path->towns, town);
	}
	printf(" (%dkm)\n", path->total);

	// finished! free everything we allocated
	free_stack(towns);
}
