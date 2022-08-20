/*
 * CMSC 22200, Fall 2016
 *
 * ARM pipeline timing simulator
 *
 * Jonathan Lee and Kevin Tse 2016
 */

#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include <limits.h>

/* CPU-level globals */
CPU_State *CURRENT_STATE; // TODO: make into a pointer
extern Pipeline_Queue *QUEUE_IF;
extern Pipeline_Queue *QUEUE_IF_TO_ID;
extern Pipeline_Queue *QUEUE_ID_TO_EX;
extern Pipeline_Queue *QUEUE_EX_TO_MEM;
extern Pipeline_Queue *QUEUE_MEM_TO_WB;
extern int *stall_count;
extern int *exec_FLAG_N;
extern int *exec_FLAG_Z;
extern int *exec_FLAG_V;
extern int *exec_FLAG_C;

/* CPU struct for multicore*/

/* boilerplate */
Pipeline_Register *pipeline_register_new(
    uint64_t data0,
    uint64_t data1,
    uint64_t data2,
    uint64_t data3
);

/* flush pipeline after branch misprediction */
void flush_IF_TO_ID();

/* called during simulator startup */
void pipe_init();

/* this function calls the others */
void pipe_cycle();

/* each of these functions implements one stage of the pipeline */
void pipe_stage_fetch();
void pipe_stage_decode();
void pipe_stage_execute();
void pipe_stage_mem();
void pipe_stage_wb();

uint64_t get_fragment_of(uint64_t item, uint64_t start, uint64_t end);
uint32_t get_opcode(uint32_t instruction);

#endif
