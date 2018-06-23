/* * * * * * * * *
 * Dynamic hash table using cuckoo hashing, resolving collisions by switching
 * keys between two tables with two separate hash functions
 *
 * created for COMP20007 Design of Algorithms - Assignment 2, 2017
 * by Max Philip
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cuckoo.h"

void cuckoo_hash_table_print(CuckooHashTable *table);
bool cuckoo_hash_table_insert(CuckooHashTable *table, int64 key);
bool cuckoo_hash_table_lookup(CuckooHashTable *table, int64 key);


// an inner table represents one of the two internal tables for a cuckoo
// hash table. it stores two parallel arrays: 'slots' for storing keys and
// 'inuse' for marking which entries are occupied
typedef struct inner_table {
	int64 *slots;	// array of slots holding keys
	bool  *inuse;	// is this slot in use or not?
} InnerTable;

// a cuckoo hash table stores its keys in two inner tables
struct cuckoo_table {
	InnerTable *table1; // first table
	InnerTable *table2; // second table
	int size;			// size of each table
};


// initialise a cuckoo hash table with 'size' slots in each table
CuckooHashTable *new_cuckoo_hash_table(int size) {

	// error message taken from linear.c file
	assert(size < MAX_TABLE_SIZE && "error: table has grown too large!");

	CuckooHashTable *table = malloc(sizeof *table);
	//assert(table);

	//printf("fuckuasshole");

	table->table1 = malloc((sizeof *table->table1) * size);
	table->table2 = malloc((sizeof *table->table2) * size);

	table->table1->slots = malloc((sizeof *table->table1->slots) * size);
	assert(table->table1->slots);
	table->table2->slots = malloc((sizeof *table->table2->slots) * size);
	assert(table->table2->slots);

	table->table1->inuse = malloc((sizeof *table->table1->inuse) * size);
	assert(table->table1->inuse);
	table->table2->inuse = malloc((sizeof *table->table2->inuse) * size);
	assert(table->table2->inuse);

	// set all slots as not having a key
	int i;
	for (i = 0; i < size; i++) {
		table->table1->inuse[i] = false;
		table->table2->inuse[i] = false;
	}

	table->size = size;

	return table;
}


// free all memory associated with 'table'
void free_cuckoo_hash_table(CuckooHashTable *table) {
	assert(table != NULL);

	free(table->table1->slots);
	free(table->table2->slots);

	free(table->table1->inuse);
	free(table->table2->inuse);

	free(table->table1);
	free(table->table2);

	free(table);
}


CuckooHashTable *cuck_double_table(CuckooHashTable *table) {
	int64 *oldslots1 = table->table1->slots;
	int64 *oldslots2 = table->table2->slots;
	bool  *oldinuse1 = table->table1->inuse;
	bool  *oldinuse2 = table->table2->inuse;
	int oldsize = table->size;


	// new table as double the size
	table = new_cuckoo_hash_table(oldsize * 2);

	// insert all the old keys after doubling
	int i;
	for (i = 0; i < oldsize; i++){
		if (oldinuse1[i] == true){
			cuckoo_hash_table_insert(table, oldslots1[i]);
		}
		if (oldinuse2[i] == true){
			cuckoo_hash_table_insert(table, oldslots2[i]);
		}
	}

	// free allocated memory
	free(oldslots1);
	free(oldslots2);
	free(oldinuse1);
	free(oldinuse2);

	return table;
}


// insert 'key' into 'table', if it's not in there already
// returns true if insertion succeeds, false if it was already in there
bool cuckoo_hash_table_insert(CuckooHashTable *table, int64 key) {
	//fprintf(stderr, "not yet implemented\n");
	assert(table != NULL);

	//cuckoo_hash_table_print(table);

	int h = h1(key) % table->size;
	//printf("%llu: h1: %d,    h2: %d\n", key, h, h2(key)%table->size);

	int curr_inner_table = 1;

	int64 temp_key, curr_key = key;
	int loop=0;

	// make sure key is not already in table
	if (cuckoo_hash_table_lookup(table, key)){
		return false;
	}
	while (true){
		// exits while loop if a cycle has been confirmed
		if ((loop > table->size) && (key == curr_key)){
			break;
		}

		// either insert key or swap key in table 1
		if (curr_inner_table == 1){
			if (table->table1->inuse[h] == false){
				table->table1->slots[h] = key;
				table->table1->inuse[h] = true;
				return true;
			} else {
				temp_key = table->table1->slots[h];
				table->table1->slots[h] = key;
				key = temp_key;
				h = h2(key) % table->size;
				loop++;
			}
		}

		// either insert key or swap key in table 2
		if (curr_inner_table == 2){
			if (table->table2->inuse[h] == false){
				table->table2->slots[h] = key;
				table->table2->inuse[h] = true;
				return true;
			} else {
				temp_key = table->table2->slots[h];
				table->table2->slots[h] = key;
				key = temp_key;
				h = h1(key) % table->size;
				loop++;
			}
		}

		// change table for next iteration
		if (curr_inner_table == 1){
			curr_inner_table = 2;
		} else {
			curr_inner_table = 1;
		}
	}

	// double size of table if there is a cycle
	*table = *cuck_double_table(table);

	// finally insert the most recently inserted key
	cuckoo_hash_table_insert(table, key);
	return true;
}


// lookup whether 'key' is inside 'table'
// returns true if found, false if not
bool cuckoo_hash_table_lookup(CuckooHashTable *table, int64 key) {

	assert(table != NULL);

	int hash1 = h1(key) % table->size;
	int hash2 = h2(key) % table->size;

	if ((table->table1->slots[hash1] == key) || (table->table2->slots[hash2] == key)){
		return true;
	}

	return false;
}


// print the contents of 'table' to stdout
void cuckoo_hash_table_print(CuckooHashTable *table) {
	assert(table);
	printf("--- table size: %d\n", table->size);

	// print header
	printf("                    table one         table two\n");
	printf("                  key | address     address | key\n");

	// print rows of each table
	int i;
	for (i = 0; i < table->size; i++) {

		// table 1 key
		if (table->table1->inuse[i]) {
			printf(" %*llu ", 20, table->table1->slots[i]);
		} else {
			printf(" %*s ", 20, "-");
		}

		// addresses
		printf("| %-*d %*d |", 9, i, 9, i);

		// table 2 key
		if (table->table2->inuse[i]) {
			printf(" %-*u\n", 11, table->table2->slots[i]);
		} else {
			printf(" %s\n",  "-");
		}
	}

	// done!
	printf("--- end table ---\n");
}


// print some statistics about 'table' to stdout
void cuckoo_hash_table_stats(CuckooHashTable *table) {
	assert(table != NULL);
	printf("--- table stats ---\n");

	// print some information about the table
	printf("current size: %d slots\n", table->size);

	printf("--- end stats ---\n");
}
