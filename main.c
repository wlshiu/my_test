#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "virtual_phy.h"
#include "log.h"
#include "rbi.h"

typedef struct buf
{
    uint32_t    *pBuf;
    uint32_t    buf_len;

} buf_t;

typedef struct node_args
{
    int         node_id;
    buf_t       buf_rx;
    buf_t       buf_tx;

    void        *pRoute_table;
} node_args_t;

typedef struct node_attr
{
    pthread_t   tid;

} node_attr_t;

static node_attr_t      *g_pNode_attr = 0;
static pthread_cond_t   g_usr_cond;
static pthread_mutex_t  g_usr_mtx;

static void
_phy_recv(
    vphy_dataq_t    *pDataq)
{
    int     rval = 0;
    do {
        node_args_t     *pNode_info = (node_args_t*)pDataq->pUser_data;
        vphy_package_t  *pPackage = 0;

        rval = rbi_pop(pDataq->dataq,
                       (uint8_t*)pNode_info->buf_rx.pBuf,
                       (int)pNode_info->buf_rx.buf_len);
        if( !rval )  break;

        pPackage = (vphy_package_t*)pNode_info->buf_rx.pBuf;

        log_out("tid= %08x: rx= %s\n", pPackage->src_tid, pPackage->data);

    } while(0);

    return;
}

static void*
_task_node(void *argv)
{
    int             rval = 0;
    int             node_id = *((int*)argv);
    node_args_t     node_info = {0};

    if( !(node_info.buf_rx.pBuf = malloc(CONFIG_VPHY_MSG_SIZE)) )
    {
        pthread_exit(0);
        return 0;
    }

    if( !(node_info.buf_tx.pBuf = malloc((CONFIG_VPHY_MSG_SIZE - 32))) )
    {
        pthread_exit(0);
        return 0;
    }

    pthread_mutex_lock(&g_usr_mtx);
    pthread_cond_signal(&g_usr_cond);
    pthread_mutex_unlock(&g_usr_mtx);

    node_info.node_id        = node_id;
    node_info.buf_rx.buf_len = CONFIG_VPHY_MSG_SIZE;
    node_info.buf_tx.buf_len = CONFIG_VPHY_MSG_SIZE - 32;

    vphy_register_event_callback(_phy_recv, &node_info);

    log_out("node id %d (tid= x%08x)\n", node_id, pthread_self());

    memset((void*)node_info.buf_tx.pBuf, 0x0, node_info.buf_tx.buf_len);
    snprintf((char*)node_info.buf_tx.pBuf, node_info.buf_tx.buf_len, "-src=%d", node_id);

    rval = vphy_send((uint8_t*)node_info.buf_tx.pBuf, strlen((char*)node_info.buf_tx.pBuf) + 1);
    if( rval )
    {
        log_out("=> node id %d (tid= x%08x) sent fail (%d)\n", node_id, pthread_self(), rval);
    }

    while(1)
    {
        __asm("nop");
    }

    if( node_info.buf_rx.pBuf )    free(node_info.buf_rx.pBuf);
    if( node_info.buf_tx.pBuf )    free(node_info.buf_tx.pBuf);

    pthread_exit(0);
    return 0;
}

static void
_usage(char *pProgram)
{
    printf("%s usage:\n"
           "    -n      node number\n"
           , pProgram);
    system("pause");
    exit(-1);
}

int main(int argc, char **argv)
{
    int     node_number = 0;

    /*
     * Read through command-line arguments for options.
     */
    for(int i = 1; i < argc; i++)
    {
        if( argv[i][0] == '-' )
        {
            if (argv[i][1] == 'n')
            {
                node_number = atoi(argv[i + 1]);
                i++;
            }
            else
            {
                printf("Invalid option.\n");
                _usage(argv[0]);
                return 2;
            }
        }
    }

    do {
        int     rval = 0;
        if( !(g_pNode_attr = malloc(sizeof(node_attr_t) * node_number)) )
        {
            break;
        }
        memset(g_pNode_attr, 0x0, sizeof(node_attr_t) * node_number);

        vphy_init();

        rval = pthread_mutex_init(&g_usr_mtx, 0);
        if( rval )   break;

        rval = pthread_cond_init(&g_usr_cond, 0);
        if( rval )   break;

        for(int i = 0; i < node_number; i++)
        {
            pthread_create(&g_pNode_attr[i].tid, 0, _task_node, &i);

            pthread_mutex_lock(&g_usr_mtx);
            pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
            pthread_mutex_unlock(&g_usr_mtx);
        }

        while(1)
            __asm("nop");

        for(int i = 0; i < node_number; i++)
        {
            pthread_join(g_pNode_attr[i].tid, 0);
        }

        vphy_deinit();

    } while(0);

    return 0;
}
