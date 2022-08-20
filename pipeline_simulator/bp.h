/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2016
 */

#ifndef _BP_H_
#define _BP_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "shell.h"

/* CPU-level globals */
extern bp_t *bpt;

void bp_predict(Pipeline_Register *reg);

void bp_update(uint64_t addr, uint64_t target, uint64_t taken, uint64_t conditional, Pipeline_Register *reg);

#endif
