/***************************************************************/
/*                                                             */
/*   ARM Instruction Level Simulator                       */
/*                                                             */
/*   CMSC-22200 Computer Architecture                                            */
/*   University of Chicago                                */
/*                                                             */
/***************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*          DO NOT MODIFY THIS FILE!                            */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#ifndef _SIM_SHELL_H_
#define _SIM_SHELL_H_

#include <stdint.h>
#include <stdio.h>

#define CONIFG_CPU_NUM              1 // 4
#define CONIFG_IMG_BASE_ADDR        0x00400000

#define info(str, ...)              printf(str, ##__VA_ARGS__)
#define err(str, ...)               printf("[error] " str, ##__VA_ARGS__)
#define msg(str, ...)               printf(str, ##__VA_ARGS__)
#define dbg(str, ...)               printf(str, ##__VA_ARGS__)


#define FALSE       0
#define TRUE        1

#define ARM_REGS    32
#define NUM_REGS    4


/* Branch prediction structs */

typedef struct
{
    uint64_t addr; // address tag; fetch stage PC
    int valid; // 1 = valid; 0 = invalid
    int conditional; // 1 = conditional; 0 unconditional
    uint64_t target; // target branch
} btb_t;

typedef struct
{
    /* global branch history register*/
    uint64_t GHR;
    /* gshare */
    uint64_t PHT[256];
    /* BTB */
    btb_t BTB[1024];
    // Should an array of data structure of BTB instead

} bp_t;

/* Cache structs */

typedef struct
{
    uint64_t tag;
    uint64_t valid_bit; // one for each block
    uint64_t dirty_bit; // 1 if need to write back, 0 if not,  one for each block
    uint64_t LRU; // Counter
    uint32_t data[8]; // 32 bytes of data for each block
} block;

typedef struct
{
    // tags will be stored separately
    uint64_t sets; // set number 64 or 256
    uint64_t ways; // each set has 4 ways (instruction) or 8 ways (data), each has a block
    block **blocks;

    int stall_count;
    // The starting address of the block we're trying to address.
    // Not always going to be the actual address we're interested in, but
    // the address we're interested in with the five lowest bits zeroed out.
    uint64_t stall_addr;
} cache_t;

/* Pipeline structs */

typedef struct Pipeline_Register
{
    uint32_t instruction;
    uint64_t pc;
    uint64_t data[NUM_REGS];
    int FLAG_N;
    int FLAG_Z;
    int FLAG_V;
    int FLAG_C;
    int noop; // set to 1 to tell stage to skip
    int stall; // 1 if we should stall for a cycle, 0 if not

    // prediction data
    uint64_t predicted_target;
    int predicted_taken;

    struct Pipeline_Register *next;
} Pipeline_Register;

typedef struct Pipeline_Queue
{
    Pipeline_Register *head;
    Pipeline_Register *tail;
} Pipeline_Queue;

/* Global structs */

typedef struct CPU_State
{
    /* register file state */
    int64_t REGS[ARM_REGS];
    int FLAG_N;        /* flag N */
    int FLAG_Z;        /* flag Z */
    int FLAG_V;        /* flag V */
    int FLAG_C;        /* flag C */

    /* program counter in fetch stage */
    uint64_t PC;

} CPU_State;

typedef struct CPU
{
    CPU_State *CURRENT_STATE;

    Pipeline_Queue *QUEUE_IF;
    Pipeline_Queue *QUEUE_IF_TO_ID;
    Pipeline_Queue *QUEUE_ID_TO_EX;
    Pipeline_Queue *QUEUE_EX_TO_MEM;
    Pipeline_Queue *QUEUE_MEM_TO_WB;

    int *ERET_SERIALIZING;

    int *RUN_BIT;
    int *stall_count;
    int CURR_CPU_NUM;

    int *exec_FLAG_N;
    int *exec_FLAG_Z;
    int *exec_FLAG_V;
    int *exec_FLAG_C;

    bp_t *bpt;
    cache_t *instr_cache;
    cache_t *data_cache;
} CPU;

/* global variable -- pipeline state */
extern int NUM_CPUS;
extern CPU **CPUS;

/* CPU-level globals */
extern int *RUN_BIT;
extern int *ERET_SERIALIZING;
extern CPU_State *CURRENT_STATE;
extern int CURR_CPU_NUM;

/* only the cache touches these functions */
uint32_t mem_read_32(uint64_t address);
void     mem_write_32(uint64_t address, uint32_t value);

/* statistics */
extern uint32_t stat_cycles, stat_inst_retire, stat_inst_fetch, stat_squash;

#endif
