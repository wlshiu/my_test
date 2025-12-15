// func1.c
#include <stdio.h>
#include <stdlib.h>


// Some RW and ZI data
char* func1_string = "func1 called\n";
int func1_values[20];

void func1(void)
{
    unsigned int i;
    printf("%s\n", func1_string);
    for(i = 19; i; i--)
    {
        func1_values[i] = rand();
        printf("%d ", func1_values[i]);
    }
    printf("\n");
}
