#include <stdio.h>
#include <stdlib.h>

#include "backtrace.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static backtrace_txt_range_t    g_txt_range[] =
{
    { .start = 0x30000000, .end = 0x30200000, },
    { .start = 0x00000000, .end = 0x00020000, },
    { .start = 0x60000000, .end = 0x00040000, },
};
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    FILE            *fin = 0;
    unsigned long   *pStack = 0;

    do {
        int             filesize = 0;

        if( !(fin = fopen(argv[1], "rb")) )
        {
            break;
        }

        fseek(fin, 0, SEEK_END);
        filesize = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        if( !(pStack = (unsigned long*)malloc(filesize)) )
        {
            break;
        }

        fread(pStack, 1, filesize, fin);
        fclose(fin);
        fin = 0;

        backtrace_init(&g_txt_range, sizeof(g_txt_range)/sizeof(g_txt_range[0]));

        backtrace((unsigned long)pStack, (unsigned long)pStack + filesize, 0, 0);


    } while(0);

    if( fin )       fclose(fin);
    if( pStack )    free(pStack);

    system("pause");
    return 0;
}
