#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libLinkList.h"

//=============================================================================
//				  Constant Definition
//=============================================================================
#define MMP_NULL            NULL
#define _heapMalloc(size)   malloc(size)
#define _heapFree(ptr)      free(ptr)



struct _LL_NODE;
typedef struct _LL_NODE
{
    struct _LL_NODE*   prev;
    struct _LL_NODE*   next;

    int               index;
    void*             nodeData;
}LL_NODE;

typedef struct _LL_MGR_TAG 
{
    LL_NODE     *startNode;
    LL_NODE     *currNode;
    
    int         nodeSize;
    int         nodeCount;

    MMP_BOOL    (*cbCreateNodeData)(void**     nodeInfo, 
                                    void*      inputData,
                                    int        inputDataSize);
    MMP_BOOL    (*cbDestroyNodeData)(void*);
}LL_MGR;

//=============================================================================
//				  Public Function Definition
//=============================================================================
HLINLLIST
LL_createNodes(
    int         oneNodeSize,
    MMP_BOOL    (*cbCreateNodeData)(void**  nodeInfo, 
                                    void*   inputData,
                                    int     inputDataSize),
    MMP_BOOL    (*cbDestroyNodeData)(void*))
{
    LL_MGR     *pLLMgr = _heapMalloc(sizeof(LL_MGR));

    if( pLLMgr == MMP_NULL )
    {
        return MMP_NULL;
    }
    else
    {
        memset(pLLMgr, 0, sizeof(LL_MGR));

        pLLMgr->nodeSize = oneNodeSize;
        pLLMgr->cbCreateNodeData = cbCreateNodeData;
        pLLMgr->cbDestroyNodeData = cbDestroyNodeData;
    }

    return (HLINLLIST)pLLMgr;
}

void 
LL_destroyNotes(
    HLINLLIST  hLinkList)
{
    LL_MGR     *pLLMgr = (LL_MGR*)hLinkList;

    if( pLLMgr )
    {
        LL_NODE     *currNode = pLLMgr->startNode;

        while (currNode)
        {
            LL_NODE     *tmpNode = currNode;
            
            if( currNode->nodeData )
            {
                if( pLLMgr->cbDestroyNodeData == MMP_NULL || 
                    pLLMgr->cbDestroyNodeData(currNode->nodeData) == MMP_FALSE )
                {
                    _heapFree(currNode->nodeData);
                }
                currNode->nodeData = MMP_NULL;
            }

            currNode->prev = MMP_NULL;
            currNode = currNode->next;
            _heapFree(tmpNode);
        }

        _heapFree(pLLMgr);
        pLLMgr = MMP_NULL;
    }
}

LINKLIST_ERR
LL_addNode(
    HLINLLIST  hLinkList,
    void*      inputData,
    int        inputDataSize)
{
    LINKLIST_ERR    result = LINKLIST_ERR_OK;
    LL_MGR          *pLLMgr = (LL_MGR*)hLinkList;
    LL_NODE         *pLLNode = MMP_NULL;

    if( pLLMgr == MMP_NULL || inputData == MMP_NULL )
    {
        printf("Null Pointer (hLinkList = 0x%x, inputData = 0x%x) !!\n", pLLMgr, inputData);
        return LINKLIST_ERR_NULL_POINTER;
    }
 
    if( inputDataSize != pLLMgr->nodeSize )
    {
        printf("Wrong input data size !! \n");
        return LINKLIST_ERR_ALLOCATE_FAIL;
    }

    pLLNode = (LL_NODE*)_heapMalloc(sizeof(LL_NODE));
    if( pLLNode )
    {
        pLLNode->index = -1;
        pLLNode->prev = pLLNode->next = pLLNode->nodeData = MMP_NULL;

        if( pLLMgr->cbCreateNodeData == MMP_NULL ||
            pLLMgr->cbCreateNodeData(&pLLNode->nodeData, inputData, inputDataSize) == MMP_FALSE )
        {
            pLLNode->nodeData = (void*)_heapMalloc(pLLMgr->nodeSize);
            if( pLLNode->nodeData == MMP_NULL )
            {
                printf("malloc fail !! %s [#%d]\n", __FILE__, __LINE__);
                _heapFree(pLLNode);
                result = LINKLIST_ERR_ALLOCATE_FAIL;
                goto end;
            }
            else
            {
                memset(pLLNode->nodeData, 0, pLLMgr->nodeSize);
                memcpy((void*)pLLNode->nodeData, (void*)inputData, inputDataSize);

                pLLNode->index = pLLMgr->nodeCount++;
                pLLNode->prev  = pLLMgr->currNode;
                pLLMgr->currNode = pLLNode;
            }
        }
        else
        {
            pLLNode->index = pLLMgr->nodeCount++;
            pLLNode->prev  = pLLMgr->currNode;
            pLLMgr->currNode = pLLNode;
        }
    }
    else
    {
        printf("malloc fail !! %s [#%d]\n", __FILE__, __LINE__);
        result = LINKLIST_ERR_ALLOCATE_FAIL;
        goto end;
    }

    if( pLLMgr->startNode == MMP_NULL )
    {
        // the first node
        pLLMgr->startNode = pLLMgr->currNode;
    }

end:
    return  result;
}

LINKLIST_ERR
LL_delNode(
    HLINLLIST  hLinkList,
    void*      comparedData, 
    MMP_BOOL (*cbCheckCondition)(void* comparedData,
                                 void* nodeData)) 
{ 
    LINKLIST_ERR    result = LINKLIST_ERR_OK;
    LL_MGR          *pLLMgr = (LL_MGR*)hLinkList;
    LL_NODE         *currNode = MMP_NULL;
    LL_NODE         *prevNode = MMP_NULL;
    LL_NODE         *nextNode = MMP_NULL;
    int             index = 0;

    if( pLLMgr == MMP_NULL || cbCheckCondition == MMP_NULL )
    {
        printf("Null point (hLinkList = 0x%x, CB = 0x%x) !! %s [#%d]\n",
                    pLLMgr, cbCheckCondition, __FILE__, __LINE__);
        return LINKLIST_ERR_NULL_POINTER;
    }

    index = 0;
    currNode = pLLMgr->startNode;
    while( index < pLLMgr->nodeCount && currNode != MMP_NULL )
    {
        if( cbCheckCondition((void*)comparedData, (void*)currNode->nodeData) == MMP_TRUE )
        {
            // del node
            prevNode = currNode->prev;
            nextNode = currNode->next;
            prevNode->next = nextNode;
            nextNode->prev = prevNode;

            if( pLLMgr->cbDestroyNodeData == MMP_NULL ||
                pLLMgr->cbDestroyNodeData(currNode->nodeData) == MMP_FALSE )
            {
                if( currNode->nodeData )
                {
                    _heapFree(currNode->nodeData);
                    currNode->nodeData = MMP_NULL;
                }
            }

            _heapFree(currNode);
            break;
        }

        currNode = currNode->next;
        index++;
    }

    return result;
}

void* 
LL_getNode(
    HLINLLIST  hLinkList,
    void*      comparedData, 
    MMP_BOOL (*cbCheckCondition)(void* comparedData,  
                                 void* nodeData))
{
    LL_MGR      *pLLMgr = (LL_MGR*)hLinkList;
    LL_NODE     *pLLNode = MMP_NULL;
    int         index = 0;

    if( pLLMgr == MMP_NULL )
    {
        printf("Null pointer !! \n %s [#%d]\n", __FILE__, __LINE__);
        return MMP_NULL;
    }

    if( cbCheckCondition == MMP_NULL )
    {
        // if no defined CB, always return the last Node.
        pLLNode = pLLMgr->currNode;
    }
    else
    {
        LL_NODE     *currNode = MMP_NULL;

        index = 0;
        currNode = pLLMgr->startNode;
        while( index < pLLMgr->nodeCount && currNode != MMP_NULL )
        {
            if( cbCheckCondition((void*)comparedData, (void*)currNode->nodeData) == MMP_TRUE )
                break;

            currNode = currNode->next;
            index++;
        }

        if( currNode != MMP_NULL )
            pLLNode = currNode;
        else
        {
            printf("Can't find node !! %s [#%d]\n", __FILE__, __LINE__);
            return MMP_NULL;
        }
    }
    
    return (void*)pLLNode->nodeData;
}


