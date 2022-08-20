/*
 * CMSC 22200, Fall 2016
 *
 * ARM pipeline timing simulator
 *
 */
#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdint.h>
#include "shell.h"

/* CPU-level globals */
extern cache_t *instr_cache;
extern cache_t *data_cache;

uint32_t instr_cache_read(Pipeline_Register *reg, uint64_t addr);
uint32_t data_cache_read(Pipeline_Register *reg, uint64_t addr);
uint32_t data_cache_write(Pipeline_Register *reg, uint64_t addr, uint32_t data);
int data_cache_stall_count();
int instr_cache_stall_count();

void cache_init();
cache_t *cache_new(int sets, int ways);
void cache_flush(cache_t *c);
int any_cache_is_dirty();
int cache_is_dirty(cache_t *c);
void cache_destroy();
uint32_t cache_read(cache_t *c, uint64_t addr, Pipeline_Register *reg);
void cache_write(cache_t *c, uint64_t addr, uint32_t input, Pipeline_Register *reg);
void cache_writeback(cache_t *c, int set, int way);

void cache_test();

#endif
