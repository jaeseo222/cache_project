/*
 * cache.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 15, 2021
 *
 */


#include <stdio.h>
#include <string.h>
#include "cache_impl.h"

extern int num_cache_hits;
extern int num_cache_misses;

extern int num_bytes;
extern int num_access_cycles;

extern int global_timestamp;

cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];
int memory_array[DEFAULT_MEMORY_SIZE_WORD];


/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_memory_content() {
	unsigned char sample_upward[16] = { 0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef };
	unsigned char sample_downward[16] = { 0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010 };
	int index, i = 0, j = 1, gap = 1;

	for (index = 0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
		memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
		if (++i >= 16)
			i = 0;
		if (++j >= 16)
			j = 0;

		if (i == 0 && j == i + gap)
			j = i + (++gap);

		printf("mem[%d] = %#x\n", index, memory_array[index]);
	}
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
	int i, j;

	for (i = 0; i < CACHE_SET_SIZE; i++) {
		for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
			cache_entry_t* pEntry = &cache_array[i][j];
			pEntry->valid = 0;
			pEntry->tag = -1;
			pEntry->timestamp = 0;
		}
	}
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
	int i, j, k;

	for (i = 0; i < CACHE_SET_SIZE; i++) {
		printf("[Set %d] ", i);
		for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
			cache_entry_t* pEntry = &cache_array[i][j];
			printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
			for (k = 0; k < DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
				printf("%#x(%d) ", pEntry->data[k], k);
			}
			printf("\t");
		}
		printf("\n");
	}
}

int get_accessed_data(cache_entry_t* p, void* addr, char type) {
	int byte_offset = ((int)addr) % DEFAULT_CACHE_BLOCK_SIZE_BYTE; // get byte offset.
	int accessed_data = -1; // initialize accessed data.
	if (type == 'b') { // if type byte.
		accessed_data = p->data[byte_offset]; // copy 1 byte.
		num_bytes += 1; // add 1 byte.
	}
	else if (type == 'h') { // if type half word.
		accessed_data = (p->data[byte_offset + 1] << 8) | ((p->data[byte_offset]) & 0x0ff); // copy 2 byte. (& operate with 0x0ff to force expanding 0)
		num_bytes += 2; // add 2 byte.
	}
	else if (type == 'w') { // if type word.
		// copy 4 byte. (&operate with 0x0ff to force expanding 0)
		accessed_data = ((p->data[byte_offset]) & 0x0ff) | ((p->data[byte_offset + 1] << 8) & 0x0ffff) | ((p->data[byte_offset + 2] << 16) & 0x0ffffff) | (p->data[byte_offset + 3] << 24); 
		num_bytes += 4; // add 4 byte.
	}

	// return accessed data.
	return accessed_data;
}

int check_cache_data_hit(void* addr, char type) {
	// add cache access cycle.
	num_access_cycles += CACHE_ACCESS_CYCLE;

	// check all entries in a set.
	int tag = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE; // get tag.

	for (int i = 0; i < CACHE_SET_SIZE; i++) { // access each entry.
		for (int j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
			cache_entry_t* p = &cache_array[i][j]; // access cache memory.

			// if hit that valid bit already exists and tag is same.
			if (p->valid == 1 && p->tag == (tag)) {
				p->timestamp = global_timestamp++; // cache time update.
				int accessed_data = get_accessed_data(p, (void*)addr, type); // get accessed data.
				return accessed_data; // Return the accessed data.
			}
		}
	}

	// if miss, return -1. 
	return -1;
}
int find_entry_index_in_set(int cache_index) {
	//initialize entry index.
	int entry_index = 0;

	// If the set has only 1 entry, return index 0.
	if (DEFAULT_CACHE_ASSOC == 1)
		return 0;

	// Check if there exists any empty cache space by checking 'valid'.
	for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) { // access each entry.
		cache_entry_t* p = &cache_array[cache_index][i]; // access cache memory.
		if (p->valid == 0) // if valid not exists, return current index.
			return i;
	}

	// Otherwise, search over all entries to find the least recently used entry by checking 'timestamp'.
	int min_timestamp = cache_array[cache_index][0].timestamp; // first entry time.
	for (int i = 1; i < DEFAULT_CACHE_ASSOC; i++) { // access each entry.
		cache_entry_t* p = &cache_array[cache_index][i]; // access cache memory.

		// find LRU.
		if (p->timestamp < min_timestamp) { // if current timestamp more smaller than min_timestamp.
			min_timestamp = p->timestamp; // update smallest timestamp.
			entry_index = i; // update entry index.
		}
	}

	// return entry index.
	return entry_index;
}

int access_memory(void* addr, char type) {
	// get the entry index by invoking find_entry_index_in_set() for copying to the cache.
	int cache_index = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE; // get cache index.(set index)
	int entry_index = find_entry_index_in_set(cache_index); // get entry index.

	// add memory access cycle.
	num_access_cycles += MEMORY_ACCESS_CYCLE;

	// Fetch the data from the main memory and copy them to the cache.
	cache_entry_t* p = &cache_array[cache_index][entry_index]; // access cache memory.
	p->timestamp = global_timestamp++; // cache time update.
	p->valid = 1; // cache valid update.
	p->tag = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE; // get tag.

	int word_index = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) * DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE; // get word index.
	for (int i = 0; i < DEFAULT_CACHE_BLOCK_SIZE_BYTE; i++) { // copy main memory data to cache data.
		int index = i / WORD_SIZE_BYTE; // adding index to word index(0 or 1).
		int shift_bits = (i % WORD_SIZE_BYTE) * DEFAULT_CACHE_BLOCK_SIZE_BYTE; // shift oprating bits.
		p->data[i] = memory_array[word_index + index] >> shift_bits; // copy each 1 byte.
	}

	// Return the accessed data with a suitable type.
	int accessed_data = get_accessed_data(p, (void*)addr, type);
	return accessed_data;
}
