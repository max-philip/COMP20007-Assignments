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

#ifndef PATH_H
#define PATH_H

#include "graph.h"

typedef struct path Path;

// create a new path starting from starting_town, for a graph with ntowns total
// towns (i.e. the highest permissible town id is ntowns-1)
// return the address of the new town
Path *new_path(int starting_town, int ntowns);

// create a new path object which is a copy of 'path'. That is, it contains
// the same towns and distances. return the address of this new path.
Path *path_clone(Path *path);

// delete a path and all of its memory
void free_path(Path *path);



// add a new step to the end of a path: a step to 'town' with distance 'dist'
void path_add(Path *path, int town, int dist);

// remove the trailing step from the end of a path
void path_pop(Path *path);

// is this town currently in the path 'path'? this operation is O(1)
// (made possible by maintaining a boolean array of which towns are in the path)
bool path_has(Path *path, int town);

// how long is this path currently?
int path_distance(Path *path);



// print a path, one town per line, including cumulative distances (in km)
// e.g.:
// Cooma (0km)
// Bega (110km)
// Bairnsdale (435km)
// 
void path_print_detailed(Path *path, Graph *graph);

// print a path, all on one line, separated by command and spaces
// e.g.:
// Cooma, Bega, Bairnsdale
// 
void path_print(Path *path, Graph *graph);

// print a path, all on one line, including its total distance (in km)
// e.g.:
// Cooma, Bega, Bairnsdale (435km)
// 
void path_print_distance(Path *path, Graph *graph);

#endif