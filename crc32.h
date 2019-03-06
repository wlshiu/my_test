/**
 * @file   crc32.h
 * @author pesoli <pesoli@pc-eusodp.roma2.infn.it>
 * @date   Thu Jan  3 09:33:41 2013
 *
 * @brief
 *
 *
 */

#ifndef _CRC32_H
#define _CRC32_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

/**
 * Provides a table driven implementation of the IEEE-802.3 32-bit CRC
 * algorithm for byte data.
 *
 * @param data Pointer to the byte data
 * @param dataLen Number of bytes of data to be processed
 * @param crc Initial CRC value to be used (can be the output from a previous call to this function)
 *
 * @return  32-bit CRC value for the specified data
 */
uint32_t crc32(uint8_t *data, unsigned int dataLen);


#ifdef __cplusplus
}
#endif

#endif
