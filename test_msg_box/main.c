
#if (_MSC_VER)
    #define _CRTDBG_MAP_ALLOC
#endif

#include <stdlib.h>
#include <stdio.h>
#include "msg_box.h"

#include <string.h>

#if (_MSC_VER)
    #include <crtdbg.h>
#endif

static int
_create_node(
    msg_info_t  *pMsg_info)
{
    msg_node_t      *pCur_node = 0;

    pCur_node = malloc(sizeof(msg_node_t));
    memset(pCur_node, 0x0, sizeof(msg_node_t));

    pCur_node->type         = pMsg_info->type;
    pCur_node->destroy_node = pMsg_info->destroy_node;
    pCur_node->handle_node  = pMsg_info->handle_node;
    if( pMsg_info->pMsg_arg )
        memcpy(&pCur_node->msg_arg, pMsg_info->pMsg_arg, sizeof(msg_arg_t));

    pMsg_info->pMsg_node = pCur_node;
    return 0;
}

static int
_del_node(
    msg_node_t  **ppMsg_node)
{
    free(*ppMsg_node);
    *ppMsg_node = 0;
    return 0;
}

static int
_hanlde_node(
    msg_node_t  *pMsg_node)
{
    printf(" msg arg = 0x%x\n", pMsg_node->msg_arg.reserved);
    return 0;
}

void main()
{
    msg_arg_t       msg_arg = {0};
    msg_info_t      msg_info = {0};
    msg_node_t      *pCur_msg_node = 0;

    msg_arg.reserved = 0xdddd;

    msg_info.create_node  = _create_node;
    msg_info.destroy_node = _del_node;
    msg_info.handle_node  = _hanlde_node;
    msg_info.pMsg_arg     = &msg_arg;
    msg_box_post_node(&msg_info);

    msg_box_fetch_node(&pCur_msg_node);

    if( pCur_msg_node->handle_node )
        pCur_msg_node->handle_node(pCur_msg_node);

    if( pCur_msg_node->destroy_node )
        pCur_msg_node->destroy_node(&pCur_msg_node);

#if (_MSC_VER)
    _CrtDumpMemoryLeaks();
#endif
    return;
}
