/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file elf.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/07/08
 * @license
 * @description
 */


#include <stdio.h>
#include <string.h>
#include "elf32.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define is_elf64(pElfHdr)       ((pElfHdr)->e_ident[EI_CLASS] == ELFCLASS64)

#define msg(str, ...)           printf(str, ##__VA_ARGS__)

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
static bool
_is_elf32(Elf32_Ehdr *pEh)
{
    bool    rval = false;
    do {
        /**
         * ELF magic bytes are 0x7f,'E','L','F'
         * Using  octal escape sequence to represent 0x7f
         */
        if( strncmp((char*)pEh->e_ident, "\177ELF", 4) )
        {
            err("ELFMAGIC mismatch!\n"); // not a ELF file
            break;
        }

        rval = (is_elf64(pEh)) ? false : true;
    } while(0);
    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
elf32_log_info(
    uint8_t     *pElf_raw,
    uint32_t    data_len)
{
    int     rval = -1;
    do {
        Elf32_Ehdr      *pElf_hdr = (Elf32_Ehdr*)pElf_raw;
        Elf32_Shdr      *pSection_hdr = 0;
        Elf32_Shdr      *pSec_strtab = 0;
        char            *pSh_name_tab = 0;

        msg("Magic: ");
        for(int i = 0; i < 16; i++)
            msg("%02x ", pElf_hdr->e_ident[i]);

        if( _is_elf32(pElf_hdr) == false )
        {
            err("Not elf32 file \n");
            break;
        }

        msg("Type                     : %d\n", pElf_hdr->e_type);
        msg("Machine                  : %d\n", pElf_hdr->e_machine); /* Architecture */
        msg("Version                  : %#02x\n", pElf_hdr->e_version);
        msg("Entry point address      : %#02x\n", pElf_hdr->e_entry);
        msg("Start of program headers : %d(bytes)\n", pElf_hdr->e_phoff);
        msg("Start of section headers : %d(bytes)\n", pElf_hdr->e_shoff);
        msg("Flags                    : %#02x\n", pElf_hdr->e_flags);
        msg("Size of this header      : %d(bytes)\n", pElf_hdr->e_ehsize);
        msg("Size of program headers  : %d(bytes)\n", pElf_hdr->e_phentsize);
        msg("Number of program headers: %d\n", pElf_hdr->e_phnum);
        msg("Size of section headers  : %d(bytes)\n", pElf_hdr->e_shentsize);
        msg("Number of section headers: %d\n", pElf_hdr->e_shnum);
        msg("Section header string table index: %d\n", pElf_hdr->e_shstrndx);

        if( pElf_hdr->e_ehsize != sizeof(Elf32_Ehdr) )
        {
            err("\nELF file header size is err\n!");
            break;
        }

        if( pElf_hdr->e_type != ET_REL && pElf_hdr->e_type != ET_EXEC )
        {
            err("file type is err\n");
            break;
        }

        msg("\n=================== section header table =================\n");
        pSection_hdr = (Elf32_Shdr*)((uintptr_t)pElf_raw + pElf_hdr->e_shoff);
        pSec_strtab  = &pSection_hdr[pElf_hdr->e_shstrndx];
        pSh_name_tab = (char*)((uintptr_t)pElf_raw + pSec_strtab->sh_offset);

        msg("[Nr]   Name                    Type          Addr         Off       Size    ES  Flg  Al");
        for(int i = 0; i < pElf_hdr->e_shnum; i++)
        {
            msg("\n[%02d]  %-15s      %8x         %08x     %06x    %06x   %02x  %02x   %02x ",
                i, pSh_name_tab + pSection_hdr->sh_name, pSection_hdr->sh_type, pSection_hdr->sh_addr,
                pSection_hdr->sh_offset, pSection_hdr->sh_size, pSection_hdr->sh_entsize,
                pSection_hdr->sh_flags, pSection_hdr->sh_addralign);

            // if symtab
            if( pSection_hdr->sh_type == 2 )
            {
                Elf32_Sym   *pSymbol_tab = 0;

                pSymbol_tab = (Elf32_Sym*)((uintptr_t)pElf_raw + pSection_hdr->sh_offset);
            }
            pSection_hdr++;
        }

        rval = 0;
    } while(0);
    return rval;
}

int
elf32_load_to_vma(
    uint8_t     *pElf_raw)
{
    int     rval = 0;
    do{
        Elf32_Ehdr      *pElf_hdr = (Elf32_Ehdr*)pElf_raw;
        Elf32_Phdr      *pProg_hdr = 0;

        pProg_hdr = (Elf32_Phdr*)((uintptr_t)pElf_raw + pElf_hdr->e_phoff);

        msg("\n============= Program Headers:\n");
        msg("starting at offset       : %d\n", pElf_hdr->e_phoff);
        msg("Number of program headers: %d\n", pElf_hdr->e_phnum);
        msg("Type           Offset   VirtAddr     PhysAddr   FileSiz     MemSiz     Flg\n");
        for(int num = 0; num < pElf_hdr->e_phnum; num++)
        {
            msg("x%08x    %8x   %8x   %8x  %8x    %8x  %8x\n",
               pProg_hdr->p_type, pProg_hdr->p_offset,
               pProg_hdr->p_vaddr, pProg_hdr->p_paddr,
               pProg_hdr->p_filesz, pProg_hdr->p_memsz,
               pProg_hdr->p_flags);
            pProg_hdr++;
        }

        pProg_hdr = (Elf32_Phdr*)((uintptr_t)pElf_raw + pElf_hdr->e_phoff);
        for(int i = 0; i < pElf_hdr->e_phnum; i++, pProg_hdr++)
        {
            if( pProg_hdr->p_type != PT_LOAD )
            {
                msg("\nNot PT_LOAD x%08x (prog section num= %d)\n",
                       pProg_hdr->p_type, pElf_hdr->e_phnum);
                continue;
            }

            if( pProg_hdr->p_filesz )
            {
                uint8_t     *pRaw_data = 0;

                pRaw_data = (uint8_t*)((uintptr_t)pElf_raw + pProg_hdr->p_offset);
                msg("copy raw: 0x%08x -> vma (0x%08x), len= 0x%x\n",
                    (uintptr_t)pProg_hdr->p_offset,
                    (uintptr_t)pProg_hdr->p_vaddr,
                    pProg_hdr->p_filesz);
            }

            // bss section
            if( pProg_hdr->p_filesz < pProg_hdr->p_memsz )
            {
                uint32_t    bss_len = pProg_hdr->p_memsz - pProg_hdr->p_filesz;
                msg("bss: 0x%08x, len= 0x%x\n", (uintptr_t)pProg_hdr->p_vaddr + pProg_hdr->p_filesz, bss_len);
            }
        }

        rval = 0;
    } while(0);
    return rval;
}





