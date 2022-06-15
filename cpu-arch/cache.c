/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2021
 * Yanjing Li
 */

/*
 * Group members:
 * 1. Joshua Ahn (CNET: jjahn)
 * 2. Zach Pizer (CNET: zpizer)
 */

#include "cache.h"
#include "pipe.h"
#include <stdlib.h>
#include <stdio.h>

/* ---------------- */
/*  CACHE INDEXING  */
/* ---------------- */

unsigned int icache_index(uint64_t PC) {
    unsigned int index = (PC >> 5) & 0x3F;
    return index;
}

uint8_t dcache_index(uint64_t address) {
    uint8_t index = (address >> 5) & 0xFF;
    return index;
}

unsigned int icache_tag(uint64_t PC) {
    unsigned int tag = (PC >> 10) & 0x1FFFFFFFFFFFFF;
    return tag;
}

unsigned int dcache_tag(uint64_t address) {
    unsigned int tag = (address >> 12) & 0x7FFFFFFFFFFFF;
    return tag;
}

unsigned int block_offset(uint64_t address) {
    unsigned int offset = address & 0x1F;
    return offset;
}

/* ----------------- */
/*  CACHE DEBUGGING  */
/* ----------------- */

void cache_full(cache_t *cache) {
    int full = 1;
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

void cache_print(cache_t *cache) {
    if (cache->num_sets == 64) {
        printf("-------------Instruction Cache-------------\n");
        for (int i = 0; i < cache->num_sets; i++) {
            printf("-----Set %d-----\n", i + 1);
            for (int j = 0; j < cache->num_ways; j++) {
                printf("Block %d - ", j + 1);
                printf("Valid: %d, Tag: %d, Values: [",
                       cache->sets[i].blocks[j].valid,
                       cache->sets[i].blocks[j].tag);
                for (int g = 0; g < 7; g++) {
                    printf("%d, ", cache->sets[i].blocks[j].values[g]);
                }
                printf("%d]\n", cache->sets[i].blocks[j].values[7]);
            }
        }
    } else {
        printf("-----------------Data Cache-----------------\n");
        for (int i = 0; i < cache->num_sets; i++) {
            printf("-----Set %d-----\n", i + 1);
            for (int j = 0; j < cache->num_ways; j++) {
                printf("Block %d - ", j + 1);
                printf("Valid: %d, Tag: %d, Values: [",
                       cache->sets[i].blocks[j].valid,
                       cache->sets[i].blocks[j].tag);
                for (int g = 0; g < 7; g++) {
                    printf("%d, ", cache->sets[i].blocks[j].values[g]);
                }
                printf("%d]\n", cache->sets[i].blocks[j].values[7]);
            }
        }
    }
}

/* ------------------ */
/*  CACHE OPERATIONS  */
/* ------------------ */

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

void cache_init(cache_t **cache1, cache_t **cache2) {
    *cache1 = cache_new(64, 4);
    *cache2 = cache_new(256, 8);
}

void cache_destroy(cache_t *c) {
    for (int i = 0; i < c->num_sets; i++) {
        free(c->sets[i].blocks);
    }
    free(c->sets);
    free(c);
}

void cache_update(cache_t *c, uint64_t addr, unsigned int index,
                  unsigned int i, int *hit) {
    int blocks = c->num_ways;
    int comparator;
    if (*hit) {
        comparator = c->sets[index].blocks[i].LRU_value;
        c->sets[index].blocks[i].LRU_value = blocks - 1;
        for (int j = 0; j < blocks; j++) {
            if (j == i) {
                continue;
            } else if (j != i && c->sets[index].blocks[j].valid == 1) {
                if (c->sets[index].blocks[j].LRU_value == 0) {
                    continue;
                } else if (c->sets[index].blocks[j].LRU_value > comparator) {
                    c->sets[index].blocks[j].LRU_value -= 1;
                } else {
                    continue;
                }
            } else if (j != i && c->sets[index].blocks[j].valid == 0) {
                continue;
            }
        }
    } else {
        int full = 1;
        int empty_block = 0;
        for (int b = 0; b < blocks; b++) {
            if (c->sets[index].blocks[b].valid == 0) {
                full = 0;
                empty_block = b;
                break;
            }
        }

        if (!full) {
            c->sets[index].blocks[empty_block].valid = 1;
            c->sets[index].blocks[empty_block].block_offset = block_offset(addr);

            comparator = c->sets[index].blocks[empty_block].LRU_value;
            c->sets[index].blocks[empty_block].LRU_value = blocks - 1;
            for (int j = 0; j < blocks; j++) {
                if (j == empty_block) {
                    continue;
                } else if (c->sets[index].blocks[j].valid == 1 && j != empty_block) {
                    if (c->sets[index].blocks[j].LRU_value == 0) {
                        continue;
                    } else if (c->sets[index].blocks[j].LRU_value > comparator) {
                        c->sets[index].blocks[j].LRU_value -= 1;
                    } else {
                        continue;
                    }
                } else if (c->sets[index].blocks[j].valid == 0 && j != empty_block) {
                    continue;
                }
            }

            if (c->num_sets == 64) {
                c->sets[index].blocks[empty_block].tag = icache_tag(addr);
            } else {
                c->sets[index].blocks[empty_block].tag = dcache_tag(addr);
            }

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
        } else {
            int LRU_block = blocks - 1;
            for (int j = 0; j < blocks; j++) {
                if (c->sets[index].blocks[j].LRU_value < c->sets[index].blocks[LRU_block].LRU_value) {
                    LRU_block = j;
                }
            }

            c->sets[index].blocks[LRU_block].valid = 1;
            c->sets[index].blocks[LRU_block].block_offset = block_offset(addr);

            comparator = c->sets[index].blocks[LRU_block].LRU_value;
            c->sets[index].blocks[LRU_block].LRU_value = blocks - 1;
            for (int j = 0; j < blocks; j++) {
                if (j == LRU_block) {
                    continue;
                } else if (j != LRU_block && c->sets[index].blocks[j].valid == 1) {
                    if (c->sets[index].blocks[j].LRU_value == 0) {
                        continue;
                    } else if (c->sets[index].blocks[j].LRU_value > comparator) {
                        c->sets[index].blocks[j].LRU_value -= 1;
                    } else {
                        continue;
                    }
                } else if (j != LRU_block && c->sets[index].blocks[j].valid == 0) {
                    continue;
                }
            }

            if (c->num_sets == 64) {
                c->sets[index].blocks[LRU_block].tag = icache_tag(addr);
            } else {
                c->sets[index].blocks[LRU_block].tag = dcache_tag(addr);
            }

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
