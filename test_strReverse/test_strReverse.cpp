//#include <stdlib.h>
#include <stdio.h>
//#include <string.h>


#if 1

int strReverse(char *inStr, int length, int startIdx)
{
    if( startIdx < (length>>1) )
    {
        inStr[length] = inStr[startIdx];
        inStr[startIdx] = inStr[length - 1 - startIdx];
        inStr[length - 1 - startIdx] = inStr[length];
        inStr[length] = 0;
        strReverse(inStr, length, startIdx + 1);
    }
    return 0;
}

void
main()
{
    char    txt[] = "123456";

    char    *pEnd = 0;

    pEnd = txt;
    while( *pEnd++ );
    pEnd--;

    strReverse(txt, (pEnd - txt), 0);

    printf("final: %s\n", txt);

    return ;
}

#else
int 
strReverse(
    char    *inStr,
    int     strLeng,
    int     startIdx)
{
    if( startIdx < (strLeng >> 1) )
    {
        char c = 0;

        c = inStr[startIdx];
        inStr[startIdx] = inStr[strLeng - startIdx - 1];
        inStr[strLeng - startIdx - 1] = c;
        strReverse(inStr, strLeng, startIdx + 1);
    }

    return 0;
}

void main()
{
    int i;
    char str[] = "123456789";

    printf("The string is: %s\n", str);
    
    strReverse(&str[5], 8-5, 0);
    printf("\t str = %s\n", str);

    for(i = 0; i < strlen(str); i++)
    {
        strncpy(str, "123456789", 9);
	    strReverse(str, strlen(str), i);
	    printf("Reversed string (%d) is: %s\n", i, str);
    }

    system("pause");
}

#endif