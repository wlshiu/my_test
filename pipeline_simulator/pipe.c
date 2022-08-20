/*
 * CMSC 22200
 *
 * ARM pipeline timing simulator
 *
 * Jonathan Lee (jonathanlee) and Kevin Tse (kevintse)
 */

#include "cache.h"
#include "pipe.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "bp.h"
#include "opcode.h"

#define DEBUG 1

#undef dbg
#if defined(DEBUG) && (DEBUG)
#define dbg(str, ...)               printf(str, ##__VA_ARGS__)
#else
#define dbg(str, ...)
#endif



/* Data array format:
   - R format: { Rm, shamt, Rn, Rd }
   - I format: { alu_immediate, Rn, Rd }
   - D format: { dt_address, op, Rn, Rt }
   - B format: { br_address }
   - CB format: { cond_br_address, Rt }
   - IW format: { MOV_immediate, Rd } */
#define R_RM    0
#define R_SHAMT 1
#define R_RN    2
#define R_RD    3

#define I_ALU_IMMEDIATE     0
#define I_RN                1
#define I_RD                2

#define D_DT_ADDRESS    0
#define D_OP            1
#define D_RN            2
#define D_RT            3

#define B_BR_ADDRESS    0

#define CB_COND_BR_ADDRESS  0
#define CB_RT               1

#define IW_MOV_IMMEDIATE    0
#define IW_RD               1

/* === CPU-level globals ===
 * These should be pointers that are modified each time we change
 * to a new CPU. */

/* global pipeline state */
CPU_State *CURRENT_STATE;
Pipeline_Queue *QUEUE_IF;
Pipeline_Queue *QUEUE_IF_TO_ID;
Pipeline_Queue *QUEUE_ID_TO_EX;
Pipeline_Queue *QUEUE_EX_TO_MEM;
Pipeline_Queue *QUEUE_MEM_TO_WB;
int *stall_count;

int *exec_FLAG_N = 0;
int *exec_FLAG_Z = 0;
int *exec_FLAG_V = 0;
int *exec_FLAG_C = 0;

/* === Boilerplate functions === */

/* Caller should pass in a struct containing instruction data */
Pipeline_Register *pipeline_register_new(
    uint64_t data0,
    uint64_t data1,
    uint64_t data2,
    uint64_t data3)
{
    Pipeline_Register *reg = malloc(sizeof(Pipeline_Register));
    memset(reg, 0, sizeof(Pipeline_Register));
    reg->data[0] = data0;
    reg->data[1] = data1;
    reg->data[2] = data2;
    reg->data[3] = data3;
    reg->noop = 0;
    reg->stall = 0;
    reg->next = NULL; // technically redundant but just in case
    return reg;
}

void pipeline_register_free(Pipeline_Register *reg)
{
    free(reg);
}

Pipeline_Queue *pipeline_queue_new()
{
    Pipeline_Queue *queue = malloc(sizeof(Pipeline_Queue));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

void pipeline_queue_free(Pipeline_Queue *queue)
{
    if (queue == NULL)
        return;

    Pipeline_Register *reg = queue->head;
    while (reg != NULL)
    {
        Pipeline_Register *next = reg->next;
        pipeline_register_free(reg);
        reg = next;
    }
    free(queue);
}

/* Push a new register to the end of the queue */
void pipeline_queue_push(Pipeline_Queue *queue, Pipeline_Register *reg)
{
    if (queue == NULL)
    {
        dbg("pipeline_queue_push: ERROR null queue\n");
        return;
    }

    if (reg == NULL)
    {
        dbg("pipeline_queue_push: ERROR null reg\n");
    }

    Pipeline_Register *current_tail = queue->tail;
    if (current_tail != NULL)
        current_tail->next = reg;
    queue->tail = reg;

    Pipeline_Register *current_head = queue->head;
    if (current_head == NULL)
        queue->head = reg;

    return;
}

/* Push a new register to the front of the queue */
void pipeline_queue_push_front(Pipeline_Queue *queue, Pipeline_Register *reg)
{
    if (queue == NULL)
        return;
    Pipeline_Register *current_head = queue->head;
    reg->next = current_head;
    queue->head = reg;

    Pipeline_Register *current_tail = queue->tail;
    if (current_tail == NULL)
        queue->tail = reg;
}

/* Pop the register currently at the head of the queue */
Pipeline_Register *pipeline_queue_pop(Pipeline_Queue *queue)
{
    if (queue == NULL)
        return NULL;

    Pipeline_Register *current_head = queue->head;
    if (current_head != NULL)
    {
        queue->head = current_head->next;
        Pipeline_Register *current_tail = queue->tail;
        if (current_tail == current_head) // was a 1-element queue
        {
            queue->tail = current_head->next; // should be NULL
        }
    }
    return current_head;
}


// Returns 1 if queue is empty, else 0.
int pipeline_queue_is_empty(Pipeline_Queue *queue)
{
    return queue->head == NULL ? 1 : 0;
}

/* === Util functions === */

/* Gets the bit fragment from starting bit to ending bit, inclusive. */
uint64_t get_fragment_of(uint64_t item, uint64_t start, uint64_t end)
{
    if (start == 0 && end == 63)
        return item;

    uint64_t long_zero = 0;
    uint64_t long_one = 1;
    uint64_t mask = ~(~long_zero << (end - start + long_one));

    return (item >> start) & mask;
}

/* Sometimes we want to know the register # of Rd */
uint64_t get_destination_register(uint64_t instruction)
{
    return get_fragment_of(instruction, 0, 4);
}

uint32_t get_opcode(uint32_t instruction)
{
    return get_fragment_of(instruction, 21, 31);
}

uint32_t get_shamt(uint32_t instruction)
{
    return get_fragment_of(instruction, 10, 15);
}

uint64_t sign_extend(uint64_t item, uint64_t sign_bit_index)
{
    uint64_t long_one = 1;
    uint64_t sign = !!(item & (long_one << sign_bit_index));
    uint64_t base = 0;
    if (sign) // sign bit is turned on
    {
        uint64_t mask = ~0 << (sign_bit_index + 1);
        base = base | mask;
    }
    return base | item;

}

Pipeline_Register *get_R_data(uint32_t instruction)
{
    uint64_t Rm = CURRENT_STATE->REGS[get_fragment_of(instruction, 16, 20)];
    uint64_t shamt = get_fragment_of(instruction, 10, 15);
    uint64_t Rn = CURRENT_STATE->REGS[get_fragment_of(instruction, 5, 9)];
    uint64_t Rd = CURRENT_STATE->REGS[get_fragment_of(instruction, 0, 4)];
    return pipeline_register_new(Rm, shamt, Rn, Rd);
}

Pipeline_Register *get_I_data(uint32_t instruction)
{
    uint64_t alu_immediate = get_fragment_of(instruction, 10, 21);
    uint64_t Rn = CURRENT_STATE->REGS[get_fragment_of(instruction, 5, 9)];
    uint64_t Rd = CURRENT_STATE->REGS[get_fragment_of(instruction, 0, 4)];
    return pipeline_register_new(alu_immediate, Rn, Rd, 0);
}

Pipeline_Register *get_D_data(uint32_t instruction)
{
    uint64_t dt_address = get_fragment_of(instruction, 12, 20);
    uint64_t op = get_fragment_of(instruction, 10, 11);
    uint64_t Rn = CURRENT_STATE->REGS[get_fragment_of(instruction, 5, 9)];
    uint64_t Rt = CURRENT_STATE->REGS[get_fragment_of(instruction, 0, 4)];
    return pipeline_register_new(dt_address, op, Rn, Rt);
}

Pipeline_Register *get_B_data(uint32_t instruction)
{
    uint64_t br_address = get_fragment_of(instruction, 0, 25);
    return pipeline_register_new(br_address, 0, 0, 0);
}

Pipeline_Register *get_CB_data(uint32_t instruction)
{
    uint64_t cond_br_address = get_fragment_of(instruction, 5, 23);
    uint64_t Rt = CURRENT_STATE->REGS[get_fragment_of(instruction, 0, 4)];
    return pipeline_register_new(cond_br_address, Rt, 0, 0);
}

Pipeline_Register *get_IW_data(uint32_t instruction)
{
    uint64_t MOV_immediate = get_fragment_of(instruction, 5, 20);
    uint64_t Rd = CURRENT_STATE->REGS[get_fragment_of(instruction, 0, 4)];
    return pipeline_register_new(MOV_immediate, Rd, 0, 0);
}

/* Sets the negative (N) and zero (Z) flags based on the given value */
void set_NZ_flags(Pipeline_Register *reg, uint64_t value)
{
    *exec_FLAG_Z = value == 0 ? 1 : 0;
    *exec_FLAG_N = (int64_t)value < 0 ? 1 : 0;

    if (*exec_FLAG_Z)   dbg("Z flag set\n");
    else                dbg("Z flag cleared\n");

    if (*exec_FLAG_N)   dbg("N flag set\n");
    else                dbg("N flag cleared\n");

    return;
}

void clear_CV_flags(Pipeline_Register *reg)
{
    *exec_FLAG_C = 0;
    *exec_FLAG_V = 0;

    dbg("C flag cleared\nV flag cleared\n");
    return;
}

void set_flags_ADD(Pipeline_Register *reg, uint64_t operand1, uint64_t operand2, uint64_t result)
{
    set_NZ_flags(reg, result);

    uint64_t long1 = 1;
    uint64_t mask = long1 << 63;
    uint64_t sign1 = !!(operand1 & mask);
    uint64_t sign2 = !!(operand2 & mask);
    uint64_t sign_result = !!(result & mask);

    *exec_FLAG_C = !!(sign1 == 1 || sign2 == 1) & !!(sign_result == 0);
    *exec_FLAG_V = !!(sign1 == 0 && sign2 == 0 && sign_result == 1) |
                   !!(sign1 == 1 && sign2 == 1 && sign_result == 0);

    if (*exec_FLAG_V)   dbg("V flag set\n");
    else                dbg("V flag cleared\n");

    if (*exec_FLAG_C)   dbg("C flag set\n");
    else                dbg("C flag cleared\n");

    return;
}

void set_flags_SUB(Pipeline_Register *reg, uint64_t operand1, uint64_t operand2, uint64_t result)
{
    set_NZ_flags(reg, result);

    uint64_t long1 = 1;
    uint64_t mask = long1 << 63;
    uint64_t sign1 = !!(operand1 & mask);
    uint64_t sign2 = !!(operand2 & mask);
    uint64_t sign_result = !!(result & mask);

    *exec_FLAG_C = !!(sign1 == 0 && sign_result == 1);
    *exec_FLAG_V = !!(sign1 == 0 && sign2 == 1 && sign_result == 1) |
                   !!(sign1 == 1 && sign2 == 0 && sign_result == 0);

    if (DEBUG)
    {
        if (*exec_FLAG_V)
        {
            printf("V flag set\n");
        }
        else
        {
            printf("V flag cleared\n");
        }
        if (*exec_FLAG_C)
        {
            printf("C flag set\n");
        }
        else
        {
            printf("C flag cleared\n");
        }
    }
}

void stall(int n)
{
    *stall_count += n;
}

void flush_IF_TO_ID()
{
    Pipeline_Register *reg = pipeline_queue_pop(QUEUE_IF_TO_ID);
    pipeline_register_free(reg);
    stall(1);
}

// Processes

void process_ADD(Pipeline_Register *reg)
{
    // R Format
    uint64_t operand1 = reg->data[2];
    uint64_t operand2 = reg->data[0];
    uint64_t result = operand1 + operand2;

    reg->data[0] = result;

    dbg("process_ADD ran.\n");
    return;
}

void process_ADDS(Pipeline_Register *reg)
{
    // R Format
    uint64_t operand1 = reg->data[2];
    uint64_t operand2 = reg->data[0];

    uint64_t result = operand1 + operand2;

    reg->data[0] = result;


    dbg("process_ADDS ran.\n");

    set_flags_ADD(reg, operand1, operand2, result);
    return;
}

void process_ADDI(Pipeline_Register *reg)
{
    // I Format
    uint64_t operand1 = reg->data[1];
    uint64_t operand2 = reg->data[0];
    uint64_t result = operand1 + operand2;

    reg->data[0] = result;

    dbg("process_ADDI ran.\n");
    return;
}

void process_ADDIS(Pipeline_Register *reg)
{
    // I Format
    uint64_t operand1 = reg->data[1];
    uint64_t operand2 = reg->data[0];
    uint64_t result = operand1 + operand2;

    reg->data[0] = result;

    dbg("process_ADDIS ran.\n");

    set_flags_ADD(reg, operand1, operand2, result);
    return;
}

void process_AND(Pipeline_Register *reg)
{
    // R Format
    uint64_t operand1 = reg->data[2];
    uint64_t operand2 = reg->data[0];

    uint64_t result = operand1 & operand2;

    reg->data[0] = result;

    dbg("process_AND ran.\n");
    return;
}

void process_ANDS(Pipeline_Register *reg)
{
    // R Format
    uint64_t operand1 = reg->data[2];
    uint64_t operand2 = reg->data[0];

    uint64_t result = operand1 & operand2;

    reg->data[0] = result;

    set_NZ_flags(reg, result);
    clear_CV_flags(reg);

    dbg("process_ANDS ran.\n");
    return;
}

void process_B(Pipeline_Register *reg)
{
    // B Format
    uint64_t br_address = sign_extend(reg->data[0], 25);
    br_address = br_address << 2;

    uint64_t target = reg->pc + br_address;

    // CURRENT_STATE->PC = target;
    bp_update(reg->pc, target, 1, 0, reg); // 1 = taken, 0 = unconditional

    reg->noop = 1;

    dbg("process_B ran.\n");
    return;
}

void process_BL(Pipeline_Register *reg)
{
    // B Format
    uint64_t br_address = sign_extend(reg->data[0], 25);
    br_address = br_address << 2;

    uint64_t target = reg->pc + br_address;
    uint64_t link = reg->pc + 4;

    bp_update(reg->pc, target, 1, 0, reg); // 1 = taken, 0 = unconditional

    reg->data[0] = link;

    dbg("process_BL ran.\n");
    return;
}

void process_BR(Pipeline_Register *reg)
{
    // R Format
    uint64_t target = reg->data[2];
    bp_update(reg->pc, target, 1, 0, reg); // 1 = taken, 0 = unconditional

    reg->noop = 1;

    dbg("process_BR ran.\n");
    return;
}


// BEQ, BNE, BGT, BLT, BGE, BLE

void process_BEQ(Pipeline_Register *reg)
{
    // CB Format
    uint64_t offset = sign_extend(reg->data[0], 18);
    offset = offset << 2;

    uint64_t target = reg->pc + offset;
    int taken = 0;

    if (*exec_FLAG_Z == 1)   // If Zero flag is set, it will be 1
    {
        taken = 1;
        dbg("process_BEQ: Zero flag is set, we are branching\n");
    }
    else
    {
        dbg("process_BEQ: Zero flag is not set, we are not branching\n");
    }
    bp_update(reg->pc, target, taken, 1, reg);
    reg->noop = 1;
    return;
}

void process_BNE(Pipeline_Register *reg)
{
    // CB Format
    uint64_t offset = sign_extend(reg->data[0], 18);
    offset = offset << 2;

    uint64_t target = reg->pc + offset;
    int taken = 0;

    if (*exec_FLAG_Z == 0)   // If Zero flag is set, it will be 1
    {
        taken = 1;
        dbg("process_BNE: Zero flag is not set, we are branching\n");
    }
    else
    {
        dbg("process_BNE: Zero flag is set, we are not branching\n");
    }
    bp_update(reg->pc, target, taken, 1, reg);
    reg->noop = 1;
    return;
}

void process_BGT(Pipeline_Register *reg)
{
    // CB Format
    uint64_t offset = sign_extend(reg->data[0], 18);
    offset = offset << 2;

    uint64_t target = reg->pc + offset;
    int taken = 0;

    // !Z and (N == V)
    if (*exec_FLAG_Z == 0 && *exec_FLAG_N == *exec_FLAG_V)
    {
        taken = 1;
        dbg("process_BGT: We are branching\n");
    }
    else
    {
        dbg("process_BGT: We are not branching\n");
    }

    dbg("process_BGT: predicted_taken = %d\n", reg->predicted_taken);

    bp_update(reg->pc, target, taken, 1, reg);
    reg->noop = 1;
    return;
}

void process_BLT(Pipeline_Register *reg)
{
    // CB Format

    uint64_t offset = sign_extend(reg->data[0], 18);
    offset = offset << 2;

    uint64_t target = reg->pc + offset;
    int taken = 0;

    // N != V
    if (*exec_FLAG_N != *exec_FLAG_V)
    {
        taken = 1;
        dbg("process_BLT: We are branching\n");
    }
    else
    {
        dbg("process_BLT: We are not branching\n");
    }
    bp_update(reg->pc, target, taken, 1, reg);
    reg->noop = 1;
    return;
}

void process_BGE(Pipeline_Register *reg)
{
    // CB Format
    uint64_t offset = sign_extend(reg->data[0], 18);
    offset = offset << 2;

    uint64_t target = reg->pc + offset;
    int taken = 0;

    // N == V
    if (*exec_FLAG_N == *exec_FLAG_V)
    {
        taken = 1;
        dbg("process_BGE: We are branching\n");
    }
    else
    {
        dbg("process_BGE: We are not branching\n");
    }
    bp_update(reg->pc, target, taken, 1, reg);
    reg->noop = 1;
    return;
}

void process_BLE(Pipeline_Register *reg)
{
    // CB Format

    uint64_t offset = sign_extend(reg->data[0], 18);
    offset = offset << 2;

    uint64_t target = reg->pc + offset;
    int taken = 0;

    // Z or (N != V)
    if (*exec_FLAG_Z == 1 || *exec_FLAG_N != *exec_FLAG_V)
    {
        taken = 1;
        dbg("process_BLE: We are branching\n");
    }
    else
    {
        dbg("process_BLE: We are not branching\n");
    }
    bp_update(reg->pc, target, taken, 1, reg);
    reg->noop = 1;
    return;
}

void process_HLT(Pipeline_Register *reg)
{
    stall(1);
    // do nothing, because we should handle this in mem instead
}

void process_SDIV(Pipeline_Register *reg)
{
    // R format
    int64_t operand1 = reg->data[R_RN];
    int64_t operand2 = reg->data[R_RM];
    int64_t result = operand1 / operand2;

    reg->data[0] = result;

    dbg("process_SDIV\n");
    return;
}

void process_UDIV(Pipeline_Register *reg)
{
    // R format
    uint64_t operand1 = reg->data[R_RN];
    uint64_t operand2 = reg->data[R_RM];
    uint64_t result = operand1 / operand2;

    reg->data[0] = result;

    dbg("process_UDIV\n");
    return;
}

void process_MOVZ(Pipeline_Register *reg)
{
    // IW format
    uint64_t shift = get_fragment_of(reg->instruction, 21, 22);
    uint64_t result = reg->data[IW_MOV_IMMEDIATE] << (shift * 16);

    reg->data[0] = result;

    dbg("process_MOVZ\n");
    return;
}

void process_CBZ(Pipeline_Register *reg)
{
    // CB Format

    uint64_t offset = sign_extend(reg->data[0], 18);
    offset = offset << 2;

    uint64_t target = reg->pc + offset;
    int taken = 0;

    if (reg->data[1] == 0)   // If Zero flag is set, it will be 1
    {
        dbg("process_CBZ: Register equals to zero, we are branching\n");
        taken = 1;
    }
    else
    {
        dbg("process_CBZ: Register does not equal to zero, we are not branching\n");
    }
    bp_update(reg->pc, target, taken, 1, reg);
    reg->noop = 1;
    return;
}

void process_CBNZ(Pipeline_Register *reg)
{
    // CB Format

    uint64_t offset = sign_extend(reg->data[0], 18);
    offset = offset << 2;

    uint64_t target = reg->pc + offset;
    int taken = 0;

    if (reg->data[1] != 0)   // If Zero flag is not set, it will be 0
    {
        dbg("process_CBNZ: Register does not equal to zero, we are branching\n");
        taken = 1;
    }
    else
    {
        dbg("process_CBNZ: Register equals to zero, we are not branching\n");
    }
    bp_update(reg->pc, target, taken, 1, reg);
    reg->noop = 1;
    return;
}

void process_LSL(Pipeline_Register *reg, uint64_t src, uint64_t shift, uint32_t Rd)
{
    uint64_t result = src << shift;

    reg->data[0] = result;
    dbg("process_LSL: 0x%llx << %llu = 0x%llx\n",
        (long long unsigned) src,
        (long long unsigned) shift,
        (long long unsigned) result);
    return;
}

void process_LSL_register(Pipeline_Register *reg)
{
    // R Format

    process_LSL(
        reg,
        reg->data[2], // src
        reg->data[0] & 63, // shift
        reg->data[3]
    );
}

void process_LSL_immediate(Pipeline_Register *reg)
{
    // I Forma
    uint32_t Rd = reg->data[2];
    uint32_t Rn = reg->data[1];
    uint32_t uimm = reg->data[0];
    process_LSL(
        reg,
        Rn, // src
        -uimm % 64, // shift
        Rd
    );
}

void process_LSR(Pipeline_Register *reg, uint64_t src, uint64_t shift, uint32_t Rd)
{
    uint64_t result = src >> shift;
    // Set right most bits to zero
    uint64_t mask = ~(~0 << (63 - shift));
    result = result & mask;

    reg->data[0] = result;
    dbg("process_LSR: 0x%llx >> %llu = 0x%llx\n",
        (long long unsigned) src,
        (long long unsigned) shift,
        (long long unsigned) result);
    return;
}

void process_LSR_register(Pipeline_Register *reg)
{
    // R Format

    process_LSR(
        reg,
        reg->data[2], // src
        reg->data[0] & 63, // shift
        reg->data[3]
    );
}

void process_LSR_immediate(Pipeline_Register *reg)
{

    uint32_t Rd = reg->data[2];
    uint32_t Rn = reg->data[1];
    uint32_t uimm = reg->data[0];;
    process_LSR(
        reg,
        Rn, // src
        uimm, // shift
        Rd
    );
}

void process_MUL(Pipeline_Register *reg)
{

    // R format

    uint64_t operand1 = reg->data[2];
    uint64_t operand2 = reg->data[0];
    uint64_t result = operand1 * operand2;

    reg->data[0] = result;

    dbg("process_MUL\n");
    return;
}

// Generic function to handle all kinds of STU* instructions
void process_STU(Pipeline_Register *reg, uint32_t start, uint32_t stop)
{
    // D format
    uint64_t dt_addr = sign_extend(reg->data[D_DT_ADDRESS], 8);
    uint64_t addr = reg->data[D_RN] + dt_addr + (start / 8);

    uint64_t fragment = get_fragment_of(reg->data[D_RT], (uint64_t)start, (uint64_t)stop);

    reg->data[0] = addr;
    reg->data[1] = fragment;

    dbg("process_STU: writing 0x%llx to 0x%llx\n", (long long unsigned)fragment, (long long unsigned)addr);
    return;
}

void process_STUR(Pipeline_Register *reg)
{
    process_STU(reg, 0, 63);
}

void process_STURB(Pipeline_Register *reg)
{
    process_STU(reg, 0, 7);
}

void process_STURH(Pipeline_Register *reg)
{
    process_STU(reg, 0, 15);
}

void process_STURW(Pipeline_Register *reg)
{
    process_STU(reg, 0, 31);
}

void process_SUB(Pipeline_Register *reg)
{
    // R format
    uint64_t operand1 = reg->data[R_RN];
    uint64_t operand2 = reg->data[R_RM];
    uint64_t result = operand1 - operand2;

    reg->data[0] = result;

    dbg("process_SUB\n");
    return;
}

void process_SUBI(Pipeline_Register *reg)
{
    // I format
    uint64_t operand1 = reg->data[I_RN];
    uint64_t operand2 = reg->data[I_ALU_IMMEDIATE];
    uint64_t result = operand1 - operand2;

    reg->data[0] = result;

    dbg("process_SUBI\n");
    return;
}

void process_SUBIS(Pipeline_Register *reg)
{
    // I format
    uint64_t operand1 = reg->data[I_RN];
    uint64_t operand2 = reg->data[I_ALU_IMMEDIATE];
    uint64_t result = operand1 - operand2;

    reg->data[0] = result;
    set_flags_SUB(reg, operand1, operand2, result);

    dbg("process_SUBI\n");
    return;
}

void process_SUBS(Pipeline_Register *reg)
{
    // R format
    uint64_t operand1 = reg->data[R_RN];
    uint64_t operand2 = reg->data[R_RM];
    uint64_t result = operand1 - operand2;

    reg->data[0] = result;
    set_flags_SUB(reg, operand1, operand2, result);

    dbg("process_SUBS\n");
    return;
}

void process_EOR(Pipeline_Register *reg)
{
    // R format
    uint64_t operand1 = reg->data[R_RN];
    uint64_t operand2 = reg->data[R_RM];
    uint64_t result = operand1 ^ operand2;

    reg->data[0] = result;

    dbg("process_EOR\n");
    return;
}

void process_ORR(Pipeline_Register *reg)
{
    // R format
    uint64_t operand1 = reg->data[R_RN];
    uint64_t operand2 = reg->data[R_RM];
    uint64_t result = operand1 | operand2;

    reg->data[0] = result;

    dbg("process_ORR\n");
    return;
}

void process_LDUR(Pipeline_Register *reg)
{
    // D format
    uint64_t dt_addr = sign_extend(reg->data[D_DT_ADDRESS], 8);
    uint64_t result = reg->data[D_RN] + dt_addr;

    reg->data[0] = result;

    dbg("process_LDUR\n");
    dbg("process_ADDIS ran.\n");
    return;
}

void process_LDURB(Pipeline_Register *reg)
{
    // D format
    uint64_t dt_addr = sign_extend(reg->data[D_DT_ADDRESS], 8);
    uint64_t result = reg->data[D_RN] + dt_addr;
    reg->data[0] = result;

    dbg("process_LDURB\n");
    return;
}

void process_LDURH(Pipeline_Register *reg)
{
    // D format
    uint64_t dt_addr = sign_extend(reg->data[D_DT_ADDRESS], 8);
    uint64_t result = reg->data[D_RN] + dt_addr;
    reg->data[0] = result;

    dbg("process_LDURH\n");
    return;
}

void data_forwarding(uint64_t des, uint64_t rv)
{
    // In exec, somehow get des from the other stack, but should have already written back

    // Isnt called by B or IW Format

    // des is the destination register that the first instruction is writing to
    // input1, 2 are the registers the next instructions are reading from

    // let input1 or 2 be a value greater than 32 if the input are immediates or does not exist

    // make sure inputs are not immediates, but register numbers
    // make sure flags are passed on as well

    Pipeline_Register *reg = pipeline_queue_pop(QUEUE_ID_TO_EX);

    if (reg == NULL)
    {
        dbg("data_forwarding: found NULL; not forwarding\n");
        return;
    }

    pipeline_queue_push(QUEUE_ID_TO_EX, reg);

    uint32_t instruction = reg->instruction;
    uint32_t opcode = get_opcode(instruction);

    dbg("data_forwarding: looking at instruction 0x%x\n", opcode);

    // HLT
    if (opcode == OPCODE_HLT)
    {
        dbg("data_forwarding: found HLT; not forwarding\n");
        return;
    }

    // R format (check Rn and Rm)
    if (opcode == 0x458 || opcode == 0x558 ||  // ADD || ADDS
            opcode == 0x450 || opcode == 0x750 ||  // AND || ANDS
            opcode == 0x650 || opcode == 0x550 || // EOR || ORR
            opcode == 0x658 || opcode == 0x758 || // SUB || SUBS
            opcode == 0x4D8 || // MUL
            opcode == 0x4D6 || // SDIV, UDIV
            opcode == 0x6B0 || // BR
            opcode == 0x4D6) // LSL/LSR (reg)
    {
        dbg("data_forwarding: checking R format instruction\n");

        uint64_t Rm = get_fragment_of(instruction, 16, 20); // Check if Register Numbers are the same (Rm == des)
        if (Rm == des)
        {
            dbg("data_forwarding: forwarded value 0x%llx to register %llu\n", (long long unsigned)rv, (long long int)des);
            reg->data[R_RM] = rv;
        }
        uint64_t Rn = get_fragment_of(instruction, 5, 9); // (Rn == des)
        if (Rn == des)
        {
            dbg("data_forwarding: forwarded value 0x%llx to register %llu\n", (long long unsigned)rv, (long long int)des);
            reg->data[R_RN] = rv;
        }
    }
    // I format (check Rn)
    else if ((0x488 <= opcode && opcode <= 0x489) || // ADDI
             (0x588 <= opcode && opcode <= 0x589) || // ADDIS
             (0x688 <= opcode && opcode <= 0x689) || // SUBI
             (0x788 <= opcode && opcode <= 0x789) || // SUBIS
             (0x69A <= opcode && opcode <= 0x69B)) // LSL/LSR (imm)
    {
        dbg("data_forwarding: checking I format instruction\n");

        uint64_t Rn = get_fragment_of(instruction, 5, 9);
        if (Rn == des)
        {
            dbg("data_forwarding: forwarded value 0x%llx to register %llu\n", (long long unsigned)rv, (long long int)des);
            reg->data[I_RN] = rv;
        }
    }
    // D Format (check Rd)
    else if (opcode == 0x7C2 || opcode == 0x1C2 || opcode == 0x3C2) // LDUR || LDURB || LDURH
    {
        dbg("data_forwarding: checking D format instruction\n");

        // Should we implement a stall here? Just for the loading case
        // That should be the only difference between forwarding from WB vs MEM
        // make sure when we shall, we shall all pipelines after
        uint64_t Rn = get_fragment_of(instruction, 5, 9);
        if (Rn == des)
        {
            dbg("data_forwarding: forwarded value 0x%llx to register %llu\n", (long long unsigned)rv, (long long int)des);
            reg->data[D_RN] = rv;
        }
    }
    // STUR || STURB || STURH || STURW
    else if (opcode == 0x7C0 || opcode == 0x1C0 ||
             opcode == 0x3C0 || opcode == 0x5C0)
    {
        uint64_t Rn = get_fragment_of(instruction, 5, 9);
        if (Rn == des)
        {
            dbg("data_forwarding: forwarded value 0x%llx to register %llu\n", (long long unsigned)rv, (long long int)des);
            reg->data[D_RN] = rv;
        }
        uint64_t Rt = get_fragment_of(instruction, 0, 4);
        if (Rt == des)
        {
            dbg("data_forwarding: forwarded value 0x%llx to register %llu\n", (long long unsigned)rv, (long long int)des);
            reg->data[D_RT] = rv;
        }

    }
    // CB format (check Rt)
    else if ((0x5A8 <= opcode && opcode <= 0x5AF) || // CBNZ
             (0x5A0 <= opcode && opcode <= 0x5A7) || // CBZ
             (0x2A0 <= opcode && opcode <= 0x2A7)) // Bcond (BEQ, BNE, BGE, BLT, BGT, BLE)
    {
        dbg("data_forwarding: checking CB format instruction\n");

        uint64_t Rt = get_fragment_of(instruction, 0, 4);
        if (Rt == des)
        {
            dbg("data_forwarding: forwarded value 0x%llx to register %llu\n", (long long unsigned)rv, (long long int)des);
            reg->data[CB_RT] = rv;
        }
    }
    return;
}

void data_forwarding_mem(uint64_t des)
{
    // If the next instruction to execution stage uses the register
    // that we're writing to, force a one-cycle stall.
    // This only applies to LOAD instructions.
    Pipeline_Register *reg = pipeline_queue_pop(QUEUE_ID_TO_EX);

    if (reg == NULL)
        return;

    pipeline_queue_push(QUEUE_ID_TO_EX, reg);

    uint32_t instruction = reg->instruction;
    uint32_t opcode = get_opcode(instruction);

    // HLT
    if (opcode == 0x6a2)
    {
        if (DEBUG)
            printf("data_forwarding_mem: found HLT; not forwarding\n");
        return;
    }

    // R format (check Rn and Rm)
    if (opcode == 0x458 || opcode == 0x558 ||  // ADD || ADDS
            opcode == 0x450 || opcode == 0x750 ||  // AND || ANDS
            opcode == 0x650 || opcode == 0x550 || // EOR || ORR
            opcode == 0x658 || opcode == 0x758 || // SUB || SUBS
            opcode == 0x4D8 || // MUL
            opcode == 0x4D6 || // SDIV, UDIV
            opcode == 0x6B0 || // BR
            opcode == 0x4D6) // LSL/LSR (reg)
    {
        uint64_t Rm = get_fragment_of(instruction, 16, 20); // Check if Register Numbers are the same (Rm == des)
        uint64_t Rn = get_fragment_of(instruction, 5, 9); // (Rn == des)
        if (Rm == des || Rn == des)
            stall(1);
    }
    // I format (check Rn)
    else if ((0x488 <= opcode && opcode <= 0x489) || // ADDI
             (0x588 <= opcode && opcode <= 0x589) || // ADDIS
             (0x688 <= opcode && opcode <= 0x689) || // SUBI
             (0x788 <= opcode && opcode <= 0x789) || // SUBIS
             (0x69A <= opcode && opcode <= 0x69B)) // LSL/LSR (imm)
    {
        uint64_t Rn = get_fragment_of(instruction, 5, 9);
        if (Rn == des)
            stall(1);
    }
    // D Format (check Rd)
    else if (opcode == 0x7C0 || opcode == 0x1C0 ||
             opcode == 0x3C0 || opcode == 0x5C0) // STUR || STURB || STURH || STURW
    {
        uint64_t Rn = get_fragment_of(instruction, 5, 9);
        uint64_t Rt = get_fragment_of(instruction, 0, 4);
        if (Rn == des || Rt == des)
            stall(1);
    }
    else if (opcode == 0x7C2 || opcode == 0x1C2 || opcode == 0x3C2) // LDUR || LDURB || LDURH
    {
        // Should we implement a stall here? Just for the loading case
        // That should be the only difference between forwarding from WB vs MEM
        // make sure when we shall, we shall all pipelines after
        uint64_t Rn = get_fragment_of(instruction, 5, 9);
        if (Rn == des)
            stall(1);
    }
    // CB format (check Rt)
    else if ((0x5A8 <= opcode && opcode <= 0x5AF) || // CBNZ
             (0x5A0 <= opcode && opcode <= 0x5A7) || // CBZ
             (0x2A0 <= opcode && opcode <= 0x2A7)) // Bcond (BEQ, BNE, BGE, BLT, BGT, BLE)
    {
        uint64_t Rt = get_fragment_of(instruction, 0, 4);
        if (Rt == des)
            stall(1);
    }
}

// Run in writeback
CPU *new_cpu_init(int cpu_num, uint64_t init_PC)
{
    CPU *new_cpu = malloc(sizeof(CPU));
    CPU_State *state = malloc(sizeof(CPU_State));

    new_cpu->CURRENT_STATE = state;

    new_cpu->CURRENT_STATE->REGS[29] = 1;
    new_cpu->CURRENT_STATE->REGS[30] = cpu_num;
    // new_cpu->CURR_CPU_NUM = cpu_num;
    new_cpu->CURRENT_STATE->PC = init_PC + 4;
    return new_cpu;
}

void pipe_init(CPU *new_cpu)
{
    if (DEBUG)
    {
        printf("pipe_init\n");
    }
    new_cpu->QUEUE_IF = pipeline_queue_new();
    new_cpu->QUEUE_IF_TO_ID = pipeline_queue_new();
    new_cpu->QUEUE_ID_TO_EX = pipeline_queue_new();
    new_cpu->QUEUE_EX_TO_MEM = pipeline_queue_new();
    new_cpu->QUEUE_MEM_TO_WB = pipeline_queue_new();
}

void pipe_cycle()
{
    if (DEBUG)
        printf("=== begin cycle %u ===\n", stat_cycles + 1);

    pipe_stage_wb();
    pipe_stage_mem();
    pipe_stage_execute();
    pipe_stage_decode();
    pipe_stage_fetch();

    // ERET
    if (*stall_count > 0)
    {
        //if ((*stall_count > 0) && ((*ERET_SERIALIZING) == 0)) {
        *stall_count = *stall_count - 1;
    }
    if (*RUN_BIT == 0)
    {
        cache_destroy();
    }
    if (DEBUG) printf("=== end cycle %u ===\n", stat_cycles + 1);
}

void pipe_stage_wb()
{
    if (DEBUG) printf("WRITEBACK: beginning\n");

    if (*stall_count > 0)
    {
        if (DEBUG)
            printf("WRITEBACK: stalling for %d instruction(s)\n", *stall_count);
        return;
    }

    Pipeline_Register *reg = pipeline_queue_pop(QUEUE_MEM_TO_WB);
    if (reg == NULL)
    {
        if (DEBUG) printf("WRITEBACK: found null register, so skipping\n");
        return;
    }

    uint32_t instruction = reg->instruction;
    uint32_t opcode = get_opcode(instruction);

    stat_inst_retire++;

    // Set flags
    CURRENT_STATE->FLAG_N = reg->FLAG_N;
    CURRENT_STATE->FLAG_V = reg->FLAG_V;
    CURRENT_STATE->FLAG_Z = reg->FLAG_Z;
    CURRENT_STATE->FLAG_C = reg->FLAG_C;

    if (reg->noop)
    {
        if (DEBUG) printf("WRITEBACK: noop\n");
        return;
    }

    // STUR || STURB || STURH || STURW
    if (opcode == 0x7C0 || opcode == 0x1C0 || opcode == 0x3C0 || opcode == 0x5C0)
    {
        // Do nothing for STUR in WB
    }
    else if (instruction == 0xd69f03e0)
    {
        if (DEBUG)
        {
            printf("ERET at writeback\n");
        }
        // Check X30 for which CPU to start
        // What about data forwarding?

        // Find which CPU to initiate
        int cpu_num = CURRENT_STATE->REGS[30];

        if (cpu_num == 11) // Special case for sys call
        {
            printf("OUT (CPU %d): %lx\n", CURR_CPU_NUM, CURRENT_STATE->REGS[29]);
        }
        else
        {
            // Set current cpu's reg 29 to 0
            if (DEBUG)
                printf("About to set REG 29 to Zero\n");
            CURRENT_STATE->REGS[29] = 0;
            CPUS[cpu_num]->CURRENT_STATE->REGS[29] = 1;
            CPUS[cpu_num]->CURRENT_STATE->PC = reg->pc + 4;
            if (DEBUG)
                printf("About to set run bit of new CPU to 1\n");
            *(CPUS[cpu_num]->RUN_BIT) = 1;
        }
        *ERET_SERIALIZING = 0;
    }
    else
    {
        /* If instruction is BL, save to register 30; otherwise,
           we save it to the register number encoded in bits 0-4. */
        int is_BL = 0x4A0 <= opcode && opcode <= 0x4BF;
        uint64_t dest = is_BL ? 30 : get_fragment_of(instruction, 0, 4);

        /* We always assume the data to store is in register 0. */
        uint64_t data = reg->data[0];

        /* Store the data to the destination register, and handle
           data dependencies. */
        if (DEBUG)
        {
            printf(
                "WRITEBACK: setting R[%llu] = 0x%llx\n",
                (long long unsigned)dest,
                (long long unsigned)data
            );
        }
        CURRENT_STATE->REGS[dest] = data;

        // R31 is the zero register, so it should always be zero.
        CURRENT_STATE->REGS[31] = 0;

        // Update the value inside of destination register in subsequent instrs
        if (DEBUG)
            printf("Checking for Data Forwarding\n");
        data_forwarding(dest, reg->data[0]);
    }

    if (DEBUG)
        printf("WRITEBACK: ending\n");
}

void pipe_stage_mem()
{
    if (DEBUG) printf ("MEM: beginning\n");

    if (*stall_count > 0)
    {
        if (DEBUG)
            printf("MEM: stalling for %d instruction(s)\n", *stall_count);
        return;
    }

    if (!pipeline_queue_is_empty(QUEUE_MEM_TO_WB))
    {
        if (DEBUG)
            printf("MEM: MEM_TO_WB pipeline occupied; stalling\n");
        return;
    }

    Pipeline_Register *reg = pipeline_queue_pop(QUEUE_EX_TO_MEM);
    if (reg == NULL)
    {
        if (DEBUG) printf("MEM: found null register, so skipping\n");
        return;
    }

    if (reg->noop)
    {
        if (DEBUG) printf("MEM: noop\n");
        pipeline_queue_push(QUEUE_MEM_TO_WB, reg);
        return;
    }

    uint32_t instruction = reg->instruction;
    uint32_t opcode = get_opcode(instruction);

    char *name = NULL;

    if (instruction == 0xd69f03e0) // ERET
    {
        if (DEBUG)
        {
            printf("ERET at memory stage\n");
        }
        stall(1); // every preceding instruction must stall until ERET leaves pipeline
    }
    else if (opcode == 0x7C2)
    {
        name = "LDUR";
        uint64_t addr = reg->data[0];

        uint64_t first_32 = data_cache_read(reg, addr);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }

        uint64_t last_32 = data_cache_read(reg, addr + 4);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }

        uint64_t result = (last_32 << 32) | first_32;
        reg->data[0] = result;
        uint64_t dest = get_fragment_of(instruction, 0, 4);
        data_forwarding_mem(dest);
    }
    else if (opcode == 0x1C2)
    {
        name = "LDURB";
        uint64_t addr = reg->data[0];
        uint64_t result = data_cache_read(reg, addr);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }

        result = result & 0xFF;
        reg->data[0] = result;
        uint64_t dest = get_fragment_of(instruction, 0, 4);
        data_forwarding_mem(dest);
    }
    else if (opcode == 0x3C2)
    {
        name = "LDURH";
        uint64_t addr = reg->data[0];
        uint64_t result = data_cache_read(reg, addr);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }

        result = result & 0xFFFF;
        reg->data[0] = result;
        uint64_t dest = get_fragment_of(instruction, 0, 4);
        data_forwarding_mem(dest);
    }
    else if (opcode == 0x7C0)
    {
        name = "STUR";
        uint64_t addr = reg->data[0];
        uint64_t fragment = reg->data[1];
        reg->data[1] = fragment;

        uint32_t first_32 = fragment & 0xFFFFFFFF;
        uint32_t last_32 = (fragment >> 32) & 0xFFFFFFFF;

        data_cache_write(reg, addr, first_32);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }

        data_cache_write(reg, addr + 4, last_32);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }
    }
    else if (opcode == 0x1C0)
    {
        name = "STURB";
        uint64_t addr = reg->data[0];
        uint64_t fragment = reg->data[1];

        // Read the relevant 32 bits first
        uint32_t current_data = data_cache_read(reg, addr);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }

        // Zero out 8 bits of current_data and write in the fragment
        uint64_t new_data = (current_data & (~0xFF)) | (fragment & 0xFF);
        // Write the actual new data
        data_cache_write(reg, addr, new_data);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }
    }
    else if (opcode == 0x3C0)
    {
        name = "STURH";
        uint64_t addr = reg->data[0];
        uint64_t fragment = reg->data[1];

        // Read the relevant 32 bits first
        uint32_t current_data = data_cache_read(reg, addr);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }

        // Zero out 16 bits of current_data and write in the fragment
        uint64_t new_data = (current_data & (~0xFFFF)) | (fragment & 0xFFFF);
        // Write the actual new data
        data_cache_write(reg, addr, new_data);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }
    }
    else if (opcode == 0x5C0)
    {
        name = "STURW";
        uint64_t addr = reg->data[0];
        uint32_t fragment = reg->data[1] & 0xFFFFFFFF;

        // Write the actual new data
        data_cache_write(reg, addr, fragment);

        if (reg->stall)
        {
            reg->stall = 0;
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
            return;
        }
    }
    else if (opcode == 0x6a2)
    {
        if (DEBUG)
        {
            printf("HALTING\n");
        }
        *RUN_BIT = 0;
        // Flush Stage 1 and 2 queues
        QUEUE_IF->head = NULL;
        QUEUE_IF->tail = NULL;
        QUEUE_IF_TO_ID->head = NULL;
        QUEUE_IF_TO_ID->tail = NULL;
        QUEUE_ID_TO_EX->head = NULL;
        QUEUE_ID_TO_EX->tail = NULL;
        stall(1);
        return;
    }

    if (DEBUG)
    {
        if (name == NULL)
        {
            printf("MEM: did nothing\n");
        }
        else
        {
            printf("MEM: fetched memory operand for %s\n", name);
        }
        if (opcode == 0x6a2)
        {
            printf("MEM: processing HLT (shouldn't happen)\n");
        }
    }


    // For every instruction except for store & branching & ERET, we want to
    // forward the result to destination register.
    // (Branching is already handled by checking the noop flag.)
    if (!(opcode == 0x7C0 || opcode == 0x1C0 || opcode == 0x3C0 || opcode == 0x5C0 || instruction == 0xd69f03e0))
    {
        if (DEBUG)
            printf("Checking for Data Forwarding\n");
        uint64_t des = get_fragment_of(instruction, 0, 4);
        data_forwarding(des, reg->data[0]);
    }

    pipeline_queue_push(QUEUE_MEM_TO_WB, reg);

    if (DEBUG) printf("MEM: finished\n");
}

void pipe_stage_execute()
{
    if (DEBUG) printf("EXECUTE: beginning\n");

    if (*stall_count > 0)
    {
        if (DEBUG)
            printf("EXECUTE: stalling for %d instruction(s)\n", *stall_count);
        return;
    }

    if (!pipeline_queue_is_empty(QUEUE_EX_TO_MEM))
    {
        if (DEBUG)
            printf("EXECUTE: EX_TO_MEM pipeline occupied; stalling\n");
        return;
    }

    Pipeline_Register *reg = pipeline_queue_pop(QUEUE_ID_TO_EX);

    if (reg == NULL)
    {
        if (DEBUG) printf("EXECUTE: found null register, so skipping\n");
        return;
    }

    uint32_t instruction = reg->instruction;
    uint32_t opcode = get_opcode(instruction);
    uint32_t shamt = get_shamt(instruction); // only relevant for processing R format

    if (instruction == 0xd69f03e0) // ERET
    {
        if (DEBUG)
        {
            printf("process_ERET\n");
        }
        *exec_FLAG_N = 0;
        *exec_FLAG_Z = 0;
        *exec_FLAG_V = 0;
        *exec_FLAG_C = 0;

        stall(1); // every preceding instruction must stall until ERET leaves pipeline
    }
    else if (opcode == 0x458)
        process_ADD(reg);
    else if (0x488 <= opcode && opcode <= 0x489)
        process_ADDI(reg);
    else if (0x588 <= opcode && opcode <= 0x589)
        process_ADDIS(reg);
    else if (opcode == 0x558)
        process_ADDS(reg);
    else if (opcode == 0x450)
        process_AND(reg);
    else if (opcode == 0x750)
        process_ANDS(reg);
    else if (0x0A0 <= opcode && opcode <= 0x0BF)
        process_B(reg);
    else if (0x2A0 <= opcode && opcode <= 0x2A7) // Bcond
    {
        uint32_t cond = get_fragment_of(instruction, 0, 3);
        if (cond == 0)
            process_BEQ(reg);
        else if (cond == 1)
            process_BNE(reg);
        else if (cond == 10)
            process_BGE(reg);
        else if (cond == 11)
            process_BLT(reg);
        else if (cond == 12)
            process_BGT(reg);
        else if (cond == 13)
            process_BLE(reg);
        else
            printf("Processing B.cond - we have encountered an unmatched conditional code.\n");
    }
    else if (0x4A0 <= opcode && opcode <= 0x4BF)
        process_BL(reg);
    else if (opcode == 0x6B0)
        process_BR(reg);
    else if (opcode == 0x650)
        process_EOR(reg);
    else if (opcode == 0x550)
        process_ORR(reg);
    else if (0x5A8 <= opcode && opcode <= 0x5AF)
        process_CBNZ(reg);
    else if (0x5A0 <= opcode && opcode <= 0x5A7)
        process_CBZ(reg);
    else if (opcode == 0x4D8 && shamt == 0x1F)
        process_MUL(reg);
    else if (opcode == 0x4D6 && shamt == 0x03)
        process_SDIV(reg);
    else if (opcode == 0x4D6 && shamt == 0x02)
        process_UDIV(reg);
    else if (opcode == 0x6a2)
        process_HLT(reg);
    else if (0x69A <= opcode && opcode <= 0x69B)
    {
        uint32_t subtype = get_fragment_of(instruction, 10, 15);
        if (subtype == 0x3F)
            process_LSR_immediate(reg);
        else
            process_LSL_immediate(reg);
    }
    else if (opcode == 0x4D6)
    {
        uint32_t op2 = get_fragment_of(instruction, 10, 11);
        if (op2 == 1)
            process_LSR_register(reg);
        else if (op2 == 0)
            process_LSL_register(reg);
    }
    else if (0x694 <= opcode && opcode <= 0x697)
        process_MOVZ(reg);
    else if (opcode == 0x7C0)
        process_STUR(reg);
    else if (opcode == 0x1C0)
        process_STURB(reg);
    else if (opcode == 0x3C0)
        process_STURH(reg);
    else if (opcode == 0x5C0)
        process_STURW(reg);
    else if (opcode == 0x658)
        process_SUB(reg);
    else if (0x688 <= opcode && opcode <= 0x689)
        process_SUBI(reg);
    else if (0x788 <= opcode && opcode <= 0x789)
        process_SUBIS(reg);
    else if (opcode == 0x758) // cataches CMP as well
        process_SUBS(reg);
    else if (opcode == 0x7C2)
        process_LDUR(reg);
    else if (opcode == 0x1C2)
        process_LDURB(reg);
    else if (opcode == 0x3C2)
        process_LDURH(reg);
    else
    {
        if (DEBUG)
            printf("ERROR on opcode %x\n", opcode);
        process_HLT(reg);
    }

    // Set flags of the current instruction
    reg->FLAG_V = *exec_FLAG_V;
    reg->FLAG_N = *exec_FLAG_N;
    reg->FLAG_Z = *exec_FLAG_Z;
    reg->FLAG_C = *exec_FLAG_C;

    if (reg->stall)
    {
        // Push back to current queue
        reg->stall = 0;
        pipeline_queue_push(QUEUE_ID_TO_EX, reg);
    }
    else
    {
        if (pipeline_queue_is_empty(QUEUE_EX_TO_MEM))
        {
            pipeline_queue_push(QUEUE_EX_TO_MEM, reg);
        }
        else
        {
            pipeline_queue_push(QUEUE_ID_TO_EX, reg);
        }
    }

    if (DEBUG) printf("EXECUTE: returning\n");
}

void pipe_stage_decode()
{
    if (DEBUG) printf("DECODE: beginning\n");

    if (*stall_count > 0)
    {
        if (DEBUG)
            printf("DECODE: stalling for %d instruction(s)\n", *stall_count);
        return;
    }

    if (!pipeline_queue_is_empty(QUEUE_ID_TO_EX))
    {
        if (DEBUG)
            printf("DECODE: ID_TO_EX pipeline occupied; stalling\n");
        return;
    }

    Pipeline_Register *reg = pipeline_queue_pop(QUEUE_IF_TO_ID);
    if (reg == NULL)
    {
        if (DEBUG)
            printf("DECODE: found null register, so skipping\n");
        return;
    }
    uint32_t instruction = reg->instruction;
    uint32_t opcode = get_opcode(instruction);
    uint32_t shamt = get_shamt(instruction); // only relevant for processing R format

    char *name;

    Pipeline_Register *new_reg;

    if (instruction == 0xd69f03e0)
    {
        name = "ERET";
        *ERET_SERIALIZING = 1;

        // Check that every stage in front of us is empty
        if (
            !pipeline_queue_is_empty(QUEUE_ID_TO_EX) ||
            !pipeline_queue_is_empty(QUEUE_EX_TO_MEM) ||
            !pipeline_queue_is_empty(QUEUE_MEM_TO_WB)
        )
        {
            if (DEBUG)
            {
                printf("DECODE: stalling because ERET is waiting for pipeline to clear\n");
            }
            pipeline_queue_push(QUEUE_IF_TO_ID, reg);
            return;
        }

        new_reg = pipeline_register_new(0, 0, 0, 0);

        // should we stall here
        // if (instr_cache_stall_count())
        //     stall(1);
    }
    else if (opcode == 0x458)
    {
        name = "ADD";
        new_reg = get_R_data(instruction);
    }
    else if (0x488 <= opcode && opcode <= 0x489)
    {
        name = "ADDI";
        new_reg = get_I_data(instruction);
    }
    else if (0x588 <= opcode && opcode <= 0x589)
    {
        name = "ADDIS";
        new_reg = get_I_data(instruction);
    }
    else if (opcode == 0x558)
    {
        name = "ADDS";
        new_reg = get_R_data(instruction);
    }
    else if (opcode == 0x450)
    {
        name = "AND";
        new_reg = get_R_data(instruction);
    }
    else if (opcode == 0x750)
    {
        name = "ANDS";
        new_reg = get_R_data(instruction);
    }
    else if (0x0A0 <= opcode && opcode <= 0x0BF)
    {
        name = "B";
        new_reg = get_B_data(instruction);
    }
    else if (0x2A0 <= opcode && opcode <= 0x2A7) // Bcond
    {
        uint32_t cond = get_fragment_of(instruction, 0, 3);
        new_reg = get_CB_data(instruction);
        if (cond == 0)
        {
            name = "BEQ";
        }
        else if (cond == 1)
        {
            name = "BNE";
        }
        else if (cond == 10)
        {
            name = "BGE";
        }
        else if (cond == 11)
        {
            name = "BLT";
        }
        else if (cond == 12)
        {
            name = "BGT";
        }
        else if (cond == 13)
        {
            name = "BLE";
        }
        else
            printf("Processing B.cond - we have encountered an unmatched conditional code.\n");
    }
    else if (0x4A0 <= opcode && opcode <= 0x4BF)
    {
        name = "BL";
        new_reg = get_B_data(instruction);
    }
    else if (opcode == 0x6B0)
    {
        name = "BR";
        new_reg = get_R_data(instruction);
    }
    else if (opcode == 0x650)
    {
        name = "EOR";
        new_reg = get_R_data(instruction);
    }
    else if (opcode == 0x550)
    {
        name = "ORR";
        new_reg = get_R_data(instruction);
    }
    else if (0x5A8 <= opcode && opcode <= 0x5AF)
    {
        name = "CBNZ";
        new_reg = get_CB_data(instruction);
    }
    else if (0x5A0 <= opcode && opcode <= 0x5A7)
    {
        name = "CBZ";
        new_reg = get_CB_data(instruction);
    }
    else if (opcode == 0x4D8 && shamt == 0x1F)
    {
        name = "MUL";
        new_reg = get_R_data(instruction);
    }
    else if (opcode == 0x4D6 && shamt == 0x03)
    {
        name = "SDIV";
        new_reg = get_R_data(instruction);
    }
    else if (opcode == 0x4D6 && shamt == 0x02)
    {
        name = "UDIV";
        new_reg = get_R_data(instruction);
    }
    else if (opcode == 0x6a2)
    {
        name = "HLT";
        new_reg = pipeline_register_new(0, 0, 0, 0);
    }
    else if (0x69A <= opcode && opcode <= 0x69B)
    {
        uint32_t subtype = get_fragment_of(instruction, 10, 15);

        // uimm is from bits 16 to 21
        uint64_t alu_immediate = get_fragment_of(instruction, 16, 21);
        uint64_t Rn = CURRENT_STATE->REGS[get_fragment_of(instruction, 5, 9)];
        uint64_t Rd = CURRENT_STATE->REGS[get_fragment_of(instruction, 0, 4)];
        new_reg = pipeline_register_new(alu_immediate, Rn, Rd, 0);

        if (subtype == 0x3F)
        {
            name = "LSR (imm)";
        }
        else
        {
            name = "LSL (imm)";
        }
    }
    else if (opcode == 0x4D6)
    {
        uint32_t op2 = get_fragment_of(instruction, 10, 11);
        new_reg = get_R_data(instruction);

        if (op2 == 1)
        {
            name = "LSR (reg)";
        }
        else if (op2 == 0)
        {
            name = "LSL (reg)";
        }
    }
    else if (0x694 <= opcode && opcode <= 0x697)
    {
        name = "MOVZ";
        new_reg = get_IW_data(instruction);
    }
    else if (opcode == 0x7C0)
    {
        name = "STUR";
        new_reg = get_D_data(instruction);
    }
    else if (opcode == 0x1C0)
    {
        name = "STURB";
        new_reg = get_D_data(instruction);
    }
    else if (opcode == 0x3C0)
    {
        name = "STURH";
        new_reg = get_D_data(instruction);
    }
    else if (opcode == 0x5C0)
    {
        name = "STURW";
        new_reg = get_D_data(instruction);
    }
    else if (opcode == 0x658)
    {
        name = "SUB";
        new_reg = get_R_data(instruction);
    }
    else if (0x688 <= opcode && opcode <= 0x689)
    {
        name = "SUBI";
        new_reg = get_I_data(instruction);
    }
    else if (0x788 <= opcode && opcode <= 0x789)
    {
        name = "SUBIS";
        new_reg = get_I_data(instruction);
    }
    else if (opcode == 0x758)
    {
        name = "SUBS";
        new_reg = get_R_data(instruction);
        if (DEBUG)
        {
            printf("SUBS: destination register %llu\n", (long long unsigned)new_reg->data[3]);
        }
    }
    else if (opcode == 0x7C2)
    {
        name = "LDUR";
        new_reg = get_D_data(instruction);
    }
    else if (opcode == 0x1C2)
    {
        name = "LDURB";
        new_reg = get_D_data(instruction);
    }
    else if (opcode == 0x3C2)
    {
        name = "LDURH";
        new_reg = get_D_data(instruction);
    }
    else
    {
        if (DEBUG)
            printf("ERROR on opcode %x\n", opcode);
        return;
    }
    if (DEBUG)
    {
        printf("Decoded %s\n", name);
    }

    new_reg->instruction = instruction;
    new_reg->pc = reg->pc;
    new_reg->noop = reg->noop;
    new_reg->predicted_target = reg->predicted_target;
    new_reg->predicted_taken = reg->predicted_taken;
    pipeline_queue_push(QUEUE_ID_TO_EX, new_reg);
    pipeline_register_free(reg);
}

void pipe_stage_fetch()
{
    if (!pipeline_queue_is_empty(QUEUE_IF_TO_ID))
    {
        if (DEBUG)
            printf("FETCH: IF_TO_ID registers occupied; stalling\n");
        return;
    }

    if (DEBUG) printf("FETCH: beginning...\n");

    Pipeline_Register *reg = pipeline_queue_pop(QUEUE_IF);

    if (reg == NULL)
    {
        reg = pipeline_register_new(0, 0, 0, 0);
    }

    // If we are stalling due to branching, push back into queue for later
    if (*stall_count > 0)
    {
        pipeline_queue_push(QUEUE_IF, reg);
        if (DEBUG)
            printf("FETCH: stalling for %d instruction(s)\n", *stall_count);


        // Was going to be commented out
        if (instr_cache_stall_count() || (*ERET_SERIALIZING))
        {
            //if (instr_cache_stall_count()) {
            uint32_t instruction = instr_cache_read(reg, CURRENT_STATE->PC);
            if (reg->stall)
            {
                reg->stall = 0;
                return;
            }
        }
        return;
    }


    // Experiment
    // Check if ERET is in WB

    // Pipeline_Register *p_reg = pipeline_queue_pop(QUEUE_MEM_TO_WB);
    // uint32_t pinstruction = p_reg->instruction;
    // pipeline_queue_push(QUEUE_MEM_TO_WB, p_reg);
    //if (pinstruction == 0)
    //    pinstruction = 0;
    // if ((*ERET_SERIALIZING) && (pinstruction == 0xd69f03e0)) {
    if (*ERET_SERIALIZING)
    {
        pipeline_queue_push(QUEUE_IF, reg);
        //stall(1);
        return;
    }
    uint32_t instruction = instr_cache_read(reg, CURRENT_STATE->PC);

    // If we got a cache miss, we need to stall
    if (reg->stall)
    {
        reg->stall = 0;
        pipeline_queue_push(QUEUE_IF, reg);
        return;
    }

    reg->instruction = instruction;
    reg->pc = CURRENT_STATE->PC;
    bp_predict(reg);
    pipeline_queue_push(QUEUE_IF_TO_ID, reg);
    if (DEBUG) printf("FETCH: incremented PC to %x\n", (uint32_t)CURRENT_STATE->PC);
    if (DEBUG) printf("FETCH: returning\n");
}
