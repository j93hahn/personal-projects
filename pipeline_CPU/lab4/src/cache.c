/*
 * CMSC 22200, Fall 2016
 *
 * ARM pipeline timing simulator
 *
 */

/* Group Members- 
    1. Joshua Ahn (CNETID: jjahn)
    2. Zach Pizer (CNETID: zpizer)
*/

#include "cache.h"
#include "pipe.h"
#include <stdlib.h>
#include <stdio.h>

/* used to index into the instruction cache */
unsigned int icache_index(uint64_t PC) {
    unsigned int index = (PC >> 5) & 0x3F; // 6 bits
    return index;
}

/* used to index into the data cache */
uint8_t dcache_index(uint64_t address) {
    uint8_t index = (address >> 5) & 0xFF; // 8 bits
    return index;
}

/* determine which block to pick in the icache set */
unsigned int icache_tag(uint64_t PC) {
    unsigned int tag = (PC >> 10) & 0x1FFFFFFFFFFFFF; // 53 bits
    return tag; 
}

/* determine which block to pick in the dcache set */
unsigned int dcache_tag(uint64_t address) {
    unsigned int tag = (address >> 12) & 0x7FFFFFFFFFFFF; // 51 bits
    return tag;
}

/* determine the block offset for the block */
unsigned int block_offset(uint64_t address) {
    unsigned int offset = address & 0x1F; // first 5 bits; 
    return offset;
}

/* determines if the cache is full */
void cache_full(cache_t *cache) {
    int full = 1; // assume cache full
    for (int i = 0; i < cache->num_sets; i++) {
        for (int j = 0; j < cache->num_ways; j++) {
            if (cache->sets[i].blocks[j].valid == 0) {
                full = 0;
                break;
            }
        }
    }
    if (full) {
        printf("The cache is full.\n"); 
    }
}

/* print the contents of the cache */
void cache_print(cache_t *cache) {
    if (cache->num_sets == 64) { // instruction cache
        printf("-------------Instruction Cache-------------\n");
        for (int i = 0; i < cache->num_sets; i++) {
            printf("-----Set %d-----\n", i + 1);
            for (int j = 0; j < cache->num_ways; j++) {
                printf("Block %d - ", j + 1);
                printf("Valid: %d, Tag: %d, Values: [", cache->sets[i].blocks[j].valid,
                       cache->sets[i].blocks[j].tag);
                for (int g = 0; g < 7; g++) {
                    printf("%d, ", cache->sets[i].blocks[j].values[g]);
                }
                printf("%d]\n", cache->sets[i].blocks[j].values[7]);
            }
        }
    } else { // data cache
        printf("-----------------Data Cache-----------------\n");
        for (int i = 0; i < cache->num_sets; i++) {
            printf("-----Set %d-----\n", i + 1);
            for (int j = 0; j < cache->num_ways; j++) {
                printf("Block %d - ", j + 1);
                printf("Valid: %d, Tag: %d, Values: [", cache->sets[i].blocks[j].valid,
                       cache->sets[i].blocks[j].tag);
                for (int g = 0; g < 7; g++) {
                    printf("%d, ", cache->sets[i].blocks[j].values[g]);
                }
                printf("%d]\n", cache->sets[i].blocks[j].values[7]);
            }
        }
    }
}

/* create a new cache */
cache_t *cache_new(int sets, int ways) { 
    cache_t *cache = (cache_t *)malloc(sizeof(cache_t));
    cache->num_sets = sets;
    cache->num_ways = ways;

    cache->sets = (set_t *)malloc(sets * sizeof(set_t));
    for (int i = 0; i < sets; i++) {
        cache->sets[i].blocks = (block_t *)malloc(ways * sizeof(block_t));
    }
    return cache; 
}

/* initialize caches */
void cache_init(cache_t **cache1, cache_t **cache2) {
    *cache1 = cache_new(64, 4); // instruction cache
    *cache2 = cache_new(256, 8); // data cache
}

/* do not call this function in the pipeline. free the allocated space but don't call in the pipeline */
void cache_destroy(cache_t *c) {
    for (int i = 0; i < c->num_sets; i++) {
        free(c->sets[i].blocks);
    }
    free(c->sets);
    free(c);
}

/* update the cache with new information */
void cache_update(cache_t *c, uint64_t addr, unsigned int index, unsigned int i, int *hit) {
    int blocks = c->num_ways;
    int comparator;
    if (*hit) { // cache hit
        /* only have to reorganize LRU values */
        comparator = c->sets[index].blocks[i].LRU_value; // old LRU value
        c->sets[index].blocks[i].LRU_value = blocks - 1; // new LRU value
        for (int j = 0; j < blocks; j++) {
            if (j == i) {
                continue;
            } else if (j != i && c->sets[index].blocks[j].valid == 1) { // only update blocks that are valid 
                if (c->sets[index].blocks[j].LRU_value == 0) { // already least recently used --> no need to change
                    continue;
                } else if (c->sets[index].blocks[j].LRU_value > comparator) { // comparator serves to determine how far down the cache LRU hierarchy you should reorganize
                    c->sets[index].blocks[j].LRU_value -= 1; // reduce by 1
                } else {
                    continue;
                }
            } else if (j != i && c->sets[index].blocks[j].valid == 0) { // if cache is partially full 
                continue; 
            }
        }
        printf("cache hit\n");
    } else { // cache miss
        /* first determine if the cache is full or not */
        int full = 1; // assume cache is full
        int empty_block = 0; // determines first empty block in the set, if it exists
        for (int b = 0; b < blocks; b++) {
            if (c->sets[index].blocks[b].valid == 0) {
                full = 0; 
                empty_block = b; 
                break;
            }
        }

        if (!full) { // cache is !full -> update empty cache block
            printf("NOt full\n");
            c->sets[index].blocks[empty_block].valid = 1; // valid bit
            c->sets[index].blocks[empty_block].block_offset = block_offset(addr); // block offset
            
            // LRU values reorganization
            comparator = c->sets[index].blocks[empty_block].LRU_value; // old LRU value
            c->sets[index].blocks[empty_block].LRU_value = blocks - 1; // new LRU value
            for (int j = 0; j < blocks; j++) {
                if (j == empty_block) {
                    continue;
                } else if (j != empty_block && c->sets[index].blocks[j].valid == 1) { // only update blocks that are valid 
                    if (c->sets[index].blocks[j].LRU_value == 0) { // already least recently used --> no need to change
                        continue;
                    } else if (c->sets[index].blocks[j].LRU_value > comparator) { // comparator serves to determine how far down the cache LRU hierarchy you should reorganize
                        c->sets[index].blocks[j].LRU_value -= 1; // reduce by 1
                    } else {
                        continue;
                    }
                } else if (j != empty_block && c->sets[index].blocks[j].valid == 0) { // if cache is partially full 
                    continue; 
                }
            }

            // input tag
            if (c->num_sets == 64) {
                c->sets[index].blocks[empty_block].tag = icache_tag(addr);
            } else {
                c->sets[index].blocks[empty_block].tag = dcache_tag(addr);
            }

            // fetch data from memory 
            unsigned int offset = addr % 32; 
            uint64_t start = addr - offset;
            c->sets[index].blocks[empty_block].values[0] = mem_read_32(start);
            c->sets[index].blocks[empty_block].values[1] = mem_read_32(start + 0x4);
            c->sets[index].blocks[empty_block].values[2] = mem_read_32(start + 0x8);
            c->sets[index].blocks[empty_block].values[3] = mem_read_32(start + 0xc);
            c->sets[index].blocks[empty_block].values[4] = mem_read_32(start + 0x10);
            c->sets[index].blocks[empty_block].values[5] = mem_read_32(start + 0x14);
            c->sets[index].blocks[empty_block].values[6] = mem_read_32(start + 0x18);
            c->sets[index].blocks[empty_block].values[7] = mem_read_32(start + 0x1c);

        } else { // cache is full -> LRU block replacement policy
            printf("cache full\n"); 
            /* determine the LRU block */
            int LRU_block = blocks - 1; 
            for (int j = 0; j < blocks; j++) {
                if (c->sets[index].blocks[j].LRU_value < c->sets[index].blocks[LRU_block].LRU_value) {
                    LRU_block = j; 
                }
            }

            /* evict LRU block by overwriting it with the new block from memory */
            c->sets[index].blocks[LRU_block].valid = 1; // valid bit
            c->sets[index].blocks[LRU_block].block_offset = block_offset(addr); // block offset

            // LRU values reorganization
            comparator = c->sets[index].blocks[LRU_block].LRU_value; 
            c->sets[index].blocks[LRU_block].LRU_value = blocks - 1;
            for (int j = 0; j < blocks; j++) {
                if (j == LRU_block) {
                    continue;
                } else if (j != LRU_block && c->sets[index].blocks[j].valid == 1) { // only update blocks that are valid 
                    if (c->sets[index].blocks[j].LRU_value == 0) { // already least recently used --> no need to change
                        continue;
                    } else if (c->sets[index].blocks[j].LRU_value > comparator) { // comparator serves to determine how far down the cache LRU hierarchy you should reorganize
                        c->sets[index].blocks[j].LRU_value -= 1; // reduce by 1
                    } else {
                        continue;
                    }
                } else if (j != LRU_block && c->sets[index].blocks[j].valid == 0) { // if cache is partially full 
                    continue; 
                }
            }

            // input tag
            if (c->num_sets == 64) {
                c->sets[index].blocks[LRU_block].tag = icache_tag(addr);
            } else {
                c->sets[index].blocks[LRU_block].tag = dcache_tag(addr);
            }

            // fetch data from memory
            unsigned int offset = addr % 32; 
            uint64_t start = addr - offset;
            c->sets[index].blocks[LRU_block].values[0] = mem_read_32(start);
            c->sets[index].blocks[LRU_block].values[1] = mem_read_32(start + 0x4);
            c->sets[index].blocks[LRU_block].values[2] = mem_read_32(start + 0x8);
            c->sets[index].blocks[LRU_block].values[3] = mem_read_32(start + 0xc);
            c->sets[index].blocks[LRU_block].values[4] = mem_read_32(start + 0x10);
            c->sets[index].blocks[LRU_block].values[5] = mem_read_32(start + 0x14);
            c->sets[index].blocks[LRU_block].values[6] = mem_read_32(start + 0x18);
            c->sets[index].blocks[LRU_block].values[7] = mem_read_32(start + 0x1c);
        }
    }
}