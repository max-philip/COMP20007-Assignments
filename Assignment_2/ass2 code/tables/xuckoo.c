/* * * * * * * * *
* Dynamic hash table using a combination of extendible hashing and cuckoo
* hashing with a single keys per bucket, resolving collisions by switching keys
* between two tables with two separate hash functions and growing the tables
* incrementally in response to cycles
*
* created for COMP20007 Design of Algorithms - Assignment 2, 2017
* by Max Philip
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "xuckoo.h"

// macro to calculate the rightmost n bits of a number x
#define rightmostnbits(n, x) (x) & ((1 << (n)) - 1)

// a bucket stores a single key (full=true) or is empty (full=false)
// it also knows how many bits are shared between possible keys, and the first
// table address that references it
typedef struct bucket {
	int id;		// a unique id for this bucket, equal to the first address
				// in the table which points to it
	int depth;	// how many hash value bits are being used by this bucket
	bool full;	// does this bucket contain a key
	int64 key;	// the key stored in this bucket
} Bucket;

// an inner table is an extendible hash table with an array of slots pointing
// to buckets holding up to 1 key, along with some information about the number
// of hash value bits to use for addressing
typedef struct inner_table {
	Bucket **buckets;	// array of pointers to buckets
	int size;			// how many entries in the table of pointers (2^depth)
	int depth;			// how many bits of the hash value to use (log2(size))
	int nkeys;			// how many keys are being stored in the table
} InnerTable;

// a xuckoo hash table is just two inner tables for storing inserted keys
struct xuckoo_table {
	InnerTable *table1;
	InnerTable *table2;
};


bool xuckoo_hash_table_lookup(XuckooHashTable *table, int64 key);

// create a new bucket to contain the new key
static Bucket *new_bucket(int first_address, int depth) {
	Bucket *bucket = malloc(sizeof *bucket);
	assert(bucket);

	bucket->id = first_address;
	bucket->depth = depth;
	bucket->full = false;

	return bucket;
}


// double the table of bucket pointers, duplicating the bucket pointers in the
// first half into the new second half of the table
static void double_table(InnerTable *table) {
	int size = table->size * 2;
	assert(size < MAX_TABLE_SIZE && "error: table has grown too large!");

	// get a new array of twice as many bucket pointers, and copy pointers down
	table->buckets = realloc(table->buckets, (sizeof *table->buckets) * size);
	assert(table->buckets);
	int i;
	for (i = 0; i < table->size; i++) {
		table->buckets[table->size + i] = table->buckets[i];
	}

	// finally, increase the table size and the depth we are using to hash keys
	table->size = size;
	table->depth++;
}

// reinsert a key into the hash table after splitting a bucket --- we can assume
// that there will definitely be space for this key because it was already
// inside the hash table previously
// use 'xtndbl1_hash_table_insert()' instead for inserting new keys
static void reinsert_key(XuckooHashTable *table, int64 key, int curr_inner) {

	int address;
	if (curr_inner == 1){
		address = rightmostnbits(table->table1->depth, h1(key));
		table->table1->buckets[address]->key = key;
		table->table1->buckets[address]->full = true;
	} else {
		address = rightmostnbits(table->table2->depth, h2(key));
		table->table2->buckets[address]->key = key;
		table->table2->buckets[address]->full = true;
	}
}

// split the bucket in 'table' at address 'address', growing table if necessary
static void split_bucket(XuckooHashTable *main_table, InnerTable *table, int address, int curr_inner) {

	// FIRST,
	// do we need to grow the table?
	if (table->buckets[address]->depth == table->depth) {
		// yep, this bucket is down to its last pointer
		double_table(table);
	}
	// either way, now it's time to split this bucket

	// SECOND,
	// create a new bucket and update both buckets' depth
	Bucket *bucket = table->buckets[address];
	int depth = bucket->depth;
	int first_address = bucket->id;

	int new_depth = depth + 1;
	bucket->depth = new_depth;

	// new bucket's first address will be a 1 bit plus the old first address
	int new_first_address = 1 << depth | first_address;
	Bucket *newbucket = new_bucket(new_first_address, new_depth);
	//table->stats.nbuckets++;

	// THIRD,
	// redirect every second address pointing to this bucket to the new bucket
	// construct addresses by joining a bit 'prefix' and a bit 'suffix'
	// (defined below)

	// suffix: a 1 bit followed by the previous bucket bit address
	int bit_address = rightmostnbits(depth, first_address);
	int suffix = (1 << depth) | bit_address;

	// prefix: all bitstrings of length equal to the difference between the new
	// bucket depth and the table depth
	// use a for loop to enumerate all possible prefixes less than maxprefix:
	int maxprefix = 1 << (table->depth - new_depth);

	int prefix;
	for (prefix = 0; prefix < maxprefix; prefix++) {

		// construct address by joining this prefix and the suffix
		int a = (prefix << new_depth) | suffix;

		// redirect this table entry to point at the new bucket
		table->buckets[a] = newbucket;
	}

	// FINALLY,
	// filter the key from the old bucket into its rightful place in the new
	// table (which may be the old bucket, or may be the new bucket)

	// remove and reinsert the key
	int64 key = bucket->key;
	bucket->full = false;
	//table->nkeys--;
	reinsert_key(main_table, key, curr_inner);
}


// initialise an extendible cuckoo hash table
XuckooHashTable *new_xuckoo_hash_table() {

	XuckooHashTable *table = malloc(sizeof *table);

	// initialise inner tables
	table->table1 = malloc(sizeof *table->table1);
	table->table2 = malloc(sizeof *table->table2);

	table->table1->buckets = malloc(sizeof *table->table1->buckets);
	table->table2->buckets = malloc(sizeof *table->table2->buckets);

	table->table1->buckets[0] = new_bucket(0, 0);
	table->table2->buckets[0] = new_bucket(0, 0);

	table->table1->size = 1;
	table->table2->size = 1;

	table->table1->depth = 0;
	table->table2->depth = 0;

	table->table1->nkeys = 0;
	table->table2->nkeys = 0;
	return table;
}


// free all memory associated with 'table'
void free_xuckoo_hash_table(XuckooHashTable *table) {
	int i;
	for (i = table->table1->size-1; i >= 0; i--){
		if (table->table1->buckets[i]->id == i){
			free(table->table1->buckets[i]);
		}
	}
	for (i = table->table2->size-1; i >= 0; i--){
		if (table->table2->buckets[i]->id == i){
			free(table->table2->buckets[i]);
		}
	}

	free(table->table1->buckets);
	free(table->table2->buckets);

	free(table->table1);
	free(table->table2);

	free(table);
}


// insert 'key' into 'table', if it's not in there already
// returns true if insertion succeeds, false if it was already in there
bool xuckoo_hash_table_insert(XuckooHashTable *table, int64 key) {
	//fprintf(stderr, "not yet implemented\n");

	assert(table);

	// make sure to start with inner table that currently has fewer keys
	int curr_inner;
	if (table->table1->nkeys < table->table2->nkeys){
		curr_inner = 1;
	} else {
		curr_inner = 2;
	}

	// defaults to the first table if equal number of keys
	if (table->table1->nkeys == table->table2->nkeys){
		curr_inner = 1;
	} else {
		curr_inner = 2;
	}

	int address, loop=0;

	int64 temp_key;
	int64 curr_key = key;

	// make sure key is not already in the table
	if (xuckoo_hash_table_lookup(table, key)){
		return false;
	}

	while(true){
		if (curr_inner == 1){
			// quit if there is a cycle
			if ((loop > table->table1->nkeys + table->table2->nkeys ) && (key == curr_key)){
				printf("\n1CYCLE\n");
				break;
			}

			// swap keys if there is one already in there desired location
			address = rightmostnbits(table->table1->depth, h1(key));
			if (table->table1->buckets[address]->full){
				temp_key = table->table1->buckets[address]->key;
				table->table1->buckets[address]->key = key;
				key = temp_key;
				loop++;
			}

			// insert if the bucket is empty
			if (!table->table1->buckets[address]->full){
				table->table1->buckets[address]->key = key;
				table->table1->buckets[address]->full = true;
				table->table1->nkeys++;
				return true;
			}

			// otherwise split
			split_bucket(table, table->table1, address, curr_inner);
			address = rightmostnbits(table->table1->depth, h1(key));
		}

		if (curr_inner == 2){
			// quit if there is a cycle
			if ((loop > table->table2->nkeys + table->table1->nkeys ) && (key == curr_key)){
				printf("\n2CYCLE\n");
				break;
			}

			// swap keys if there is one already in there desired location
			address = rightmostnbits(table->table2->depth, h2(key));
			if (table->table2->buckets[address]->full){
				temp_key = table->table2->buckets[address]->key;
				table->table2->buckets[address]->key = key;
				key = temp_key;
				loop++;
			}

			// insert if the bucket is empty
			if (!table->table2->buckets[address]->full){
				table->table2->buckets[address]->key = key;
				table->table2->buckets[address]->full = true;
				table->table2->nkeys++;
				return true;
			}

			// otherwise split
			split_bucket(table, table->table2, address, curr_inner);
			address = rightmostnbits(table->table2->depth, h2(key));
		}

		// swap the current table for the next iteration
		if (curr_inner == 1){
			curr_inner = 2;
		} else {
			curr_inner = 1;
		}
	}
	return true;
}


// lookup whether 'key' is inside 'table'
// returns true if found, false if not
// adapted from xtndbl1.c
bool xuckoo_hash_table_lookup(XuckooHashTable *table, int64 key) {
	assert(table);

	// calculate table addresses for this key
	int address1 = rightmostnbits(table->table1->depth, h1(key));
	int address2 = rightmostnbits(table->table2->depth, h2(key));

	// check both inner tables for the key
	if (table->table1->buckets[address1]->full){
		if (table->table1->buckets[address1]->key == key){
			return true;
		}
	}
	if (table->table2->buckets[address2]->full){
		if (table->table2->buckets[address2]->key == key){
			return true;
		}
	}

	// add time elapsed to total CPU time before returning result
	return false;
}

// print the contents of 'table' to stdout
void xuckoo_hash_table_print(XuckooHashTable *table) {
	assert(table != NULL);

	printf("--- table ---\n");

	// loop through the two tables, printing them
	InnerTable *innertables[2] = {table->table1, table->table2};
	int t;
	for (t = 0; t < 2; t++) {
		// print header
		printf("table %d\n", t+1);

		printf("  table:               buckets:\n");
		printf("  address | bucketid   bucketid [key]\n");

		// print table and buckets
		int i;
		for (i = 0; i < innertables[t]->size; i++) {
			// table entry
			printf("%*d | %-*d ", 9, i, 9, innertables[t]->buckets[i]->id);

			// if this is the first address at which a bucket occurs, print it
			if (innertables[t]->buckets[i]->id == i) {
				printf("%*d ", 9, innertables[t]->buckets[i]->id);
				if (innertables[t]->buckets[i]->full) {
					printf("[%llu]", innertables[t]->buckets[i]->key);
				} else {
					printf("[ ]");
				}
			}

			// end the line
			printf("\n");
		}
	}
	printf("--- end table ---\n");
}


// print some statistics about 'table' to stdout
void xuckoo_hash_table_stats(XuckooHashTable *table) {
	fprintf(stderr, "not yet implemented\n");
	return;
}
