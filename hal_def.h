/**
 * Copyright (c) 2024 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_def.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2024/09/10
 * @license
 * @description
 */

#ifndef __hal_def_H_wrmPni7q_lbcH_Hmna_sBkY_uUTtDspcvhdh__
#define __hal_def_H_wrmPni7q_lbcH_Hmna_sBkY_uUTtDspcvhdh__

#ifdef __cplusplus
extern "C" {
#endif



#include "hal_sim.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#if defined(__GNUC__)

#else
    #warning Not supported compiler type
#endif

typedef enum
{
    DISABLE = 0,
    ENABLE  = !DISABLE
} FuncStatus, FunctionalState;

typedef enum
{
    RESET = 0,
    SET   = !RESET
} FlagStatus, ITStatus;

typedef enum
{
    ERROR = 0,
    SUCCESS = !ERROR
} ErrorStatus;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define REG_SET_BITS(__reg32__, __bit_msk__)                ((__reg32__) |= (__bit_msk__))
#define REG_CLR_BITS(__reg32__, __bit_msk__)                ((__reg32__) &= ~(__bit_msk__))

#define REG_READ(__reg32__)                                 (__reg32__)
#define REG_READ_MASK(__reg32__, __msk__)                   ((__reg32__) & (__msk__))

#define REG_WRITE(__reg32__, __val__)                       ((__reg32__) = (__val__))
#define REG_WRITE_MASK(__reg32__, __msk__, __val__)         ((__reg32__) = ((__reg32__) & ~(__msk__)) | ((__val__) & (__msk__)))

#define ALIGN_4(x)                  (((x) + 0x3ul) & ~0x3ul)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
/**
 *  register 32-bits operators
 */
__STATIC_INLINE void reg32_set_bits(uint32_t *addr, uint32_t bit_msk)
{
    *((uint32_t*)addr) |= bit_msk;
    return;
}

__STATIC_INLINE void reg32_clr_bits(uint32_t *addr, uint32_t bit_msk)
{
    *((uint32_t*)addr) &= ~bit_msk;
    return;
}

__STATIC_INLINE uint32_t reg32_read(uint32_t *addr)
{
    return *((uint32_t*)addr);
}

__STATIC_INLINE uint32_t reg32_read_mask(uint32_t *addr, uint32_t mask)
{
    return (*((uint32_t*)addr) & mask);
}

__STATIC_INLINE void reg32_write(uint32_t *addr, uint32_t value)
{
    *(uint32_t*)addr = value;
    return;
}

__STATIC_INLINE void reg32_write_mask(uint32_t *addr, uint32_t mask, uint32_t value)
{
    *(uint32_t*)addr = ((*(uint32_t*)addr) & ~mask) | (value & mask);
    return;
}

/**
 *  register 16-bits operators
 */
__STATIC_INLINE void reg16_set_bits(uint16_t *addr, uint16_t bit_msk)
{
    *((uint16_t*)addr) |= bit_msk;
    return;
}

__STATIC_INLINE void reg16_clr_bits(uint16_t *addr, uint16_t bit_msk)
{
    *((uint16_t*)addr) &= ~bit_msk;
    return;
}

__STATIC_INLINE uint16_t reg16_read(uint16_t *addr)
{
    return *((uint16_t*)addr);
}

__STATIC_INLINE uint16_t reg16_read_mask(uint16_t *addr, uint16_t mask)
{
    return (*((uint16_t*)addr) & mask);
}

__STATIC_INLINE void reg16_write(uint16_t *addr, uint16_t value)
{
    *(uint16_t*)addr = value;
    return;
}

__STATIC_INLINE void reg16_write_mask(uint16_t *addr, uint16_t mask, uint16_t value)
{
    *(uint16_t*)addr = ((*(uint16_t*)addr) & ~mask) | (value & mask);
    return;
}

/**
 *  register 8-bits operators
 */
__STATIC_INLINE void reg8_set_bits(uint8_t *addr, uint8_t bit_msk)
{
    *((uint8_t*)addr) |= bit_msk;
    return;
}

__STATIC_INLINE void reg8_clr_bits(uint8_t *addr, uint8_t bit_msk)
{
    *((uint8_t*)addr) &= ~bit_msk;
    return;
}

__STATIC_INLINE uint8_t reg8_read(uint8_t *addr)
{
    return *((uint8_t*)addr);
}

__STATIC_INLINE uint8_t reg8_read_mask(uint8_t *addr, uint8_t mask)
{
    return (*((uint8_t*)addr) & mask);
}

__STATIC_INLINE void reg8_write(uint8_t *addr, uint8_t value)
{
    *(uint8_t*)addr = value;
    return;
}

__STATIC_INLINE void reg8_write_mask(uint8_t *addr, uint8_t mask, uint8_t value)
{
    *(uint8_t*)addr = ((*(uint8_t*)addr) & ~mask) | (value & mask);
    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
