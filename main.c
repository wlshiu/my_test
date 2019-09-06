
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elf32.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#if 1
    #define err(str, ...)      do{printf("[%s:%u] " str, __func__, __LINE__, ##__VA_ARGS__); while(1);}while(0)
#else
    #define err(str, ...)
#endif // 1
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
int main(void)
{
    FILE        *felf = 0;
    char        *elf_name = "arm.elf";
    uint8_t     *pElf_data = 0;

    do {
        uint32_t    filesize = 0;
        if( !(felf = fopen(elf_name, "rb")) )
        {
            printf("open %s fail \n", elf_name);
            break;
        }

        fseek(felf, 0ul, SEEK_END);
        filesize = ftell(felf);
        fseek(felf, 0ul, SEEK_SET);

        if( !(pElf_data = malloc(filesize)) )
        {
            err("malloc %d fail \n", filesize);
            break;
        }

        fread(pElf_data, 1, filesize, felf);
        fclose(felf);

        elf32_log_info(pElf_data, filesize);

        elf32_load_to_vma(pElf_data);

    } while(0);

    if( felf )  fclose(felf);

    while(1)
        __asm("nop");
    return 0;
}

