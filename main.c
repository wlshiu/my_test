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

        backtrace((unsigned long)pStack, (unsigned long)pStack + filesize, 0, 0);


    } while(0);

    if( fin )       fclose(fin);
    if( pStack )    free(pStack);

    system("pause");
    return 0;
}
