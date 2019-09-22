/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file circbuf_template.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/09/22
 * @license
 * @description
 */


#ifndef __circbuf_template_H_1e822d71_9f65_4efb_b343_ced91b03e75b__
#define __circbuf_template_H_1e822d71_9f65_4efb_b343_ced91b03e75b__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
#define DECLARE_CBUF(T, Name)  \
  typedef struct _##Name{      \
    int len, pos_wr, pos_rd;   \
    T* pBuffer;                \
  } Name

//=============================================================================
//                  Macro Definition
//=============================================================================

#define CBUF_NEXT_POS(pos, len)             (((pos) + 1) % (len))
#define CBUF_IS_EMPTY(pHCBuf)               ((pHCBuf)->pos_wr == (pHCBuf)->pos_rd)
#define CBUF_IS_FULL(pHCBuf)                (CBUF_NEXT_POS((pHCBuf)->pos_wr, (pHCBuf)->len) == (pHCBuf)->pos_rd)



#define cbuf_init(pHCBuf, pBuf, buf_size)       do{ (pHCBuf)->pos_rd = (pHCBuf)->pos_wr = 0;  \
                                                    (pHCBuf)->len = buf_size;                 \
                                                    (pHCBuf)->pBuffer = pBuf;                 \
                                                }while(0)


#define cbuf_push(pHCBuf, T, data, pRst)                            \
    do{ int pos_rd, pos_wr, next_pos_wr;                            \
        pos_wr = (pHCBuf)->pos_wr, pos_rd = (pHCBuf)->pos_rd;       \
        next_pos_wr = CBUF_NEXT_POS(pos_wr, (pHCBuf)->len);         \
        if( next_pos_wr == pos_rd ) { *pRst = -1; break; }          \
        (pHCBuf)->pBuffer[pos_wr] = (T)(data);                      \
        (pHCBuf)->pos_wr = next_pos_wr;                             \
    }while(0)

#define cbuf_pop(pHCBuf, T, pData, pRst)                             \
    do{ int pos_rd, pos_wr;                                          \
        pos_wr = (pHCBuf)->pos_wr, pos_rd = (pHCBuf)->pos_rd;        \
        if( pos_rd == pos_wr ) { *pRst = -2; break; }                \
        *((T*)(pData)) = (pHCBuf)->pBuffer[pos_rd];                  \
        pos_rd = CBUF_NEXT_POS(pos_rd, (pHCBuf)->len);               \
        (pHCBuf)->pos_rd = pos_rd;                                   \
    }while(0)



#if 0
#define _INIT(pRi_ops, max_cnt, T)                          \
    do{ (pRi_ops)->start = (pRi_ops)->end = 0;                    \
        (pRi_ops)->size = (max_cnt) + 1;                          \
        (pRi_ops)->elems = (T*)calloc(pRi_ops->size, sizeof(T));  \
    }while(0)

/**
 *  If end_idx catches start_idx, replace the data and force start_idx forward
 */
#define _WRITE(pRi_ops, elem_data)                          \
    do{ (pRi_ops)->elems[(pRi_ops)->end] = (elem_data);           \
        (pRi_ops)->end = ((pRi_ops)->end+1) % (pRi_ops)->size;    \
        if( RI_OPS_IS_EMPTY(pRi_ops) ) {                          \
            (pRi_ops)->start = RI_OPS_NEXT_START_IDX(pRi_ops);    \
        }                                                         \
    }while(0)


#define _READ(pRi_ops, elem_data)                           \
    do{ elem_data = (pRi_ops)->elems[(pRi_ops)->start];           \
        (pRi_ops)->start = RI_OPS_NEXT_START_IDX(pRi_ops);        \
    }while(0)

#endif
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif


