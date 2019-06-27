#ifndef __rb_opt_template_H_w2e8RcxR_lasx_HFnD_swSB_u3NtoMPaJ7aa__
#define __rb_opt_template_H_w2e8RcxR_lasx_HFnD_swSB_u3NtoMPaJ7aa__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum rb_read_type
{
    RB_READ_TYPE_PEEK       = 0,
    RB_READ_TYPE_REMOVE,
    RB_READ_TYPE_ALL
}rb_read_type;
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 *  ring buffer operator, never stop when write data
 */
typedef struct rb_operator
{
    unsigned char     *pRead_ptr[RB_READ_TYPE_ALL];
    unsigned char     *pWrite_ptr;
    unsigned char     *pBuf_start_ptr;
    unsigned char     *pBuf_end_ptr;
    unsigned char     *pValid_end_ptr[RB_READ_TYPE_ALL];
}rb_operator_t;

typedef int (*get_item_size)(unsigned char *w_ptr, unsigned char *r_ptr, unsigned int *pItem_size);
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
int
rb_opt_init(
    rb_operator_t   *pRbOpt,
    unsigned char   *pStart_ptr,
    unsigned int    buf_size);

int
rb_opt_update_w(
    rb_operator_t   *pRbOpt,
    unsigned char   *pHeader,
    unsigned int    header_size,
    unsigned char   *pData,
    unsigned int    data_size);


int
rb_opt_update_r(
    rb_operator_t   *pRbOpt,
    rb_read_type    read_idx,
    unsigned char   **ppData,
    unsigned int    *pData_size,
    get_item_size   cb_get_item_size);


#ifdef __cplusplus
}
#endif

#endif
