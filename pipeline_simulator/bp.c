/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2016
 * Jonathan Lee (jonathanlee) and Kevin Tse (kevintse)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bp.h"
#include "pipe.h"

#define DEBUG 1
#undef dbg
#if defined(DEBUG) && (DEBUG)
#define dbg(str, ...)               printf(str, ##__VA_ARGS__)
#else
#define dbg(str, ...)
#endif

/* CPU-level globals */
bp_t *bpt;

void bp_init(CPU *new_cpu)
{
    if (DEBUG)
    {
        printf("bp_init\n");
    }
    new_cpu->bpt = malloc(sizeof(bp_t));
    memset(new_cpu->bpt, 0, sizeof(bp_t));
}

uint64_t hash(uint64_t addr) // addr should be PC
{
    uint64_t fragment = get_fragment_of(addr, 2, 9);
    uint64_t index = (fragment ^ bpt->GHR) % 256;

    if (DEBUG)
    {
        printf(
            "hash: (0x%llx ^ 0x%llx) mod 256\n",
            (long long unsigned)fragment,
            (long long unsigned)bpt->GHR
        );
    }

    return index;
}


void bp_predict(Pipeline_Register *reg)
{
    uint64_t addr = reg->pc;
    /* Predict next PC */
    uint64_t index = get_fragment_of(addr, 2, 11);
    uint64_t prediction;
    reg->predicted_taken = 0;

    if (index == 0x7 && DEBUG)
    {
        printf(
            "bp_predict: bpt->BTB[0x7] -> {addr=%llx, target=%llx, conditional=%llx, valid=%d}\n",
            (long long unsigned)bpt->BTB[index].addr,
            (long long unsigned)bpt->BTB[index].target,
            (long long unsigned)bpt->BTB[index].conditional,
            bpt->BTB[index].valid
        );
    }

    if (bpt->BTB[index].valid == 0)
    {
        if (DEBUG)
        {
            printf("bpt->BTB[0x%llx] MISS: invalid entry\n", (long long unsigned)index);
        }
        prediction = (addr + 4);
    }
    else if (addr != bpt->BTB[index].addr)
    {
        // check if addr is wrong or valid
        if (DEBUG)
        {
            printf("bpt->BTB[0x%llx] MISS: wrong address\n", (long long unsigned)index);
        }
        prediction = (addr + 4);
    }
    else
    {
        if (bpt->PHT[hash(addr)] > 1) // check if gshare indicates taken
        {
            if (DEBUG)
            {
                printf("bpt->BTB[0x%llx] HIT: predicting taken\n", (long long unsigned)index);
            }
            prediction = bpt->BTB[index].target;
            reg->predicted_taken = 1;
        }
        else
        {
            if (DEBUG)
            {
                printf("bpt->BTB[0x%llx] HIT: predicting not taken\n", (long long unsigned)index);
            }
            prediction = (addr + 4);
        }
    }

    if (DEBUG)
    {
        printf(
            "bp_predict: predicted %s from 0x%llx to 0x%llx\n",
            reg->predicted_taken ? "taken" : "not taken",
            (long long unsigned) reg->pc,
            (long long unsigned) prediction
        );
    }

    CURRENT_STATE->PC = prediction;

    // Save the prediction data inside of register
    reg->predicted_target = prediction;
}

void bp_update(uint64_t addr, uint64_t target, uint64_t taken, uint64_t conditional, Pipeline_Register *reg)
{
    /* Update BTB */
    uint64_t btb_index = get_fragment_of(addr, 2, 11);
    bpt->BTB[btb_index].addr = addr;
    bpt->BTB[btb_index].target = target;
    bpt->BTB[btb_index].valid = 1;
    bpt->BTB[btb_index].conditional = conditional;

    if (DEBUG)
    {
        printf(
            "bpt->BTB[0x%llx] = {addr=0x%llx, target=%llx, conditional=%d}\n",
            (long long unsigned)btb_index,
            (long long unsigned)addr,
            (long long unsigned)target,
            (int)conditional
        );
    }

    // Unconditional branches do not update PHT or GHR
    if (conditional)
    {
        /* Update gshare directional predictor */
        uint64_t index = hash(addr);
        uint64_t initial_dir = bpt->PHT[index];

        if (taken)   // branch was taken in exe
        {
            if (initial_dir < 3)
            {
                bpt->PHT[index] = initial_dir + 1;
            }
        }
        else     // branch not taken in exe
        {
            if (initial_dir > 0)
            {
                bpt->PHT[index] = initial_dir - 1;
            }
        }

        if (DEBUG)
        {
            printf(
                "bp_update: after %s, PHT[%d] = %d -> %d\n",
                taken ? "taken" : "not taken",
                (int)index,
                (int)initial_dir,
                (int)bpt->PHT[index]
            );
        }

        /* Update global history register */
        bpt->GHR = (bpt->GHR << 1) & 0xFF; // shift by 1 and mask

        if (taken)   // branch is taken
        {
            bpt->GHR = bpt->GHR + 1;
        } // else leave last bit as 0
    }

    // Check if we need to flush the pipeline
    if (reg->predicted_taken != taken || target != reg->predicted_target)
    {
        if (DEBUG)
        {
            if (reg->predicted_taken != taken)
            {
                printf(
                    "bp_update: flushing because predicted %d != actual %d\n",
                    (int)reg->predicted_taken,
                    (int)taken
                );
            }
            else if (target != reg->predicted_target)
            {
                printf(
                    "bp_update: flushing because predicted target 0x%llx != actual target 0x%llx\n",
                    (long long unsigned)reg->predicted_target,
                    (long long unsigned)target
                );
            }
        }

        if (taken && !reg->predicted_taken)
        {
            flush_IF_TO_ID();
            CURRENT_STATE->PC = target;
        }
        else if (!taken && reg->predicted_taken)
        {
            flush_IF_TO_ID();
            CURRENT_STATE->PC = reg->pc + 4;
        }
    }
    else
    {
        if (DEBUG)
        {
            printf("bp_update: we predicted correctly\n");
        }
    }
}
