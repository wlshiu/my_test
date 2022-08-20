/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file utils.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/08/20
 * @license
 * @description
 */


#include <stdlib.h>
#include "utils.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ALLOC_STEP           32
#define CONFIG_LINE_SIZE_MAX        512
//=============================================================================
//                  Macro Definition
//=============================================================================
#define _tolowcase(chr)      (((chr) >='A' && (chr) <='Z') ? ((chr) + 32) : (chr))
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
ssize_t getline(char **pLine_buf, size_t *pLen, FILE *fin)
{
    size_t  num_read = 0;

    /* First check that none of our input pointers are NULL. */
    if( !pLine_buf || !pLen || !fin )
        return -1;

    /* If output buffer is NULL, then allocate a buffer. */
    if( NULL == *pLine_buf )
    {
        *pLine_buf = malloc(CONFIG_ALLOC_STEP);
        if( NULL == *pLine_buf )
        {
            /* Can't allocate memory. */
            return -1;
        }
        else
        {
            /* Note how big the buffer is at this time. */
            *pLen = CONFIG_ALLOC_STEP;
        }
    }

    /* Step through the file, pulling characters until either a newline or EOF. */

    {
        int  c;
        while( EOF != (c = getc(fin)) )
        {
            /* Note we read a character. */
            num_read++;

            /* Reallocate the buffer if we need more room */
            if( num_read >= *pLen )
            {
                size_t  n_realloc = *pLen + CONFIG_ALLOC_STEP;
                char    *tmp = realloc(*pLine_buf, n_realloc + 1); /* +1 for the trailing NUL. */
                if( NULL != tmp )
                {
                    /* Use the new buffer and note the new buffer size. */
                    *pLine_buf  = tmp;
                    *pLen       = n_realloc;
                }
                else
                {
                    /* Exit with error and let the caller free the buffer. */
                    return -1;
                }

                /* Test for overflow. */
                if( CONFIG_LINE_SIZE_MAX < *pLen )
                {
                    return -1;
                }
            }

            /* Add the character to the buffer. */
            (*pLine_buf)[num_read - 1] = (char)c;

            /* Break from the loop if we hit the ending character. */
            if( c == '\n' )
                break;
        }

        /* Note if we hit EOF. */
        if( EOF == c )
        {
            return -1;
        }
    }

    /* Terminate the string by suffixing NUL. */
    (*pLine_buf)[num_read] = '\0';

    return (ssize_t)num_read;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    if( n == 0 )    return 0;

    while( n-- != 0 && _tolowcase(*s1) == _tolowcase(*s2) )
    {
        if( n == 0 || *s1 == '\0' || *s2 == '\0' )
            break;
        s1++;
        s2++;
    }

    return _tolowcase(*(unsigned char*)s1) - _tolowcase(*(unsigned char*)s2);
}
