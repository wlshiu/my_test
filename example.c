/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/11/18
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>

#include "protothread.h"
#include "protothread_sem.h"
#include "protothread_lock.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
typedef struct create_context_s
{
    int             i;
    pt_thread_t     pt_thread;
    pt_func_t       pt_func;
} create_context_t ;

static pt_t
create_thr(env_t const env)
{
    create_context_t    *const c = env;
    pt_resume(c);

    return PT_DONE ;
}

static void
test_thread_create(void)
{
    protothread_t       const pt = protothread_create();
    create_context_t    *const c = malloc(sizeof(*c));

    for(int i = 0; i < 100; i++)
    {
        pt_create(pt, &c->pt_thread, create_thr, c);
        protothread_run(pt);
    }
    free(c);
    protothread_free(pt);
    return;
}

/********* yield ********/
typedef struct yield_context_s
{
    pt_func_t       pt_func;
    pt_thread_t     pt_thread;
    int             i;
} yield_context_t ;

static pt_t
yield_thr(env_t const env)
{
    yield_context_t     *const c = env;
    pt_resume(c);

    for (c->i = 0; c->i < 10; c->i++)
    {
        pt_yield(c);
    }
    return PT_DONE;
}

static void
test_yield(void)
{
    protothread_t       const pt = protothread_create();
    yield_context_t     *const c = malloc(sizeof(*c));

    c->i = -1;     /* invalid value */
    pt_create(pt, &c->pt_thread, yield_thr, c);

    /* it hasn't run yet at all, make it reach the yield */
    protothread_run(pt);

    for(int i = 0; i < 10; i++)
    {
        /* make sure the protothread advances its loop */
        assert(i == c->i);
        protothread_run(pt);
    }

    free(c);
    protothread_free(pt);
    return;
}

/***** producer and consumer threads *************/

typedef struct
{
    pt_thread_t     pt_thread;
    pt_func_t       pt_func;
    int             i;
    int             *mailbox;
} pc_thread_context_t;

static pt_t
producer_thr(void *const env)
{
    pc_thread_context_t     *const c = env;

    pt_resume(c);

    for(c->i = 1; c->i <= 100; c->i++)
    {
        while( *c->mailbox )
        {
            /* mailbox is full */
            pt_wait(c, c->mailbox);
        }

        *c->mailbox = c->i;
        printf("[%s:%d] index %d\n", __func__, __LINE__, c->i);
        pt_signal(pt_get_pt(c), c->mailbox);
    }
    return PT_DONE;
}

static pt_t
consumer_thr(void *const env)
{
    pc_thread_context_t     *const c = env;
    pt_resume(c);

    for (c->i = 1; c->i <= 100; c->i++)
    {
        while( *c->mailbox == 0 )
        {
            /* mailbox is empty */
            pt_wait(c, c->mailbox);
        }

        assert(*c->mailbox == c->i);

        printf("[%s:%d] index %d\n", __func__, __LINE__, c->i);

        *c->mailbox = 0;
        pt_signal(pt_get_pt(c), c->mailbox);
    }
    return PT_DONE ;
}

static void
test_producer_consumer(void)
{
    protothread_t           const pt = protothread_create();
    pc_thread_context_t     *const cc = malloc(sizeof(*cc));
    pc_thread_context_t     *const pc = malloc(sizeof(*pc));
    int                     mailbox = 0;

#if 1
    /* set up consumer context, start consumer thread */
    cc->mailbox = &mailbox;
    cc->i = 0;
    pt_create(pt, &cc->pt_thread, consumer_thr, cc);

    /* set up producer context, start producer thread */
    pc->mailbox = &mailbox;
    pc->i = 0;
    pt_create(pt, &pc->pt_thread, producer_thr, pc);
#else
    /* set up producer context, start producer thread */
    pc->mailbox = &mailbox;
    pc->i = 0;
    pt_create(pt, &pc->pt_thread, producer_thr, pc);

    /* set up consumer context, start consumer thread */
    cc->mailbox = &mailbox;
    cc->i = 0;
    pt_create(pt, &cc->pt_thread, consumer_thr, cc);
#endif

    /* while threads are available to run ... */
    while (protothread_run(pt))
    {
    }

    /* threads have completed */
    assert(cc->i == 101);
    assert(pc->i == 101);

    free(cc);
    free(pc);
    protothread_free(pt);
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    // test_thread_create();

    // test_yield();

    test_producer_consumer();
    return 0;
}
