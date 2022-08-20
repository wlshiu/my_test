/*
 * CMSC 22200, Fall 2016
 *
 * ARM pipeline timing simulator
 *
 */

#include <assert.h>
#include "cache.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG 1

#undef dbg
#if defined(DEBUG) && (DEBUG)
#define dbg(str, ...)               printf(str, ##__VA_ARGS__)
#else
#define dbg(str, ...)
#endif


cache_t *instr_cache;
cache_t *data_cache;

uint32_t instr_cache_read(Pipeline_Register *reg, uint64_t addr)
{
    return cache_read(instr_cache, addr, reg);
}

uint32_t data_cache_read(Pipeline_Register *reg, uint64_t addr)
{
    return cache_read(data_cache, addr, reg);
}

uint32_t data_cache_write(Pipeline_Register *reg, uint64_t addr, uint32_t data)
{
    cache_write(data_cache, addr, data, reg);
}

int data_cache_stall_count()
{
    return data_cache->stall_count;
}

int instr_cache_stall_count()
{
    return instr_cache->stall_count;
}

void cache_init(CPU *new_cpu)
{
    if (DEBUG)
    {
        printf("cache_init\n");
    }
    new_cpu->instr_cache = cache_new(64, 4);
    new_cpu->data_cache = cache_new(256, 8);
}

cache_t *cache_new(int sets, int ways)
{
    cache_t *cache = malloc(sizeof(cache_t));

    block **blocks = malloc(sizeof(block *) * sets);
    int i;
    for (i = 0; i < sets; i++)
    {
        blocks[i] = malloc(sizeof(block) * ways);
        memset(blocks[i], 0, sizeof(block) * ways);
    }

    cache->blocks = blocks;
    cache->sets = sets;
    cache->ways = ways;
    cache->stall_count = 0;
    cache->stall_addr = 0;

    return cache;
}


void cache_destroy()
{
    cache_flush(data_cache);
    cache_flush(instr_cache);
}

int any_cache_is_dirty()
{
    return cache_is_dirty(data_cache) || cache_is_dirty(instr_cache);
}

int cache_is_dirty(cache_t *c)
{
    uint64_t sets = c->sets;
    uint64_t ways = c->ways;
    uint64_t set;
    uint64_t way;
    for (set = 0; set < sets; set++)
    {
        for (way = 0; way < ways; way++)
        {
            if (c->blocks[set][way].dirty_bit)
            {
                return 1;
            }
        }
    }
    return 0;
}

// Increment the LRU counter for every valid entry in the given set.
void cache_increment_counter(cache_t *c, int set)
{
    int i;
    for (i = 0; i < c->ways; i++)
    {
        if (c->blocks[set][i].valid_bit == 1)   // valid entry
        {
            c->blocks[set][i].LRU++;
        }
    }
}

void cache_initialize_block(cache_t *c, int set, int way, uint64_t addr, uint64_t tag)
{
    c->blocks[set][way].tag = tag;
    c->blocks[set][way].valid_bit = 1;
    c->blocks[set][way].dirty_bit = 0;
    c->blocks[set][way].LRU = 0;
    // Start reading 32 bytes from address with lowest five bits
    // zeroed out
    uint64_t starting_addr = addr & (~0x1F);
    int i;
    for (i = 0; i < 8; i++)
    {
        c->blocks[set][way].data[i] = mem_read_32(starting_addr + (i * 4));
    }
}

void cache_writeback(cache_t *c, int set, int way)
{
    int i;
    if (c->blocks[set][way].dirty_bit)
    {
        uint64_t tag = c->blocks[set][way].tag;
        uint64_t mem_address = c->sets == 64
                               ? (tag << 11) + (set << 5) // is instruction cache
                               : (tag << 13) + (set << 5); // is data cache
        for (i = 0; i < 8; i++)
        {
            if (DEBUG)
            {
                printf(
                    "cache_writeback: 0x%llx = %x\n",
                    (long long unsigned)mem_address + (i * 4),
                    c->blocks[set][way].data[i]
                );
            }
            mem_write_32(mem_address + (i * 4), c->blocks[set][way].data[i]);
        }
        c->blocks[set][way].dirty_bit = 0; // no longer dirty after writeback
    }
}

void cache_flush(cache_t *c)
{
    uint64_t sets = c->sets;
    uint64_t ways = c->ways;
    uint64_t set;
    uint64_t way;
    for (set = 0; set < sets; set++)
    {
        for (way = 0; way < ways; way++)
        {
            cache_writeback(c, set, way);
        }
    }
}

// Finds a block to evict, and does a writeback if it's dirty.
// Returns the index (way) of the block that got evicted.
// If there's an invalid block, we return that by default, because that means
// eviction isn't necessary.
int cache_evict(cache_t *c, int set)
{
    int victim = -1;
    int victim_counter = -1;
    int i;
    for (i = 0; i < c->ways; i++)
    {
        if (c->blocks[set][i].valid_bit == 0)
        {
            // No eviction needed, because an invalid block exists.
            if (DEBUG)
            {
                printf("cache_evict: using open block #%d\n", i);
            }
            return i;
        }
        if (victim_counter == -1 || c->blocks[set][i].LRU > victim_counter)
        {
            victim = i;
            victim_counter = c->blocks[set][i].LRU;
        }
    }
    cache_writeback(c, set, victim);
    if (DEBUG)
    {
        printf("cache_evict: evicting block #%d\n", victim);
    }
    return victim;
}


uint32_t cache_read(cache_t *c, uint64_t addr, Pipeline_Register *reg)
{
    if (DEBUG)
        printf("cache_read: beginning\n");
    // find set index
    uint64_t sets = c->sets;
    uint64_t ways = c->ways;
    uint64_t set_index;
    uint64_t tag;
    uint64_t offset;
    if (sets == 64)   // instruction cache
    {
        set_index = get_fragment_of(addr, 5, 10);
        tag = get_fragment_of(addr, 11, 31);
        offset = get_fragment_of(addr, 0, 4);
        if (DEBUG)
        {
            printf("cache_read: instruction cache\n");
        }
    }
    else              // data cache
    {
        set_index = get_fragment_of(addr, 5, 12);
        tag = get_fragment_of(addr, 13, 31);
        offset = get_fragment_of(addr, 0, 4);
        if (DEBUG)
        {
            printf("cache_read: data cache\n");
        }
    }

    // Determine hit or miss
    int i;
    uint32_t data;
    for (i = 0; i < ways; i++)
    {
        if ((c->blocks[set_index][i].valid_bit == 1) && // valid entry
                (c->blocks[set_index][i].tag == tag))   // correct tag
        {
            // Hit, return data
            if (DEBUG)
            {
                printf("cache_read: CACHE HIT (0x%llx)\n", (long long unsigned)addr);
            }
            c->stall_count = 0;
            // Increment every block's LRU counter, then reset
            // this block's LRU counter
            cache_increment_counter(c, set_index);
            c->blocks[set_index][i].LRU = 0;
            return c->blocks[set_index][i].data[offset / 4]; // return 4 bytes or 32 bits at a time?
        }
    }

    if (DEBUG)
    {
        printf("cache_read: CACHE MISS (0x%llx)\n", (long long unsigned)addr);
    }

    // Handle miss

    uint64_t current_block_addr = addr & (~0x1F);

    // Either:
    // - We weren't waiting on a stall to return, so stall pipeline.
    // - We are looking for a different address now due to redirection, so
    //   reinitialize the stall.
    if (!c->stall_count || (current_block_addr != c->stall_addr))
    {
        if (DEBUG)
        {
            printf("cache_read: stalling for 50 cycles\n");
        }
        reg->stall = 1; // Tell the pipeline stage we should stall this cycle
        c->stall_count = 50;
        c->stall_addr = current_block_addr;
        return;
    }

    // Otherwise, we are currently stalling.

    c->stall_count--;
    if (DEBUG)
    {
        printf("cache_read: stalling for %d more cycles\n", c->stall_count);
    }

    if (c->stall_count == 1)   // Finishing stall in the next step
    {
        if (DEBUG)
            printf("cache_read: filling in data\n");
        // Find a block to write to. cache_evict handles eviction if needed.
        int victim = cache_evict(c, set_index);
        cache_increment_counter(c, set_index);
        cache_initialize_block(c, set_index, victim, addr, tag);
    }

    reg->stall = 1; // Tell the pipeline stage we should stall this cycle
    return;
}

void cache_write(cache_t *c, uint64_t addr, uint32_t input, Pipeline_Register *reg)
{
    if (c->sets == 64)   // check if cache is instruction cache
    {
        if (DEBUG)
            printf("cache_write: attempt to write into instruction cache, returns nothing\n");
        return;
    }
    uint64_t set_index;
    uint64_t tag;
    uint64_t offset;
    uint64_t ways = 256;

    set_index = get_fragment_of(addr, 5, 12);
    tag = get_fragment_of(addr, 13, 31);
    offset = get_fragment_of(addr, 0, 4);

    // Check for hit
    int i;
    uint32_t data;
    for (i = 0; i < ways; i++)
    {
        if ((c->blocks[set_index][i].valid_bit == 1) && // valid entry
                (c->blocks[set_index][i].tag == tag))   // correct tag
        {
            // Hit, return data
            if (DEBUG)
            {
                printf("cache_write: CACHE HIT (0x%llx)\n", (long long unsigned)addr);
            }
            c->stall_count = 0;
            // Increment every block's LRU counter, then reset
            // this block's LRU counter
            cache_increment_counter(c, set_index);
            c->blocks[set_index][i].LRU = 0;
            c->blocks[set_index][i].dirty_bit = 1;
            c->blocks[set_index][i].data[offset / 4] = input; // return 4 bytes or 32 bits at a time?
            return;
        }
    }

    if (DEBUG)
    {
        printf("cache_write: CACHE MISS (0x%llx)\n", (long long unsigned)addr);
    }

    // Handles misses

    uint64_t current_block_addr = addr & (~0x1F);

    // We weren't waiting on a stall to return, so stall pipeline.
    if (!c->stall_count)
    {
        if (DEBUG)
        {
            printf("cache_write: stalling for 50 cycles\n");
        }
        reg->stall = 1; // Tell the pipeline stage we should stall this cycle
        c->stall_count = 50;
        c->stall_addr = current_block_addr;
        return;
    }

    // Otherwise, we are currently stalling.

    c->stall_count--;
    if (DEBUG)
    {
        printf("cache_write: stalling for %d more cycles\n", c->stall_count);
    }

    if (c->stall_count == 1)   // Finishing stall in the next step
    {
        if (DEBUG)
            printf("cache_write: filling in data\n");
        // Find a block to write to. cache_evict handles eviction if needed.
        int victim = cache_evict(c, set_index);
        cache_increment_counter(c, set_index);
        cache_initialize_block(c, set_index, victim, addr, tag);
    }

    reg->stall = 1; // Tell the pipeline stage we should stall this cycle
    return;
}



// void cache_test_init() {
//  printf("====== cache_test_init ======\n");
//  cache_t* cache1 = cache_new(64, 4);
//  int i;
//  for (i = 0; i < 64; i++) {
//      int j;
//      for (j = 0; j < 4; j++) {
//          cache1->blocks[i][j].tag = i * j;
//      }
//  }

//  for (i = 0; i < 64; i++) {
//      int j;
//      for (j = 0; j < 4; j++) {
//          assert(i * j == cache1->blocks[i][j].tag);
//      }
//  }

//  cache_destroy(cache1);
// }

// Simulates a pipeline stage with looping
// uint64_t get_data(cache_t *c, uint64_t addr, Pipeline_Register *reg) {
//  uint64_t data;
//  while (true) {
//      data = cache_read(c, addr, reg);
//      if (!reg->stall) {
//          break;
//      }
//      reg->stall = 0;
//  }
//  return data;
// }

// void cache_test_initial_miss() {
//  printf("====== cache_test_initial_miss ======\n");
//  Pipeline_Register *reg = pipeline_register_new(0, 0, 0, 0);

//  cache_t* c = cache_new(64, 4);
//  mem_write_32(0x10000000, 0x1234);
//  mem_write_32(0x10000004, 0x5678);
//  mem_write_32(0x10000008, 0x1357);
//  mem_write_32(0x10000010, 0x9876);
//  mem_write_32(0x10000020, 0x5555);

//  uint64_t data;

//  data = get_data(c, 0x10000000, reg);
//  assert(0x1234 == data);

//  data = get_data(c, 0x10000000, reg);
//  assert(0x1234 == data);

//  data = get_data(c, 0x10000004, reg);
//  assert(0x5678 == data);

//  data = get_data(c, 0x10000008, reg);
//  assert(0x1357 == data);

//  data = get_data(c, 0x10000010, reg);
//  assert(0x9876 == data);

//  data = get_data(c, 0x10000020, reg);
//  assert(0x5555 == data);

//  cache_destroy(c);
// }

// void cache_test_eviction() {
//  printf("====== cache_test_eviction ======\n");
//  Pipeline_Register *reg = pipeline_register_new(0, 0, 0, 0);

//  cache_t* c = cache_new(64, 4);
//  uint64_t data;
//  mem_write_32(0x10000000, 0x1111);
//  mem_write_32(0x10010000, 0x2222);
//  mem_write_32(0x10020000, 0x3333);
//  mem_write_32(0x10030000, 0x4444);
//  mem_write_32(0x10040000, 0x5555);
//  mem_write_32(0x10050000, 0x6666);

//  data = get_data(c, 0x10000000, reg);
//  assert(data == 0x1111);

//  data = get_data(c, 0x10010000, reg);
//  assert(data == 0x2222);

//  data = get_data(c, 0x10020000, reg);
//  assert(data == 0x3333);

//  data = get_data(c, 0x10030000, reg);
//  assert(data == 0x4444);

//  // Cache should have filled up; this should cause eviction of block 0
//  data = get_data(c, 0x10040000, reg);
//  printf("** expected eviction on block #0 **\n");
//  assert(data == 0x5555);

//  // Access block #1 to make it not LRU; block #2 should be LRU now
//  data = get_data(c, 0x10010000, reg);

//  data = get_data(c, 0x10050000, reg);
//  printf("** expected eviction on block #2 **\n");
//  assert(data == 0x6666);

//  cache_destroy(c);
// }

void cache_test()
{
    if (!DEBUG)
    {
        return;
    }
    // cache_test_init();
    // cache_test_initial_miss();
    // cache_test_eviction();
    // printf("====== FINISHED TESTS ======\n");
}
