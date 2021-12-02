/*
 * main.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 15, 2021
 *
 */

#include <stdio.h>
#include "cache_impl.h"

int num_cache_hits = 0;
int num_cache_misses = 0;

int num_bytes = 0;
int num_access_cycles = 0;

int global_timestamp = 0;

int retrieve_data(void* addr, char data_type) {
    int value_returned = -1; /* accessed data */

    /* Invoke check_cache_data_hit() */
    value_returned = check_cache_data_hit((void*)addr, data_type);

    /* In case of the cache miss event, access the main memory by invoking access_memory() */
    if (value_returned == -1) { //메인 메모리에서 캐시 메모리로 가져오기
        num_cache_misses++;
        value_returned = access_memory((void*)addr, data_type);
    }
    else
        num_cache_hits++;

    return value_returned; //데이터 리턴
}

int main(void) {
    FILE* ifp = NULL, * ofp = NULL;
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */

    init_memory_content();
    init_cache_content();

    ifp = fopen("access_input.txt", "r");
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }
    ofp = fopen("access_output.txt", "w");
    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }

    /* Fill out here by invoking retrieve_data() */
    fprintf(ofp, "[Accessed Data]\n");
    while (fscanf(ifp, "%d %c", &access_addr, &access_type) != EOF) {
        int accessed_data = retrieve_data((void*)access_addr, access_type);
        fprintf(ofp, "%d \t %c \t %#x\n", access_addr, access_type, accessed_data);
    }
    fprintf(ofp,"-------------------------------------------\n");
    if (DEFAULT_CACHE_ASSOC == 1)
        fprintf(ofp, "[Direct mapped cache performance]\n");
    else if (DEFAULT_CACHE_ASSOC == 2)
        fprintf(ofp, "[2-way set associative cache performance]\n");
    else
        fprintf(ofp, "[Accessed Data]\n");
    int accesses = num_cache_hits + num_cache_misses;
    double hit_ratio = (double)num_cache_hits / ((double)accesses);
    double bandwidth = (double)num_bytes / (double)num_access_cycles;
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", hit_ratio, num_cache_hits, accesses);
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)\n", bandwidth, num_bytes, num_access_cycles);

    fclose(ifp);
    fclose(ofp);

    print_cache_entries();
    return 0;
}
