
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elf.h"


static int check_elf_head(FILE **elf)
{
    int     rval = 0;
    FILE    *felf = *elf;

    do {
        uint32_t    file_size = 0;
        Elf32_Ehdr  elf_hdr = {0};

        fseek(felf, 0, SEEK_END);
        file_size = ftell(felf);
        fseek(felf, 0, SEEK_SET);
        printf("file total size is: %u bytes\n", file_size);

        fread(&elf_hdr, 1, sizeof(Elf32_Ehdr), felf);

        printf("Magic: ");
        for(int i = 0; i < 16; i++)
            printf("%02x ", elf_hdr.e_ident[i]);

        if( (elf_hdr.e_ident[0] == '\x7f') && (elf_hdr.e_ident[1] == ELFMAG1) &&
            (elf_hdr.e_ident[2] == ELFMAG2) && (elf_hdr.e_ident[3] == ELFMAG3))
            printf("\nThis is ELF file!\n\n");
        else
        {
            printf("\n NOT ELF file!\n\n");
            rval = -1;
            break;
        }

        printf("Type                     : %d\n", elf_hdr.e_type);
        printf("Machine                  : %d\n", elf_hdr.e_machine); /* Architecture */
        printf("Version                  : %#02x\n", elf_hdr.e_version);
        printf("Entry point address      : %#02x\n", elf_hdr.e_entry);
        printf("Start of program headers : %d(bytes)\n", elf_hdr.e_phoff);
        printf("Start of section headers : %d(bytes)\n", elf_hdr.e_shoff);
        printf("Flags                    : %#02x\n", elf_hdr.e_flags);
        printf("Size of this header      : %d(bytes)\n", elf_hdr.e_ehsize);
        printf("Size of program headers  : %d(bytes)\n", elf_hdr.e_phentsize);
        printf("Number of program headers: %d\n", elf_hdr.e_phnum);
        printf("Size of section headers  : %d(bytes)\n", elf_hdr.e_shentsize);
        printf("Number of section headers: %d\n", elf_hdr.e_shnum);
        printf("Section header string table index: %d\n", elf_hdr.e_shstrndx);

        if( elf_hdr.e_ehsize != sizeof(elf_hdr) )
        {
            printf("\nELF file header size is err\n!");
            rval = -1;
            break;
        }

        if( elf_hdr.e_type != ET_REL && elf_hdr.e_type != ET_EXEC )
        {
            printf("file type is err\n");
            rval = -1;
            break;
        }

        {
            Elf32_Shdr  *pSection_hdr = 0;
            char        *pSh_name_tab = 0;
            int         shnum = elf_hdr.e_shnum;

            fseek(felf, elf_hdr.e_shoff, SEEK_SET);
            printf("\n/***** section header table ****/\n");

            if( !(pSection_hdr = malloc(sizeof(Elf32_Shdr) * shnum)) )
            {
                rval = -1;
                break;
            }

            fread(pSection_hdr, sizeof(Elf32_Shdr), shnum, felf);

            {
                Elf32_Shdr   *pSec_strtab = &pSection_hdr[elf_hdr.e_shstrndx];
                if( !(pSh_name_tab = malloc(pSec_strtab->sh_size)) )
                {
                    printf("malloc %d fail \n", pSec_strtab->sh_size);
                    rval = -1;
                    break;
                }

                fseek(felf, pSec_strtab->sh_offset, SEEK_SET);
                fread(pSh_name_tab, 1, pSec_strtab->sh_size, felf);
            }


            printf("[Nr]   Name                         Type          Addr         Off       Size    ES  Flg  Al");

            for(int i = 0; i < shnum; i++)
            {
                printf("\n[%02d]  %-20s      %8x         %08x     %06x    %06x   %02x  %02x   %02x ",
                       i, pSh_name_tab + pSection_hdr->sh_name, pSection_hdr->sh_type, pSection_hdr->sh_addr,
                       pSection_hdr->sh_offset, pSection_hdr->sh_size, pSection_hdr->sh_entsize,
                       pSection_hdr->sh_flags, pSection_hdr->sh_addralign);

                // if symtab
                if( pSection_hdr->sh_type == 2 )
                {
                    Elf32_Sym   *pSymbol_tab = 0;
                    uint32_t    symsize = pSection_hdr->sh_size;
                    uint32_t    symoff = pSection_hdr->sh_offset;
                    uint32_t    nSyms = symsize / (pSection_hdr->sh_entsize);

                    if( !(pSymbol_tab = malloc(sizeof(Elf32_Sym) * nSyms)) )
                    {
                        rval = -1;
                        break;
                    }

                    fseek(felf, symoff, SEEK_SET);
                    fread(pSymbol_tab, sizeof(Elf32_Sym), nSyms, felf);

                    free(pSymbol_tab);
                }
                pSection_hdr++;
            }

            free(pSection_hdr);
        }

    } while(0);

    return rval;
}

static int Elf2bin(FILE **elf, FILE **bin)
{
    int     rval = 0;
    FILE    *felf = *elf;
    FILE    *fbin = *bin;
    Elf32_Phdr  *pProg_hdrs = 0;

    do {
        Elf32_Ehdr  elf_hdr = {0};
        Elf32_Phdr  *pProg_hdr = 0;
        int         size = 0;

        fseek(felf, 0, SEEK_SET);
        size = fread(&elf_hdr, sizeof(Elf32_Ehdr), 1, felf);

        {   // Program Header Table
            uint32_t    phoffset = 0;   // program header table offset
            uint16_t    phnum = 0;      // number program section
            uint16_t    phentsize = 0;  // program header size

            phoffset = elf_hdr.e_phoff;
            phnum = elf_hdr.e_phnum;

            phentsize = elf_hdr.e_phentsize;
            fseek(felf, phoffset, SEEK_SET);

            if( !(pProg_hdrs = malloc(phentsize * phnum)) )
            {
                rval = -1;
                break;
            }

            fread((void*)pProg_hdrs, phentsize, phnum, felf);
            pProg_hdr = pProg_hdrs;

            printf("\n***** Program Headers:*****\n");
            printf("starting at offset       : %d\n", phoffset);
            printf("Number of program headers: %d\n", phnum);
            printf("Type           Offset   VirtAddr     PhysAddr   FileSiz     MemSiz     Flg\n");
            for(int num = 0; num < phnum; num++)
            {
                printf("x%08x    %8x   %8x   %8x  %8x    %8x  %8x\n",
                       pProg_hdr->p_type, pProg_hdr->p_offset,
                       pProg_hdr->p_vaddr, pProg_hdr->p_paddr,
                       pProg_hdr->p_filesz, pProg_hdr->p_memsz,
                       pProg_hdr->p_flags);
                pProg_hdr++;
            }
        }

        pProg_hdr = pProg_hdrs;
        for(int i = 0; i < elf_hdr.e_phnum; i++, pProg_hdr++)
        {
            if( pProg_hdr->p_type != PT_LOAD )
            {
                printf("\nNot PT_LOAD x%08x (prog section num= %d, size= %d)\n",
                       pProg_hdr->p_type, elf_hdr.e_phnum,  size);
                continue;
            }

            if( pProg_hdr->p_filesz )
            {
                uint8_t     *pBuf = 0;

                fseek(felf, pProg_hdr->p_offset, SEEK_SET);

                if( !(pBuf = malloc(pProg_hdr->p_filesz)) )
                {
                    rval = -1;
                    break;
                }

                // read_to_mem(pProg_hdr->p_vaddr, 1, pProg_hdr->p_filesz, felf);
                if( (size = fread(pBuf, 1, pProg_hdr->p_filesz, felf)) != pProg_hdr->p_filesz )
                {
                    printf("\nread p_vaddr err!\n");
                    printf("pProg_hdr->p_filesz= x%x  p_filesz= %d\n",
                           pProg_hdr->p_vaddr, pProg_hdr->p_filesz);
                    rval = -1;
                    break;
                }
                fwrite(pBuf, 1, size, fbin);

                free(pBuf);
            }

            /** bss section **/
            if( pProg_hdr->p_filesz < pProg_hdr->p_memsz )
            {
                uint8_t     *pBuf = 0;
                uint32_t    bss_len = pProg_hdr->p_memsz - pProg_hdr->p_filesz;

                if( !(pBuf = malloc(bss_len)) )
                {
                    rval = -1;
                    break;
                }
                memset(pBuf, 0x0, bss_len);
                fwrite(pBuf, 1, bss_len, fbin);
                free(pBuf);
            }
        }
    } while(0);


    if( pProg_hdrs )    free(pProg_hdrs);
    return rval;
}

int main(void)
{
    FILE    *felf = 0, *fbin = 0;
    char    *elf_name = "arm.elf";
    char    *bin_name = "out.bin";

    do {
        if( !(felf = fopen(elf_name, "rb")) )
        {
            printf("open %s fail \n", elf_name);
            break;
        }

        if( !(fbin = fopen(bin_name, "wb")) )
        {
            printf("open %s fail \n", elf_name);
            break;
        }

        if( check_elf_head(&felf) )
        {
            printf("check elf fail \n");
            break;
        }

        fseek(felf, 0, SEEK_END);

        Elf2bin(&felf, &fbin);

    } while(0);

    if( felf )  fclose(felf);
    if( fbin )  fclose(fbin);

    while(1)
        __asm("nop");
    return 0;
}

