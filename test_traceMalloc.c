#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libLinkList.h"

//=============================================================================
//				  Constant Definition
//=============================================================================
typedef unsigned int    MMP_UINT32;
typedef int             MMP_INT;

#define MMP_SIZE_T int
#define MMP_NULL   0


#define  testAlloc(name, length)    PalHeapAlloc(name, length, __FUNCTION__)
#define  testFree(name, addr)       PalHeapFree(name, addr, __FUNCTION__)
//=============================================================================
//				  Global Data Definition
//=============================================================================

typedef struct _HEAP_INFO_TAG
{
    MMP_UINT32  uid;  // using memory address 
    MMP_INT     lineNum;
    char        funcName[32];
    char        bEnTracd;   // for output message
}HEAP_INFO;


HLINLLIST   g_linkList = MMP_NULL;
//=============================================================================
//				  Private Function Definition
//=============================================================================
static MMP_BOOL
_CheckDelCondition(
                   void    *comparedData,
                   void    *nodeData)
{
    if( ((HEAP_INFO*)nodeData)->uid == ((HEAP_INFO*)comparedData)->uid )
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

static MMP_BOOL
_CheckGetCondition(
                void* comparedData, 
                void* nodeData)
{
    if( nodeData != MMP_NULL && ((HEAP_INFO*)nodeData)->funcName )
    {
        printf("%s()\n", ((HEAP_INFO*)nodeData)->funcName);
    }

    return MMP_FALSE;
}

void*
PalHeapAlloc(
             MMP_INT    name,
             MMP_SIZE_T size,
             char*      funcNum)
{
    void* mem;
    HEAP_INFO   info = {0};

    //if ((mem = (void*) MEM_DeployEdge(size)) == 0)
    mem = malloc(size);

    info.uid = (MMP_UINT32)mem;
    strcpy(info.funcName, funcNum);

    LL_addNode(g_linkList, &info, sizeof(HEAP_INFO));

    return mem;
}

void
PalHeapFree(
            MMP_INT     name,
            void*       ptr,
            char*       funcNum)
{
    if (ptr != MMP_NULL)
    {
        HEAP_INFO   info = {0};

        //if (MEM_ReleaseEdge(ptr) == MMP_FALSE)
        free(ptr);
        ptr = MMP_NULL;

        info.uid = (MMP_UINT32)ptr;
        LL_delNode(g_linkList, &info, _CheckDelCondition);
    }
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
int
main(void)
{
    char *buf0 = 0;
    char *buf1 = 0;
    int i = 0;

    g_linkList = LL_createNodes(sizeof(HEAP_INFO), MMP_NULL, MMP_NULL);


    buf0 = testAlloc(0, 256);
    buf1 = testAlloc(0, 128);

    testFree(0, buf0);
    
    while( LL_getNode(g_linkList, MMP_NULL, _CheckGetCondition) )
        ;

    LL_destroyNotes(g_linkList);
    g_linkList = MMP_NULL;

    system("pause");
    testFree(0, buf1);

    return 1;
}


