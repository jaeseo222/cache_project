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

int check_cache_data_hit(void* addr, char type) {

	/* Fill out here */

	//add cache access cycle.
	num_access_cycles += CACHE_ACCESS_CYCLE;

	//check all entries in a set.
	int block_addr = (int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE;
	int tag = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE;
	int byte_offset = ((int)addr) % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
	int word_index = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) * DEFAULT_CACHE_BLOCK_SIZE_BYTE / CACHE_SET_SIZE;
	int cache_index = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE;

	//cache의 set만큼 돌려보고
	for (int i = 0; i < CACHE_SET_SIZE; i++) {
		//entry 하나씩 접근
		for (int j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
			//캐시 메모리 접근하기
			cache_entry_t* p = &cache_array[i][j];

			//valid bit이 1이고(값 이미 있고), tag값이 일치하면 데이터 리턴(혹시 time은 건들필요 없나?)
			if (p->valid == 1 && p->tag == (tag)) {
				p->timestamp = global_timestamp++;
				return p->data;
			}
		}
	}
	/* Return the data */
	return -1;
}
int find_entry_index_in_set(int cache_index) {
	int entry_index = 0;

	/* Check if there exists any empty cache space by checking 'valid' */
	for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
		//캐시 메모리 접근하기
		cache_entry_t* p = &cache_array[cache_index][i];
		if (p->valid == 0) return i;
	}

	//근데 entry가 1개밖에 없으면(direct mapped), 그냥 index 0리턴 하라고 써져있네
	if (DEFAULT_CACHE_ASSOC == 1)
		return 0;

	/* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
	//direct 아닌경우,
	//모든 entry 구경하러 가자.
	//least recently used entry by checking timestamp를 찾자~!

	//맨 처음 entry의 time
	int min_temp = cache_array[cache_index][0].timestamp;
	for (int i = 1; i < DEFAULT_CACHE_ASSOC; i++) {
		//캐시 메모리 접근하기
		cache_entry_t* p = &cache_array[cache_index][i];

		//모든 entry 중에서 가장 작은 time 가진 친구의 entry index로 업뎃
		if (p->timestamp < min_temp) {
			min_temp = p->timestamp;
			//update된 entry 주기
			entry_index = i;
		}
	}

	return entry_index;
}

int access_memory(void* addr, char type) {

	/* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */

	/* You need to invoke find_entry_index_in_set() for copying to the cache */

	//cache_index(set_index) 보내줘서 entry_index 얻어오기
	int cache_index = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE;
	int entry_index = find_entry_index_in_set(cache_index);

	num_access_cycles += MEMORY_ACCESS_CYCLE;

	/* Fetch the data from the main memory and copy them to the cache */
	cache_entry_t* p = &cache_array[cache_index][entry_index];
	p->timestamp = global_timestamp++;
	p->valid = 1;
	p->tag = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE;
	int word_index = ((int)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) * DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE; //메인 메모리 카피하는 시작 주소
	for (int i = 0; i < DEFAULT_CACHE_BLOCK_SIZE_BYTE; i++) { //cache block size byte만큼 돌면서 main memory data를 cache에 옮기기
		int index = i / WORD_SIZE_BYTE; //memory_array 시작 인덱스 (0 or 1)
		int shift_bits = (i % WORD_SIZE_BYTE) * DEFAULT_CACHE_BLOCK_SIZE_BYTE; //shift연산 비트수
		p->data[i] = memory_array[word_index + index] >> shift_bits;
	}

	/* Return the accessed data with a suitable type */
	int byte_offset = ((int)addr) % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
	int accessed_data = -1;
	if (type == 'b') {
		accessed_data = p->data[byte_offset];
	}
	else if (type == 'h') {
		accessed_data = (p->data[byte_offset]) | (p->data[byte_offset+1] << 8);
	}
	else if (type == 'w') {
		accessed_data = (p->data[byte_offset]) | (p->data[byte_offset+1] << 8) | (p->data[byte_offset+2] << 16) | (p->data[byte_offset+3] << 24);
	}
	return accessed_data;
}
