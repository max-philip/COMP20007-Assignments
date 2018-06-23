/* * * * * * *
 * Functions for traversing a graph representing a road network
 * 
 * Includes both stack-based and recursive depth-first search algorithm,
 * Also includes both all-paths and dijkstra's based shortest path algorithm
 *
 * There were many valid aproaches to these questions, these are a sample
 * If you have questions specific to your solution, you should speak with
 * your demonstrators or see me in consultation hours (details on LMS)
 *
 * created for COMP20007 Design of Algorithms - Assignment 1, 2017
 * by Matt Farrugia <matt.farrugia@unimelb.edu.au>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>

#include "stack.h"
#include "queue.h"
#include "path.h"

#include "traverse.h"

/* * * * * * * * * * HELPERS * * * * * * * * * */

bool *new_visited_array(int size) {
	bool *array = malloc((sizeof *array) * size);
	assert(array);
	for (int i = 0; i < size; i++) {
		array[i] = false;
	}
	return array;
}

/* * * * * * * * * * PART 1 * * * * * * * * * */

void print_dfs_stack(Graph* graph, int source_id);
void print_dfs_recursive(Graph* graph, int source_id);

void print_dfs(Graph *graph, int source_id) {
	
	// uncomment the one you want to test, comment the other out

	print_dfs_recursive(graph, source_id);
	// print_dfs_stack(graph, source_id);
}

/* * * * * * * * * * PART 1 (stack) * * * * * * * * * */

void print_dfs_stack(Graph* graph, int source_id) {
	
	// to store which nodes have been visited
	bool *visited = new_visited_array(graph->n);
	
	// to-be-visited towns, starting with source town
	Stack *stack = new_stack();
	stack_push(stack, source_id);

	while (stack_size(stack) > 0) {
		int town = stack_pop(stack);
		
		// if we have already visited this town, skip it
		if (visited[town]) {
			continue;
		}

		// otherwise, visit it, and push all neighbours
		visited[town] = true;
		printf("%s\n", graph->vertices[town]->label);

		Edge *edge = graph->vertices[town]->first_edge;
		while (edge) {
			// edge->v is the id of the town this edge takes us to
			stack_push(stack, edge->v);
			edge = edge->next_edge;
		}
	}

	// finished! don't forget to free things we allocated
	free(visited);
	free_stack(stack);
}

/* * * * * * * * * * PART 1 (recursion) * * * * * * * * * */

// visit a town and all of its unvisited neighbours
void dfs_explore(Graph *graph, int town, bool *visited);

// print the roads from the perspective of a DFS
void print_dfs_recursive(Graph* graph, int source_id) {
	
	// to store which nodes have been visited
	bool *visited = new_visited_array(graph->n);
	
	// start exploration from the source town
	dfs_explore(graph, source_id, visited);

	// finished! don't forget to free things we allocated
	free(visited);
}

void dfs_explore(Graph *graph, int town, bool *visited) {
	
	// base case: already visited (note: could put this check inside the loop)
	if (visited[town]) {
		return;
	}

	// recursive case: visit and then explore neighbours
	visited[town] = true;
	printf("%s\n", graph->vertices[town]->label);

	Edge *edge = graph->vertices[town]->first_edge;
	while (edge) {
		// edge->v is the id of the town this edge takes us to
		dfs_explore(graph, edge->v, visited);
		edge = edge->next_edge;
	}
}


/* * * * * * * * * * PART 2 * * * * * * * * * */

void print_bfs(Graph* graph, int source_id) {
	
	// to store which nodes have been visited
	bool *visited = new_visited_array(graph->n);
	
	// to-be-visited towns, starting with source town
	Queue *queue = new_queue();
	queue_enqueue(queue, source_id);

	while (queue_size(queue) > 0) {
		int town = queue_dequeue(queue);
		
		// if we have already visited this town, skip it
		if (visited[town]) {
			continue;
		}

		// otherwise, visit it, and enqueue all neighbours
		visited[town] = true;
		printf("%s\n", graph->vertices[town]->label);

		Edge *edge = graph->vertices[town]->first_edge;
		while (edge) {
			// edge->v is the id of the town this edge takes us to
			queue_enqueue(queue, edge->v);
			edge = edge->next_edge;
		}
	}

	// finished! don't forget to free things we allocated
	free(visited);
	free_queue(queue);
}


/* * * * * * * * * * PART 3 * * * * * * * * * */

// returns true if destination found, with resulting path in 'path'
// returns false if destination not found, with 'path' unchanged
bool find_destination(Graph* graph, int town, int destination, bool *visited, 
		Path *path);

void detailed_path(Graph* graph, int source_id, int destination_id) {

	// to store which nodes have been visited and the current path
	bool *visited = new_visited_array(graph->n);
	Path *path = new_path(source_id, graph->n);
	
	// start exploration from the source town
	find_destination(graph, source_id, destination_id, visited, path);

	// print the resulting path
	// note: assumes there will be a path (should be ok: network is connected)
	path_print_detailed(path, graph);

	// finished! don't forget to free things we allocated
	free(visited);
	free_path(path);
}

bool find_destination(Graph* graph, int town, int destination, bool *visited, 
		Path *path) {
	
	// base case 1: found the destination! report true
	if (town == destination) {
		return true;
	}
	// base case 2: already visited, report false (we've come this way before)
	// note: could alternatively put this check inside the loop of the prev call
	if (visited[town]) {
		return false;
	}

	// recursive case: visit and then explore neighbouring towns
	visited[town] = true;

	Edge *edge = graph->vertices[town]->first_edge;
	while (edge) {
		
		// lets add it to the path then continue to search from there
		path_add(path, edge->v, edge->weight);
		if (find_destination(graph, edge->v, destination, visited, path)) {
			// found the destination? okay, lets leave the path as-is and
			// report true all the way back up
			return true;
		} else {
			// not that way? okay, lets remove it from the path and try the 
			// next road
			path_pop(path);
		}
		edge = edge->next_edge;
	}

	// we couldn't find the destination through any neighbours! report false
	return false;
}


/* * * * * * * * * * PART 4 * * * * * * * * * */

void all_explore(Graph *graph, int node, int destination, Path *path);

void all_paths(Graph* graph, int source_id, int destination_id) {
	
	// to store the path
	Path *path = new_path(source_id, graph->n);

	// start exploration from the source town. prints paths along the way
	all_explore(graph, source_id, destination_id, path);

	// finished! don't forget to free things we allocated
	free_path(path);
}

void all_explore(Graph *graph, int town, int destination, Path *path) {

	// base case: destination found! print the current path
	if (town == destination) {
		path_print(path, graph);
		return;
	}

	// recursive case: add to path, then explore neighbours
	Edge *edge = graph->vertices[town]->first_edge;
	while (edge) {
		// if this town is already in the path, we should skip it
		if ( ! path_has(path, edge->v)) {

			// otherwise, add to path and continue to explore from there
			path_add(path, edge->v, edge->weight);
			all_explore(graph, edge->v, destination, path);
			
			// when we come back, remove it from the path again so that we
			// can come this way again later
			path_pop(path);
		}

		edge = edge->next_edge;
	}
}

/* * * * * * * * * * PART 5 * * * * * * * * * */

void shortest_path_all(Graph* graph, int source_id, int destination_id);
void shortest_path_dijkstra(Graph* graph, int source_id, int destination_id);

void shortest_path(Graph *graph, int source_id, int destination_id) {

	// uncomment the one you want to test, comment the other

	// shortest_path_all(graph, source_id, destination_id);
	shortest_path_dijkstra(graph, source_id, destination_id);
}

/* * * * * * * * * * PART 5 (min of all paths) * * * * * * * * * */

void all_explore_min(Graph *graph, int node, int destination, Path *current, 
		Path **min);

void shortest_path_all(Graph* graph, int source_id, int destination_id) {
	
	// to store the path, and which nodes are currently in the path
	Path *current = new_path(source_id, graph->n);
	// to store current best-known path and its distance
	Path *min = NULL;

	// start exploration from the source town
	all_explore_min(graph, source_id, destination_id, current, &min);

	// print the resulting min path
	// note: assumes there will be a path (should be ok: network is connected)
	path_print_distance(min, graph);

	// finished! don't forget to free things we allocated
	free_path(current);
	free_path(min); // we didn't allocate min but part of all_explore_min's
					// job is to find and create a min path so we must free it
}

void all_explore_min(Graph *graph, int node, int destination, Path *current,
		Path **min) {

	// base case: destination found! update the min path?
	if (node == destination) {
		if (*min == NULL) {
			// no path has been found yet, this is the first minimum path
			*min = path_clone(current);

		} else if (path_distance(current) < path_distance(*min)){
			// a shorter path has been found! replace the minimum path
			free_path(*min);
			*min = path_clone(current);
		}
		return;
	}

	// recursive case: add to path, then explore neighbours
	Edge *edge = graph->vertices[node]->first_edge;
	while (edge) {
		// if this town is already in the path, we should skip it
		if ( ! path_has(current, edge->v)) {

			// otherwise, add to path and continue to explore from there
			path_add(current, edge->v, edge->weight);
			all_explore_min(graph, edge->v, destination, current, min);

			// when we come back, remove it from the path again so that we
			// can come this way again later
			path_pop(current);

		}

		edge = edge->next_edge;
	}
}

/* * * * * * * * * * PART 5 (dijkstra) * * * * * * * * * */

#define INFINITY INT_MAX

int *new_int_array(int size, int default_value);
int next_nearest_town(bool *expanded, int *distances, int n);
void print_shortest_path(Graph *graph, Stack *path, int total_distance);

void shortest_path_dijkstra(Graph* graph, int source_id, int destination_id) {
	
	// to store the distances, previous ids, and which towns have been expanded
	int  *distance = new_int_array(graph->n, INFINITY);
	int  *previous = new_int_array(graph->n, -1);
	bool *expanded = new_visited_array(graph->n);

	// start search at source_id with distance 0
	distance[source_id] = 0;
	
	// repeatedly extract the next nearest town, and update its distances,
	// untill we reach the destination
	int town;
	while ((town = next_nearest_town(expanded, distance, graph->n)) != -1) {
		expanded[town] = true;
		if (town == destination_id) {
			break;
		}

		Edge *edge = graph->vertices[town]->first_edge;
		while (edge) {
			int dist = distance[edge->v];
			int new_dist = distance[town] + edge->weight;
			if (new_dist < dist) {
				distance[edge->v] = new_dist;
				previous[edge->v] = town;
			}
			edge = edge->next_edge;
		}
	}

	// assuming graph is connected, expanded[destination_id] == true now
	// so we should be able to reconstruct the path for printing
	Stack *path = new_stack();
	int node = destination_id;
	while (previous[node] != -1) {
		stack_push(path, node);
		node = previous[node];
	}
	stack_push(path, source_id);
	
	// now let's print the path
	print_shortest_path(graph, path, distance[destination_id]);
}

void print_shortest_path(Graph *graph, Stack *path, int total_distance) {
	// print the starting town
	int town = stack_pop(path);
	printf("%s", graph->vertices[town]->label);

	// print the remaining towns
	while (stack_size(path) > 0) {
		int town = stack_pop(path);
		printf(", %s", graph->vertices[town]->label);
	}

	// print the distance and end the line
	printf(" (%dkm)\n", total_distance);
}

int *new_int_array(int size, int default_value) {
	int *array = malloc((sizeof *array) * size);
	assert(array);
	for (int i = 0; i < size; i++) {
		array[i] = default_value;
	}
	return array;
}

int next_nearest_town(bool *expanded, int *distance, int n) {
	
	// start the min id at -1, to be returned if we don't find a town
	int min = -1;

	// look for a shorter distance
	for (int i = 0; i < n; i++) {
		if ( ! expanded[i]) { // (exclude towns that have already been expanded)

			// if we haven't got a town yet,
			// or if this town is nearer than the one we have,
			// update min
			if (min == -1 || distance[i] < distance[min]) {
				min = i;
			}

		}
	}

	// if all towns were expanded, min will still be -1
	return min;
}
