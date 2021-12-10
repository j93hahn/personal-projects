/*
 * CMSC 22200, Fall 2016
 *
 * ARM pipeline timing simulator
 *
 */

#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdint.h>

/* block structure */
typedef struct block_t {
    int valid; 
    unsigned int tag; 
    unsigned int LRU_value; 
    unsigned int block_offset;
    uint32_t values[8]; 
} block_t; 

/* set structure */
typedef struct set_t {
    block_t *blocks;
    unsigned int LRU_total;
} set_t;

/* cache structure */
typedef struct cache_t {
    int num_sets;
    int num_ways; 
    set_t *sets;
} cache_t;

/* declare cache functions */
unsigned int icache_index(uint64_t PC);
uint8_t dcache_index(uint64_t address);
unsigned int icache_tag(uint64_t PC);
unsigned int dcache_tag(uint64_t address);
unsigned int block_offset(uint64_t address);

void cache_full(cache_t *cache);
void cache_print(cache_t *cache);

cache_t *cache_new(int sets, int ways);
void cache_init(cache_t **cache1, cache_t **cache2);
void cache_destroy(cache_t *c);
void cache_update(cache_t *c, uint64_t addr, unsigned int index, unsigned int i, int *hit);

#endif
