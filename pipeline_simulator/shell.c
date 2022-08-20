/***************************************************************/
/*                                                             */
/*   ARM Instruction Level Simulator                           */
/*                                                             */
/*   CMSC-22200 Computer Architecture                          */
/*   University of Chicago                                     */
/*                                                             */
/***************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*          DO NOT MODIFY THIS FILE!                            */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "shell.h"
#include "pipe.h"
#include "bp.h"
#include "cache.h"

#define DEBUG 1

/***************************************************************/
/* Statistics.                                                 */
/***************************************************************/

uint32_t stat_cycles = 0, stat_inst_retire = 0, stat_inst_fetch = 0;
uint32_t stat_squash = 0;

/* === Actual globals === */
int NUM_CPUS = CONIFG_CPU_NUM;
CPU **CPUS;

/* === CPU-level globals */
int *ERET_SERIALIZING;
int *RUN_BIT;
int CURR_CPU_NUM;

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/

#define MEM_DATA_START  0x10000000
#define MEM_DATA_SIZE   0x00100000
#define MEM_TEXT_START  0x00400000
#define MEM_TEXT_SIZE   0x00100000
#define MEM_STACK_START 0xfffffffc
#define MEM_STACK_SIZE  0x00100000

typedef struct
{
    uint64_t start, size;
    uint8_t *mem;
} mem_region_t;

/* memory will be dynamically allocated at initialization */
mem_region_t MEM_REGIONS[] =
{
    { MEM_TEXT_START, MEM_TEXT_SIZE, NULL },
    { MEM_DATA_START, MEM_DATA_SIZE, NULL },
    { MEM_STACK_START, MEM_STACK_SIZE, NULL },
};
#define MEM_NREGIONS (sizeof(MEM_REGIONS)/sizeof(mem_region_t))



/***************************************************************/
/*                                                             */
/* Procedure: mem_read_32                                      */
/*                                                             */
/* Purpose: Read a 32-bit word from memory                     */
/*                                                             */
/***************************************************************/
uint32_t mem_read_32(uint64_t address)
{
    int i;
    for (i = 0; i < MEM_NREGIONS; i++)
    {
        if (address >= MEM_REGIONS[i].start &&
                address < (MEM_REGIONS[i].start + MEM_REGIONS[i].size))
        {
            uint32_t offset = address - MEM_REGIONS[i].start;

            return
                (MEM_REGIONS[i].mem[offset + 3] << 24) |
                (MEM_REGIONS[i].mem[offset + 2] << 16) |
                (MEM_REGIONS[i].mem[offset + 1] <<  8) |
                (MEM_REGIONS[i].mem[offset + 0] <<  0);
        }
    }

    return 0;
}

/***************************************************************/
/*                                                             */
/* Procedure: mem_write_32                                     */
/*                                                             */
/* Purpose: Write a 32-bit word to memory                      */
/*                                                             */
/***************************************************************/
void mem_write_32(uint64_t address, uint32_t value)
{
    int i;
    for (i = 0; i < MEM_NREGIONS; i++)
    {
        if (address >= MEM_REGIONS[i].start &&
                address < (MEM_REGIONS[i].start + MEM_REGIONS[i].size))
        {
            uint32_t offset = address - MEM_REGIONS[i].start;

            MEM_REGIONS[i].mem[offset + 3] = (value >> 24) & 0xFF;
            MEM_REGIONS[i].mem[offset + 2] = (value >> 16) & 0xFF;
            MEM_REGIONS[i].mem[offset + 1] = (value >>  8) & 0xFF;
            MEM_REGIONS[i].mem[offset + 0] = (value >>  0) & 0xFF;
            return;
        }
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help()
{
    printf("----------------ARM ISIM Help-----------------------\n");
    printf("go                     -  run program to completion         \n");
    printf("run n                  -  execute program for n instructions\n");
    printf("rdump                  -  dump architectural registers      \n");
    printf("mdump low high         -  dump memory from low to high      \n");
    printf("input reg_no reg_value - set GPR reg_no to reg_value        \n");
    printf("?                      -  display this help menu            \n");
    printf("quit                   -  exit the program                  \n\n");
    return;
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle()
{
    // Change CPU and data
    int i;
    int running[NUM_CPUS];
    for (i = 0; i < NUM_CPUS; i++)
    {
        running[i] = *(CPUS[i]->RUN_BIT);
    }

    for (i = 0; i < NUM_CPUS; i++)
    {
        /* Set all of the CPU-level global pointers */
        CPU *cpu = CPUS[i];

        if (!running[i])
        {
            continue;
        }

        if (DEBUG)
        {
            printf("-*-*- cycling cpu #%d -*-*-\n", i);
        }

        CURR_CPU_NUM = i;

        CURRENT_STATE = cpu->CURRENT_STATE;

        QUEUE_IF = cpu->QUEUE_IF;
        QUEUE_IF_TO_ID = cpu->QUEUE_IF_TO_ID;
        QUEUE_ID_TO_EX = cpu->QUEUE_ID_TO_EX;
        QUEUE_EX_TO_MEM = cpu->QUEUE_EX_TO_MEM;
        QUEUE_MEM_TO_WB = cpu->QUEUE_MEM_TO_WB;

        ERET_SERIALIZING = cpu->ERET_SERIALIZING;

        RUN_BIT = cpu->RUN_BIT;
        stall_count = cpu->stall_count;

        exec_FLAG_N = cpu->exec_FLAG_N;
        exec_FLAG_Z = cpu->exec_FLAG_Z;
        exec_FLAG_V = cpu->exec_FLAG_V;
        exec_FLAG_C = cpu->exec_FLAG_C;

        bpt = cpu->bpt;

        instr_cache = cpu->instr_cache;
        data_cache = cpu->data_cache;

        pipe_cycle();
    }



    stat_cycles++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the ARM for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles)
{
    int i;

    if (!(*(CPUS[0]->RUN_BIT)) && !(*(CPUS[1]->RUN_BIT)) &&
        !(*(CPUS[2]->RUN_BIT)) && !(*(CPUS[3]->RUN_BIT)))
    {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++)
    {
        if (!(*(CPUS[0]->RUN_BIT)) && !(*(CPUS[1]->RUN_BIT)) &&
            !(*(CPUS[2]->RUN_BIT)) && !(*(CPUS[3]->RUN_BIT)))
        {
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the ARM until HALTed                 */
/*                                                             */
/***************************************************************/
void go()
{
    if (!(*(CPUS[0]->RUN_BIT)) && !(*(CPUS[1]->RUN_BIT)) &&
        !(*(CPUS[2]->RUN_BIT)) && !(*(CPUS[3]->RUN_BIT)))
    {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating...\n\n");
    while ((*(CPUS[0]->RUN_BIT) || *(CPUS[1]->RUN_BIT)) ||
           (*(CPUS[2]->RUN_BIT) || *(CPUS[3]->RUN_BIT)))
    {
        cycle();
    }
    printf("Simulator halted\n\n");
}
/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE *dumpsim_file, int start, int stop)
{
    int address;

    printf("\nMemory content [0x%08x..0x%08x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = start; address <= stop; address += 4)
        printf("  0x%08x (%d) : 0x%x\n", address, address, mem_read_32(address));
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%08x..0x%08x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = start; address <= stop; address += 4)
        fprintf(dumpsim_file, "  0x%08x (%d) : 0x%x\n", address, address, mem_read_32(address));
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE *dumpsim_file)
{
    int j, k;

    // printf("\nCurrent register/bus values :\n");
    // printf("-------------------------------------\n");
    // printf("Instruction Retired : %u\n", stat_inst_retire);
    // printf("PC                : 0x%" PRIx64 "\n", CURRENT_STATE->PC);
    // printf("Registers:\n");

    for (j = 0; j < NUM_CPUS; j++)
    {
        if ((*CPUS[j]->RUN_BIT) == 1)
            printf("CPU %d:\n", j);
        else
            printf("CPU %d:\n", j);
        for (k = 0; k < 31; k++)
            printf("X%d: 0x%" PRIx64 "\n", k, CPUS[j]->CURRENT_STATE->REGS[k]);
        printf("FLAG_N: %d\n", CPUS[j]->CURRENT_STATE->FLAG_N);
        printf("FLAG_Z: %d\n", CPUS[j]->CURRENT_STATE->FLAG_Z);
        // printf("FLAG_V: %d\n", CPUS[j]->CURRENT_STATE->FLAG_V);
        // printf("FLAG_C: %d\n", CPUS[j]->CURRENT_STATE->FLAG_C);
    }

    printf("No. of Cycles: %d\n", stat_cycles);
    printf("\n");

    /* dump the state information into the dumpsim file */
    // fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    // fprintf(dumpsim_file, "-------------------------------------\n");
    // fprintf(dumpsim_file, "Instruction Retired : %u\n", stat_inst_retire);
    // fprintf(dumpsim_file, "PC                : 0x%" PRIx64 "\n", CURRENT_STATE->PC);
    // fprintf(dumpsim_file, "Registers:\n");

    for (j = 0; j < NUM_CPUS; j++)
    {
        if ((*CPUS[j]->RUN_BIT) == 1)
            fprintf(dumpsim_file, "CPU %d:\n", j);
        else
            fprintf(dumpsim_file, "CPU %d:\n", j);
        for (k = 0; k < 31; k++)
            fprintf(dumpsim_file, "X%d: 0x%" PRIx64 "\n", k, CPUS[j]->CURRENT_STATE->REGS[k]);
        fprintf(dumpsim_file, "FLAG_N: %d\n", CPUS[j]->CURRENT_STATE->FLAG_N);
        fprintf(dumpsim_file, "FLAG_Z: %d\n", CPUS[j]->CURRENT_STATE->FLAG_Z);
        // fprintf(dumpsim_file, "FLAG_V: %d\n", CPUS[j]->CURRENT_STATE->FLAG_V);
        // fprintf(dumpsim_file, "FLAG_C: %d\n", CPUS[j]->CURRENT_STATE->FLAG_C);
    }

    fprintf(dumpsim_file, "No. of Cycles: %d\n", stat_cycles);
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE *dumpsim_file)
{
    char buffer[20];
    int start, stop, cycles;
    int register_no;
    int64_t register_value;

    printf("ARM-SIM> ");

    if (scanf("%s", buffer) == EOF)
        exit(0);

    printf("\n");

    switch(buffer[0])
    {
        case 'G':
        case 'g':
            go();
            break;

        case 'M':
        case 'm':
            printf("Set (stat-addr stop-addr) = ");
            if (scanf("%i %i", &start, &stop) != 2)
                break;

            mdump(dumpsim_file, start, stop);
            break;

        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);

        case 'R':
        case 'r':
            if (buffer[1] == 'd' || buffer[1] == 'D')
                rdump(dumpsim_file);
            else
            {
                printf("Set cycles = ");
                if (scanf("%d", &cycles) != 1)
                    break;

                run(cycles);
            }
            break;

        case 'I':
        case 'i':
            if (scanf("%i %" PRIx64, &register_no, &register_value) != 2)
                break;
            CPUS[0]->CURRENT_STATE->REGS[register_no] = register_value;
            break;

        default:
            printf("Invalid Command\n");
        case '?':
            help();
            break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Allocate and zero memory                        */
/*                                                             */
/***************************************************************/
void init_memory()
{
    int i;
    for (i = 0; i < MEM_NREGIONS; i++)
    {
        MEM_REGIONS[i].mem = malloc(MEM_REGIONS[i].size);
        memset(MEM_REGIONS[i].mem, 0, MEM_REGIONS[i].size);
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(CPU *cpu, char *program_filename)
{
    FILE *prog;
    int ii, word;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL)
    {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF)
    {
        mem_write_32(MEM_TEXT_START + ii, word);
        ii += 4;
    }

    cpu->CURRENT_STATE->PC = MEM_TEXT_START;

    printf("Read %d words from program into memory.\n\n", ii / 4);
}

CPU *cpu_new(uint64_t enter_pointer)
{
    CPU *new_cpu = malloc(sizeof(CPU));
    if (DEBUG)
    {
        printf("malloced new_cpu\n");
    }
    new_cpu->ERET_SERIALIZING = malloc(sizeof(int));
    *(new_cpu->ERET_SERIALIZING) = 0;
    new_cpu->RUN_BIT = malloc(sizeof(int));
    *(new_cpu->RUN_BIT) = 0;
    new_cpu->stall_count = malloc(sizeof(int));
    *(new_cpu->stall_count) = 0;

    new_cpu->exec_FLAG_N = malloc(sizeof(int));
    *(new_cpu->exec_FLAG_N) = 0;
    new_cpu->exec_FLAG_Z = malloc(sizeof(int));
    *(new_cpu->exec_FLAG_Z) = 0;
    new_cpu->exec_FLAG_V = malloc(sizeof(int));
    *(new_cpu->exec_FLAG_V) = 0;
    new_cpu->exec_FLAG_C = malloc(sizeof(int));
    *(new_cpu->exec_FLAG_C) = 0;

    new_cpu->CURRENT_STATE = malloc(sizeof(CPU_State));
    memset(new_cpu->CURRENT_STATE, 0, sizeof(CPU_State));
    if (DEBUG)
    {
        printf("finished memsetting CURRENT_STATE to 0\n");
    }
    new_cpu->CURRENT_STATE->PC = enter_pointer;
    if (DEBUG)
    {
        printf("finished setting CURRENT_STATE PC\n");
    }
    pipe_init(new_cpu);
    bp_init(new_cpu);
    cache_init(new_cpu);
    if (DEBUG)
    {
        printf("cpu_new returning\n");
    }
    return new_cpu;
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files)
{
    int i;

    init_memory();
    pipe_init();

    CPUS = malloc(NUM_CPUS * sizeof(CPU));

    // Initialize each CPU's data structures
    for ( i = 0; i < NUM_CPUS; i++ )
    {
        dbg("-*-*- initializing cpu #%d -*-*-\n", i);
        CPUS[i] = cpu_new(CONIFG_IMG_BASE_ADDR);
        CPUS[i]->CURR_CPU_NUM = i;

        dbg("initialized cpu #%d\n", i);
    }

    for ( i = 0; i < num_prog_files; i++ )
    {
        load_program(CPUS[0], program_filename);
        while(*program_filename++ != '\0');
    }

    dbg("setting run bit\n");

    *(CPUS[0]->RUN_BIT) = 1;

    dbg("set run bit\n");
    return;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[])
{
    FILE *dumpsim_file;

    /* Error Checking */
    if (argc < 2)
    {
        printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }

    printf("ARM Simulator\n\n");

    initialize(argv[1], argc - 1);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL )
    {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
    {
        get_command(dumpsim_file);
    }
}
