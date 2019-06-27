
#ifndef __LINKLIST_H_4VYTWDI7_CRZR_F1B5_2KLS_G46R3VHINCY3__
#define __LINKLIST_H_4VYTWDI7_CRZR_F1B5_2KLS_G46R3VHINCY3__

#ifdef __cplusplus
extern "C" {
#endif


typedef void*   HLINLLIST;
//=============================================================================
//				  Constant Definition
//=============================================================================
#ifndef MMP_BOOL
typedef enum _MMP_BOOL_TAG
{
    MMP_FALSE = 0,
    MMP_TRUE
}MMP_BOOL;
#endif

typedef enum LINKLIST_ERR_TAG
{
    LINKLIST_ERR_OK                 = 0,
    LINKLIST_ERR_NULL_POINTER,
    LINKLIST_ERR_ALLOCATE_FAIL,
    LINKLIST_ERR_ALLOCATE_WRONG_SIZE,
    LINKLIST_ERR_UNKNOW,

}LINKLIST_ERR;

//=============================================================================
//				  Public Function Definition
//=============================================================================
HLINLLIST
LL_createNodes(
    int         oneNodeSize,
    MMP_BOOL    (*cbCreateNodeData)(void**  nodeInfo, 
                                    void*   inputData,
                                    int     inputDataSize),
    MMP_BOOL    (*cbDestroyNodeData)(void*));


void 
LL_destroyNotes(
    HLINLLIST  hLinkList);


LINKLIST_ERR
LL_addNode(
    HLINLLIST  hLinkList,
    void*      inputData,
    int        inputDataSize);


LINKLIST_ERR
LL_delNode(
    HLINLLIST  hLinkList,
    void*      comparedData, 
    MMP_BOOL (*cbCheckCondition)(void* comparedData,
                                 void* nodeData));


void* 
LL_getNode(
    HLINLLIST  hLinkList,
    void*      comparedData, 
    MMP_BOOL (*cbCheckCondition)(void* comparedData,
                                 void* nodeData));


#ifdef __cplusplus
}
#endif

#endif

