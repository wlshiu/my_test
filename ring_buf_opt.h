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
    unsigned char     *pRead_ptr;
    unsigned char     *pWrite_ptr;
    unsigned char     *pBuf_start_ptr;
    unsigned char     *pBuf_end_ptr;
    unsigned char     *pValid_end_ptr;
}rb_operator_t;

typedef int (*get_item_size)(unsigned char *w_ptr, unsigned char *r_ptr, unsigned int *pItem_size);
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static int
rb_opt_init(
    rb_operator_t   *pRbOpt,
    unsigned char   *pStart_ptr,
    unsigned int    buf_size)
{
    if( !pRbOpt )   return -1;
    pRbOpt->pBuf_start_ptr = pStart_ptr + (buf_size & 0x3);
    pRbOpt->pRead_ptr      = pRbOpt->pBuf_start_ptr;
    pRbOpt->pWrite_ptr     = pRbOpt->pBuf_start_ptr;
    pRbOpt->pBuf_end_ptr   = pStart_ptr + buf_size;
    pRbOpt->pValid_end_ptr = pRbOpt->pBuf_end_ptr;
    return 0;
}

static int
rb_opt_update_w(
    rb_operator_t   *pRbOpt,
    unsigned char   *pHeader,
    unsigned int    header_size,
    unsigned char   *pData,
    unsigned int    data_size)
{
    unsigned char    *w_ptr = pRbOpt->pWrite_ptr;
    unsigned char    *r_ptr = pRbOpt->pRead_ptr;
    unsigned int     item_size = header_size + data_size;

    // 32 bits alignment
    item_size = (item_size + 0x3) & ~0x3;

    if( w_ptr > r_ptr )
    {
        if( (w_ptr + item_size) > pRbOpt->pBuf_end_ptr )
        {
            if( (pRbOpt->pBuf_start_ptr + item_size) >= r_ptr )
            {
                printf("\t!! no space to write, drop current item !!\n");
                return -1;
            }

            pRbOpt->pValid_end_ptr = (unsigned char*)(((unsigned int)w_ptr + 0x3) & ~0x3);
            printf("\t!! w-> valid_end 0x%x\n", w_ptr);
            w_ptr = pRbOpt->pBuf_start_ptr;
        }
    }
    else
    {
        if( w_ptr != r_ptr &&
            (w_ptr + item_size) >= r_ptr )
        {
            printf("\t!! write catch read, drop current item !!\n");
            return -1;
        }
    }


    printf("\t write_ptr = 0x%x\n", w_ptr);

    memcpy((void*)w_ptr, (void*)pHeader, header_size);
    w_ptr += header_size;
    memcpy((void*)w_ptr, (void*)pData, data_size);
    w_ptr += data_size;

    // 32 bits alignment
    w_ptr = (unsigned char*)(((unsigned int)w_ptr + 0x3) & ~0x3);

    pRbOpt->pWrite_ptr = w_ptr;
    return 0;
}

static int
rb_opt_update_r(
    rb_operator_t   *pRbOpt,
    unsigned char   **ppData,
    unsigned int    *pData_size,
    get_item_size   cb_get_item_size)
{
    unsigned char     *w_ptr = pRbOpt->pWrite_ptr;
    unsigned char     *r_ptr = pRbOpt->pRead_ptr;
    unsigned int      item_size = 0;
    unsigned int      item_size_align = 0;

    if( cb_get_item_size == NULL )
        return  -1;

    cb_get_item_size(w_ptr, r_ptr, &item_size);
    if( item_size == 0 )
    {
        *ppData     = NULL;
        *pData_size = 0;
        return -2;
    }

    // 32 bits alignment
    item_size_align = (item_size + 0x3) & ~0x3;

    if( w_ptr < r_ptr ||
        (r_ptr + item_size_align) < w_ptr )
    {
        *ppData     = r_ptr;
        *pData_size = item_size;

        printf("\t read_ptr  = 0x%x\n", r_ptr);

        r_ptr += item_size_align;

        if( r_ptr == pRbOpt->pValid_end_ptr )
        {
            pRbOpt->pValid_end_ptr = pRbOpt->pBuf_end_ptr;
            r_ptr = pRbOpt->pBuf_start_ptr;

            printf("!! r_ptr at valid_end 0x%x\n", r_ptr);
        }
    }
    else
    {
        *ppData     = NULL;
        *pData_size = 0;
    }

    pRbOpt->pRead_ptr = r_ptr;

    return 0;
}
//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
