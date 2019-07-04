#include "elf-parser.h"

#define err(str, ...)       do{ printf("[%s:%d] " str, __func__, __LINE__, ## __VA_ARGS__); while(1); }while(0)

/* Main entry point of elf-parser */
int main(int argc, char *argv[])
{
    file_handle_t   hFile = {0};

    do {
        Elf32_Ehdr      eh = {0};  /* elf-header is fixed size */

        if( argc != 2 )
        {
            err("Usage: %s <ELF-file>\n", argv[0]);
            break;
        }

        if( !(hFile.fp = fopen(argv[1], "rb")) )
        {
            err("open %s fail \n", argv[1]);
            break;
        }

        read_elf_header(&hFile, &eh);

        if( !is_ELF(&eh) )  break;

        if( !is_64bits(&eh) )
        {
            Elf32_Shdr  *pSh_tbl = 0; // section-header table is variable size

            print_elf_header(&eh);

            // Section header table
            if( !(pSh_tbl = malloc(eh.e_shentsize * eh.e_shnum)) )
            {
                err("malloc %d fail \n", eh.e_shentsize * eh.e_shnum);
                break;
            }

            read_section_header_table(&hFile, &eh, pSh_tbl);
            print_section_headers(&hFile, &eh, pSh_tbl);

            /**
             * Symbol tables :
             * pSh_tbl[i].sh_type
             * |`- SHT_SYMTAB
             *  `- SHT_DYNSYM
             */
//            print_symbols(&hFile, &eh, pSh_tbl);

//            save_text_section(&hFile, &eh, pSh_tbl);

            {   // output bin file
                FILE        *fout = 0;
                char        bin_name[64] = {0};
                uint8_t     *pSh_str = 0;

                pSh_str = read_section(&hFile, &pSh_tbl[eh.e_shstrndx]);
                if( !pSh_str )      break;

                snprintf(bin_name, 64, "%s.bin", argv[1]);
                if( !(fout = fopen(bin_name, "wb")) )
                {
                    err("open %s fail \n", bin_name);
                    break;
                }

                for(int i = 0; i < eh.e_phnum; i++)
                {
                    uint8_t     *pBuf = 0;

                    if( !(pBuf = malloc(pSh_tbl[i + 1].sh_size)) )
                    {
                        continue;
                    }

                    fseek(hFile.fp, pSh_tbl[i + 1].sh_offset, SEEK_SET);
                    fread(pBuf, 1, pSh_tbl[i + 1].sh_size, hFile.fp);

                    fwrite(pBuf, 1, pSh_tbl[i + 1].sh_size, fout);
                    free(pBuf);
                }

                if( pSh_str )   free(pSh_str);

                fclose(fout);
            }

            if( pSh_tbl )   free(pSh_tbl);

        }
        else
        {
    #if 0   // TODO: 64 bits elf file
            Elf64_Ehdr eh64;    /* elf-header is fixed size */
            Elf64_Shdr *sh_tbl; /* section-header table is variable size */

            read_elf_header64(fd, &eh64);
            print_elf_header64(eh64);

            /* Section header table :  */
            sh_tbl = malloc(eh64.e_shentsize * eh64.e_shnum);
            if(!sh_tbl) {
                printf("Failed to allocate %d bytes\n",
                       (eh64.e_shentsize * eh64.e_shnum));
            }
            read_section_header_table64(fd, eh64, sh_tbl);
            print_section_headers64(fd, eh64, sh_tbl);

            /* Symbol tables :
             * sh_tbl[i].sh_type
             * |`- SHT_SYMTAB
             *  `- SHT_DYNSYM
             */
            print_symbols64(fd, eh64, sh_tbl);

            /* Save .text section as text.S
            */
            save_text_section64(fd, eh64, sh_tbl);

            /* Disassemble .text section
             * Logs asm instructions to stdout
             * Currently supports ARMv7
             */
            disassemble64(fd, eh64, sh_tbl);
    #endif
        }
    } while(0);

    if( hFile.fp )    fclose(hFile.fp);

    return 0;
}
