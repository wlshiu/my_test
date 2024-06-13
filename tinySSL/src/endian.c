/******************************************************************************
*
* Copyright © 2014 Anthony Mai Mai_Anthony@hotmail.com. All Rights Reserved.
*
* This software is written by Anthony Mai who retains full copyright of this
* work. As such any Copyright Notices contained in this code. are NOT to be
* removed or modified. If this package is used in a product, Anthony Mai
* should be given attribution as the author of the parts of the library used.
* This can be in the form of a textual message at program startup or in
* documentation (online or textual) provided with the package.
*
* This library is free for commercial and non-commercial use as long as the
* following conditions are aheared to. The following conditions apply to
* all code found in this distribution:
*
* 1. Redistributions of source code must retain the copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. All advertising materials mentioning features or use of this software
*    must display the following acknowledgement:
*
*    "This product contains software written by Anthony Mai (Mai_Anthony@hotmail.com)
*     The original source code can obtained from such and such internet sites or by
*     contacting the author directly."
*
* 4. This software may or may not contain patented technology owned by a third party.
*    Obtaining a copy of this software, with or without explicit authorization from
*    the author, does NOT imply that applicable patents have been licensed. It is up
*    to you to make sure that utilization of this software package does not infringe
*    on any third party's patents or other intellectual proerty rights.
*
* THIS SOFTWARE IS PROVIDED BY ANTHONY MAI "AS IS". ANY EXPRESS OR IMPLIED WARRANTIES,
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The licence and distribution terms for any publically available version or derivative
* of this code cannot be changed.  i.e. this code cannot simply be copied and put under
* another distribution licence [including the GNU Public Licence.]
*
******************************************************************************/

/******************************************************************************
*
*  File Name:       endian.c
*
*  Description:     Endianness conversion between native and network order.
*
*
*  Programmers:     Anthony Mai (am) mai_anthony@hotmail.com
*
*  History:         6/27/2014 Initial creation
*
*  Notes:           This file uses 4 spaces indents
*
******************************************************************************/

#include <stdint.h>

#include "endian.h"


/******************************************************************************
* Function:     Int2Byte
*
* Description:  Native endian 4 bytes integers to network order bytes.
*
* Returns:      None
******************************************************************************/
void Int2Byte(const int *pIn, int8_t *pOut, int nLen)
{
    for ( ; nLen--; )
    {
        int    val = *pIn++;
        *pOut++ = (int8_t)(val >> 24);
        *pOut++ = (int8_t)(val >> 16);
        *pOut++ = (int8_t)(val >> 8);
        *pOut++ = (int8_t)(val);
    }
}


/******************************************************************************
* Function:     Byte2Int
*
* Description:  Network order bytes to native endian 4 bytes integers.
*
* Returns:      None
******************************************************************************/
void Byte2Int(const int8_t *pIn, int *pOut, int nLen)
{
    for ( ; nLen--; )
    {
        int    val;

        val =            (*pIn++);
        val = (val << 8) + (*pIn++);
        val = (val << 8) + (*pIn++);
        val = (val << 8) + (*pIn++);
        *pOut++ = val;
    }
}


/******************************************************************************
* Function:     Int2LByte
*
* Description:  Native endian 4 bytes integers to little endian order bytes.
*
* Returns:      None
******************************************************************************/
void Int2LByte(const int *pIn, int8_t *pOut, int nLen)
{
    for ( ; nLen--; )
    {
        int    val = *pIn++;
        *pOut++ = (int8_t)(val);
        *pOut++ = (int8_t)(val >> 8);
        *pOut++ = (int8_t)(val >> 16);
        *pOut++ = (int8_t)(val >> 24);
    }
}


/******************************************************************************
* Function:     LByte2Int
*
* Description:  Little endian order bytes to native endian 4 bytes integers.
*
* Returns:      None
******************************************************************************/
void LByte2Int(const int8_t *pIn, int *pOut, int nLen)
{
    for ( ; nLen--; )
    {
        int    val;

        val  = ((int)(*pIn++));
        val += ((int)(*pIn++)) << 8;
        val += ((int)(*pIn++)) << 16;
        val += ((int)(*pIn++)) << 24;
        *pOut++ = val;
    }
}
