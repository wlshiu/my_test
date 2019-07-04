

#include "elf-parser.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#if defined(WIN32)
    #define NC
    #define RED
    #define LIGHT_RED
    #define GREEN
    #define LIGHT_GREEN
    #define BLUE
    #define LIGHT_BLUE
    #define DARY_GRAY
    #define CYAN
    #define LIGHT_CYAN
    #define PURPLE
    #define LIGHT_PURPLE
    #define BROWN
    #define YELLOW
    #define LIGHT_GRAY
    #define WHITE

#else
    #define NC          "\033[m"
    #define RED         "\033[0;32;31m"
    #define LIGHT_RED   "\033[1;31m"
    #define GREEN       "\033[0;32;32m"
    #define LIGHT_GREEN "\033[1;32m"
    #define BLUE        "\033[0;32;34m"
    #define LIGHT_BLUE  "\033[1;34m"
    #define DARY_GRAY   "\033[1;30m"
    #define CYAN        "\033[0;36m"
    #define LIGHT_CYAN  "\033[1;36m"
    #define PURPLE      "\033[0;35m"
    #define LIGHT_PURPLE "\033[1;35m"
    #define BROWN       "\033[0;33m"
    #define YELLOW      "\033[1;33m"
    #define LIGHT_GRAY  "\033[0;37m"
    #define WHITE       "\033[1;37m"
#endif

//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, ...)                   do{ fprintf(stdout, "[%s:%d] " str, __func__, __LINE__, ## __VA_ARGS__); while(1); }while(0)
#define msg(str, ...)                   do{ fprintf(stdout, str, ##__VA_ARGS__); fflush(stdout); } while(0)
#define msg_color(color, str, ...)      do{ fprintf(stdout, color str NC, ##__VA_ARGS__); fflush(stdout); }while(0)
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

#if 0
void read_elf_header64(int32_t fd, Elf64_Ehdr *elf_header)
{
    assert(elf_header != NULL);
    assert(lseek(fd, (off_t)0, SEEK_SET) == (off_t)0);
    assert(read(fd, (void *)elf_header, sizeof(Elf64_Ehdr)) == sizeof(Elf64_Ehdr));
}


bool is_ELF64(Elf64_Ehdr eh)
{
    /* ELF magic bytes are 0x7f,'E','L','F'
     * Using  octal escape sequence to represent 0x7f
     */
    if(!strncmp((char*)eh.e_ident, "\177ELF", 4)) {
        msg("ELFMAGIC \t= ELF\n");
        /* IS a ELF file */
        return 1;
    } else {
        msg("ELFMAGIC mismatch!\n");
        /* Not ELF file */
        return 0;
    }
}

void print_elf_header64(Elf64_Ehdr elf_header)
{

    /* Storage capacity class */
    msg("Storage class\t= ");
    switch(elf_header.e_ident[EI_CLASS])
    {
        case ELFCLASS32:
            msg("32-bit objects\n");
            break;

        case ELFCLASS64:
            msg("64-bit objects\n");
            break;

        default:
            msg("INVALID CLASS\n");
            break;
    }

    /* Data Format */
    msg("Data format\t= ");
    switch(elf_header.e_ident[EI_DATA])
    {
        case ELFDATA2LSB:
            msg("2's complement, little endian\n");
            break;

        case ELFDATA2MSB:
            msg("2's complement, big endian\n");
            break;

        default:
            msg("INVALID Format\n");
            break;
    }

    /* OS ABI */
    msg("OS ABI\t\t= ");
    switch(elf_header.e_ident[EI_OSABI])
    {
        case ELFOSABI_SYSV:
            msg("UNIX System V ABI\n");
            break;

        case ELFOSABI_HPUX:
            msg("HP-UX\n");
            break;

        case ELFOSABI_NETBSD:
            msg("NetBSD\n");
            break;

        case ELFOSABI_LINUX:
            msg("Linux\n");
            break;

        case ELFOSABI_SOLARIS:
            msg("Sun Solaris\n");
            break;

        case ELFOSABI_AIX:
            msg("IBM AIX\n");
            break;

        case ELFOSABI_IRIX:
            msg("SGI Irix\n");
            break;

        case ELFOSABI_FREEBSD:
            msg("FreeBSD\n");
            break;

        case ELFOSABI_TRU64:
            msg("Compaq TRU64 UNIX\n");
            break;

        case ELFOSABI_MODESTO:
            msg("Novell Modesto\n");
            break;

        case ELFOSABI_OPENBSD:
            msg("OpenBSD\n");
            break;

        case ELFOSABI_ARM_AEABI:
            msg("ARM EABI\n");
            break;

        case ELFOSABI_ARM:
            msg("ARM\n");
            break;

        case ELFOSABI_STANDALONE:
            msg("Standalone (embedded) app\n");
            break;

        default:
            msg("Unknown (0x%x)\n", elf_header.e_ident[EI_OSABI]);
            break;
    }

    /* ELF filetype */
    msg("Filetype \t= ");
    switch(elf_header.e_type)
    {
        case ET_NONE:
            msg("N/A (0x0)\n");
            break;

        case ET_REL:
            msg("Relocatable\n");
            break;

        case ET_EXEC:
            msg("Executable\n");
            break;

        case ET_DYN:
            msg("Shared Object\n");
            break;
        default:
            msg("Unknown (0x%x)\n", elf_header.e_type);
            break;
    }

    /* ELF Machine-id */
    msg("Machine\t\t= ");
    switch(elf_header.e_machine)
    {
        case EM_NONE:
            msg("None (0x0)\n");
            break;

        case EM_386:
            msg("INTEL x86 (0x%x)\n", EM_386);
            break;

        case EM_X86_64:
            msg("AMD x86_64 (0x%x)\n", EM_X86_64);
            break;

        case EM_AARCH64:
            msg("AARCH64 (0x%x)\n", EM_AARCH64);
            break;

        default:
            msg(" 0x%x\n", elf_header.e_machine);
            break;
    }

    /* Entry point */
    msg("Entry point\t= 0x%08lx\n", elf_header.e_entry);

    /* ELF header size in bytes */
    msg("ELF header size\t= 0x%08x\n", elf_header.e_ehsize);

    /* Program Header */
    msg("\nProgram Header\t= ");
    msg("0x%08lx\n", elf_header.e_phoff);       /* start */
    msg("\t\t  %d entries\n", elf_header.e_phnum);  /* num entry */
    msg("\t\t  %d bytes\n", elf_header.e_phentsize);    /* size/entry */

    /* Section header starts at */
    msg("\nSection Header\t= ");
    msg("0x%08lx\n", elf_header.e_shoff);       /* start */
    msg("\t\t  %d entries\n", elf_header.e_shnum);  /* num entry */
    msg("\t\t  %d bytes\n", elf_header.e_shentsize);    /* size/entry */
    msg("\t\t  0x%08x (string table offset)\n", elf_header.e_shstrndx);

    /* File flags (Machine specific)*/
    msg("\nFile flags \t= 0x%08x\n", elf_header.e_flags);

    /* ELF file flags are machine specific.
     * INTEL implements NO flags.
     * ARM implements a few.
     * Add support below to parse ELF file flags on ARM
     */
    int32_t ef = elf_header.e_flags;
    msg("\t\t  ");

    if(ef & EF_ARM_RELEXEC)
        msg(",RELEXEC ");

    if(ef & EF_ARM_HASENTRY)
        msg(",HASENTRY ");

    if(ef & EF_ARM_INTERWORK)
        msg(",INTERWORK ");

    if(ef & EF_ARM_APCS_26)
        msg(",APCS_26 ");

    if(ef & EF_ARM_APCS_FLOAT)
        msg(",APCS_FLOAT ");

    if(ef & EF_ARM_PIC)
        msg(",PIC ");

    if(ef & EF_ARM_ALIGN8)
        msg(",ALIGN8 ");

    if(ef & EF_ARM_NEW_ABI)
        msg(",NEW_ABI ");

    if(ef & EF_ARM_OLD_ABI)
        msg(",OLD_ABI ");

    if(ef & EF_ARM_SOFT_FLOAT)
        msg(",SOFT_FLOAT ");

    if(ef & EF_ARM_VFP_FLOAT)
        msg(",VFP_FLOAT ");

    if(ef & EF_ARM_MAVERICK_FLOAT)
        msg(",MAVERICK_FLOAT ");

    msg("\n");

    /* MSB of flags conatins ARM EABI version */
    msg("ARM EABI\t= Version %d\n", (ef & EF_ARM_EABIMASK)>>24);

    msg("\n");  /* End of ELF header */

}

void read_section_header_table64(int32_t fd, Elf64_Ehdr eh, Elf64_Shdr sh_table[])
{
    uint32_t i;

    assert(lseek(fd, (off_t)eh.e_shoff, SEEK_SET) == (off_t)eh.e_shoff);

    for(i=0; i<eh.e_shnum; i++) {
        assert(read(fd, (void *)&sh_table[i], eh.e_shentsize)
                == eh.e_shentsize);
    }

}

char * read_section64(int32_t fd, Elf64_Shdr sh)
{
    char* buff = malloc(sh.sh_size);
    if(!buff) {
        msg("%s:Failed to allocate %ldbytes\n",
                __func__, sh.sh_size);
    }

    assert(buff != NULL);
    assert(lseek(fd, (off_t)sh.sh_offset, SEEK_SET) == (off_t)sh.sh_offset);
    assert(read(fd, (void *)buff, sh.sh_size) == sh.sh_size);

    return buff;
}

void print_section_headers64(int32_t fd, Elf64_Ehdr eh, Elf64_Shdr sh_table[])
{
    uint32_t i;
    char* sh_str;   /* section-header string-table is also a section. */

    /* Read section-header string-table */
    debug("eh.e_shstrndx = 0x%x\n", eh.e_shstrndx);
    sh_str = read_section64(fd, sh_table[eh.e_shstrndx]);

    msg("========================================");
    msg("========================================\n");
    msg(" idx offset     load-addr  size       algn"
            " flags      type       section\n");
    msg("========================================");
    msg("========================================\n");

    for(i=0; i<eh.e_shnum; i++) {
        msg(" %03d ", i);
        msg("0x%08lx ", sh_table[i].sh_offset);
        msg("0x%08lx ", sh_table[i].sh_addr);
        msg("0x%08lx ", sh_table[i].sh_size);
        msg("%4ld ", sh_table[i].sh_addralign);
        msg("0x%08lx ", sh_table[i].sh_flags);
        msg("0x%08x ", sh_table[i].sh_type);
        msg("%s\t", (sh_str + sh_table[i].sh_name));
        msg("\n");
    }
    msg("========================================");
    msg("========================================\n");
    msg("\n");  /* end of section header table */
}

void print_symbol_table64(int32_t fd,
        Elf64_Ehdr eh,
        Elf64_Shdr sh_table[],
        uint32_t symbol_table)
{

    char *str_tbl;
    Elf64_Sym* sym_tbl;
    uint32_t i, symbol_count;

    sym_tbl = (Elf64_Sym*)read_section64(fd, sh_table[symbol_table]);

    /* Read linked string-table
     * Section containing the string table having names of
     * symbols of this section
     */
    uint32_t str_tbl_ndx = sh_table[symbol_table].sh_link;
    debug("str_table_ndx = 0x%x\n", str_tbl_ndx);
    str_tbl = read_section64(fd, sh_table[str_tbl_ndx]);

    symbol_count = (sh_table[symbol_table].sh_size/sizeof(Elf64_Sym));
    msg("%d symbols\n", symbol_count);

    for(i=0; i< symbol_count; i++) {
        msg("0x%08lx ", sym_tbl[i].st_value);
        msg("0x%02x ", ELF32_ST_BIND(sym_tbl[i].st_info));
        msg("0x%02x ", ELF32_ST_TYPE(sym_tbl[i].st_info));
        msg("%s\n", (str_tbl + sym_tbl[i].st_name));
    }
}

void print_symbols64(int32_t fd, Elf64_Ehdr eh, Elf64_Shdr sh_table[])
{
    uint32_t i;

    for(i=0; i<eh.e_shnum; i++) {
        if ((sh_table[i].sh_type==SHT_SYMTAB)
                || (sh_table[i].sh_type==SHT_DYNSYM)) {
            msg("\n[Section %03d]", i);
            print_symbol_table64(fd, eh, sh_table, i);
        }
    }
}

void save_text_section64(int32_t fd, Elf64_Ehdr eh, Elf64_Shdr sh_table[])
{
    uint32_t i;
    int32_t fd2;    /* to write text.S in current directory */
    char* sh_str;   /* section-header string-table is also a section. */
    char* buf;  /* buffer to hold contents of the .text section */

    /*   */
    char *pwd = getcwd(NULL, (size_t)NULL);
    msg("%s\n", pwd);
    pwd = realloc(pwd, strlen(pwd)+8);
    strcat(pwd,"/text.S");
    msg("%s\n", pwd);

    /* Read section-header string-table */
    debug("eh.e_shstrndx = 0x%x\n", eh.e_shstrndx);
    sh_str = read_section64(fd, sh_table[eh.e_shstrndx]);

    for(i=0; i<eh.e_shnum; i++) {
        if(!strcmp(".text", (sh_str + sh_table[i].sh_name))) {
            msg("Found section\t\".text\"\n");
            msg("at offset\t0x%08lx\n", sh_table[i].sh_offset);
            msg("of size\t\t0x%08lx\n", sh_table[i].sh_size);

            break;
        }
    }

    assert(lseek(fd, sh_table[i].sh_offset, SEEK_SET)==sh_table[i].sh_offset);
    buf = malloc(sh_table[i].sh_size);
    if(!buf) {
        msg("Failed to allocate %ldbytes!!\n", sh_table[i].sh_size);
        goto EXIT;
    }
    assert(read(fd, buf, sh_table[i].sh_size)==sh_table[i].sh_size);
    fd2 = open(pwd, O_RDWR|O_SYNC|O_CREAT);
    write(fd2, buf, sh_table[i].sh_size);
    fsync(fd2);

EXIT:
    close(fd2);
    free(pwd);

}
#endif

void
read_elf_header(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pElf_header)
{
    do {
        long    len = 0;

        if( !pElf_header )      break;

        fseek(pHFile->fp, 0, SEEK_SET);
        len = fread((void*)pElf_header, 1, sizeof(Elf32_Ehdr), pHFile->fp);
        if( len != sizeof(Elf32_Ehdr) )
            err("read fail %ld/%ld\n", len, sizeof(Elf32_Ehdr));
    } while(0);
    return;
}


bool is_ELF(Elf32_Ehdr *pEh)
{
    bool    rval = false;
    do {
        /**
         * ELF magic bytes are 0x7f,'E','L','F'
         * Using  octal escape sequence to represent 0x7f
         */
        if( strncmp((char*)pEh->e_ident, "\177ELF", 4) )
        {
            msg("ELFMAGIC mismatch!\n"); // not a ELF file
            break;
        }

        msg("ELFMAGIC \t= ELF\n");  // it is a ELF file
        rval = true;
    } while(0);
    return rval;
}

void print_elf_header(Elf32_Ehdr *pElf_header)
{
    // Storage capacity class
    msg("Storage class\t= ");
    switch( pElf_header->e_ident[EI_CLASS] )
    {
        case ELFCLASS32:    msg("32-bit objects\n");    break;
        case ELFCLASS64:    msg("64-bit objects\n");    break;
        default:            msg("INVALID CLASS\n");     break;
    }

    // Data Format
    msg("Data format\t= ");
    switch(pElf_header->e_ident[EI_DATA])
    {
        case ELFDATA2LSB:   msg("2's complement, little endian\n"); break;
        case ELFDATA2MSB:   msg("2's complement, big endian\n");    break;
        default:            msg("INVALID Format\n");                break;
    }

    // OS ABI
    msg("OS ABI\t\t= ");
    switch(pElf_header->e_ident[EI_OSABI])
    {
        case ELFOSABI_SYSV:     msg("UNIX System V ABI\n"); break;
        case ELFOSABI_HPUX:     msg("HP-UX\n");             break;
        case ELFOSABI_NETBSD:   msg("NetBSD\n");            break;
        case ELFOSABI_LINUX:    msg("Linux\n");             break;
        case ELFOSABI_SOLARIS:  msg("Sun Solaris\n");       break;
        case ELFOSABI_AIX:      msg("IBM AIX\n");           break;
        case ELFOSABI_IRIX:     msg("SGI Irix\n");          break;
        case ELFOSABI_FREEBSD:  msg("FreeBSD\n");           break;
        case ELFOSABI_TRU64:    msg("Compaq TRU64 UNIX\n"); break;
        case ELFOSABI_MODESTO:  msg("Novell Modesto\n");    break;
        case ELFOSABI_OPENBSD:  msg("OpenBSD\n");           break;
        case ELFOSABI_ARM_AEABI: msg("ARM EABI\n");         break;
        case ELFOSABI_ARM:      msg("ARM\n");               break;
        case ELFOSABI_STANDALONE: msg("Standalone (embedded) app\n");   break;
        default:
            msg("Unknown (0x%x)\n", pElf_header->e_ident[EI_OSABI]);
            break;
    }

    // ELF filetype
    msg("Filetype \t= ");
    switch(pElf_header->e_type)
    {
        case ET_NONE:   msg("N/A (0x0)\n");     break;
        case ET_REL:    msg("Relocatable\n");   break;
        case ET_EXEC:   msg("Executable\n");    break;
        case ET_DYN:    msg("Shared Object\n"); break;
        default:
            msg("Unknown (0x%x)\n", pElf_header->e_type);
            break;
    }

    // ELF Machine-id
    msg("Machine\t\t= ");
    switch(pElf_header->e_machine)
    {
        case EM_NONE:   msg("None (0x0)\n");                break;
        case EM_386:    msg("INTEL x86 (0x%x)\n", EM_386);  break;
        case EM_ARM:    msg("ARM (0x%x)\n", EM_ARM);        break;
        default:
            msg(" 0x%x\n", pElf_header->e_machine);
            break;
    }

    // Entry point
    msg("Entry point\t= 0x%08x\n", pElf_header->e_entry);

    // ELF header size in bytes
    msg("ELF header size\t= 0x%08x\n", pElf_header->e_ehsize);

    // Program Header
    msg("\nProgram Header\t= ");
    msg("0x%08x\n", pElf_header->e_phoff);              // start
    msg("\t\t  %d entries\n", pElf_header->e_phnum);    // num entry
    msg("\t\t  %d bytes\n", pElf_header->e_phentsize);  // size/entry

    // Section header starts at
    msg("\nSection Header\t= ");
    msg("0x%08x\n", pElf_header->e_shoff);              // start
    msg("\t\t  %d entries\n", pElf_header->e_shnum);    // num entry
    msg("\t\t  %d bytes\n", pElf_header->e_shentsize);  // size/entry
    msg("\t\t  0x%08x (string table offset)\n", pElf_header->e_shstrndx);

    // File flags (Machine specific)
    msg("\nFile flags \t= 0x%08x\n", pElf_header->e_flags);

    /**
     * ELF file flags are machine specific.
     * INTEL implements NO flags.
     * ARM implements a few.
     * Add support below to parse ELF file flags on ARM
     */
    int     ef = pElf_header->e_flags;
    msg("\t\t  ");

    if(ef & EF_ARM_RELEXEC)     msg(",RELEXEC ");
    if(ef & EF_ARM_HASENTRY)    msg(",HASENTRY ");
    if(ef & EF_ARM_INTERWORK)   msg(",INTERWORK ");
    if(ef & EF_ARM_APCS_26)     msg(",APCS_26 ");
    if(ef & EF_ARM_APCS_FLOAT)  msg(",APCS_FLOAT ");
    if(ef & EF_ARM_PIC)         msg(",PIC ");
    if(ef & EF_ARM_ALIGN8)      msg(",ALIGN8 ");
    if(ef & EF_ARM_NEW_ABI)     msg(",NEW_ABI ");
    if(ef & EF_ARM_OLD_ABI)     msg(",OLD_ABI ");
    if(ef & EF_ARM_SOFT_FLOAT)  msg(",SOFT_FLOAT ");
    if(ef & EF_ARM_VFP_FLOAT)   msg(",VFP_FLOAT ");
    if(ef & EF_ARM_MAVERICK_FLOAT)  msg(",MAVERICK_FLOAT ");

    msg("\n");

    // MSB of flags conatins ARM EABI version
    msg("ARM EABI\t= Version %d\n\n", (ef & EF_ARM_EABIMASK) >> 24);
    return;
}

void
read_section_header_table(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table)
{
    int     rva = 0;
    do {
        fseek(pHFile->fp, pEh->e_shoff, SEEK_SET);
        for(int i = 0; i < pEh->e_shnum; i++)
        {
            long    len = 0;
            len = fread((void*)(pSh_table + i), 1, pEh->e_shentsize, pHFile->fp);
            if( len != pEh->e_shentsize )
                err("read fail %ld/%ld\n", len, pEh->e_shentsize);
        }
    } while(0);
    return;
}

uint8_t*
read_section(
    file_handle_t   *pHFile,
    Elf32_Shdr      *pSh)
{
    uint8_t     *pBuf = 0;
    do {
        long    len = 0;
        if( !(pBuf = malloc(pSh->sh_size)) )
        {
            err("malloc %d fail \n", pSh->sh_size);
            break;
        }

        fseek(pHFile->fp, pSh->sh_offset, SEEK_SET);
        len = fread((void*)pBuf, 1, pSh->sh_size, pHFile->fp);
        if( len != pSh->sh_size )
            err("read fail %ld/%ld\n", len, pSh->sh_size);

    } while(0);
    return pBuf;
}

void
print_section_headers(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table)
{
    uint8_t     *pSh_str = 0;   // section-header string-table is also a section.

    do {
        /* Read section-header string-table */
        debug("eh.e_shstrndx = 0x%x\n", pEh->e_shstrndx);
        pSh_str = read_section(pHFile, &pSh_table[pEh->e_shstrndx]);

        msg("============================================\n");
        msg(" idx   offset     load-addr  size         algn  flags      type        section\n");
        msg("============================================\n");

        for(int i = 0; i < pEh->e_shnum; i++)
        {
            msg(" %03d   0x%08x 0x%08x 0x%08x %4d    0x%08x 0x%08x %s\t\n",
                i,
                pSh_table[i].sh_offset,
                pSh_table[i].sh_addr,
                pSh_table[i].sh_size,
                pSh_table[i].sh_addralign,
                pSh_table[i].sh_flags,
                pSh_table[i].sh_type,
                (pSh_str + pSh_table[i].sh_name));
        }
        msg("============================================\n\n");
    } while(0);

    if( pSh_str )   free(pSh_str);
    return;
}

void
print_symbol_table(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table,
    uint32_t        symbol_table)
{
    uint8_t     *pStr_tbl = 0;
    Elf32_Sym   *pSym_tbl = 0;

    do {
        uint32_t    symbol_count;

        pSym_tbl = (Elf32_Sym*)read_section(pHFile, &pSh_table[symbol_table]);

        /**
         * Read linked string-table
         * Section containing the string table having names of
         * symbols of this section
         */
        uint32_t    str_tbl_ndx = pSh_table[symbol_table].sh_link;
        debug("str_table_ndx = 0x%x\n", str_tbl_ndx);
        pStr_tbl = read_section(pHFile, &pSh_table[str_tbl_ndx]);

        symbol_count = (pSh_table[symbol_table].sh_size/sizeof(Elf32_Sym));
        msg("%d symbols\n", symbol_count);

        for(int i = 0; i < symbol_count; i++)
        {
            msg("0x%08x 0x%02x 0x%02x %s\n",
                pSym_tbl[i].st_value,
                ELF32_ST_BIND(pSym_tbl[i].st_info),
                ELF32_ST_TYPE(pSym_tbl[i].st_info),
                (pStr_tbl + pSym_tbl[i].st_name));
        }
    } while(0);

    if( pStr_tbl )  free(pStr_tbl);
    if( pSym_tbl )  free(pSym_tbl);

    return;
}

void
print_symbols(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table)
{
    for(int i = 0; i < pEh->e_shnum; i++)
    {
        if( (pSh_table[i].sh_type == SHT_SYMTAB) ||
            (pSh_table[i].sh_type == SHT_DYNSYM) )
        {
            msg("\n[Section %03d]", i);
            print_symbol_table(pHFile, pEh, pSh_table, i);
        }
    }
}

void
save_text_section(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table)
{
    uint8_t     *pSh_str = 0;   /* section-header string-table is also a section. */
    uint8_t     *pBuf_txt = 0;

    do {
        int     i = 0;
        long    len = 0;

        pSh_str = read_section(pHFile, &pSh_table[pEh->e_shstrndx]);
        if( !pSh_str )
        {
            err("no data \n");
            break;
        }

        for(i = 0; i < pEh->e_shnum; i++)
        {
            if( !strcmp(".text", (pSh_str + pSh_table[i].sh_name)) )
            {
                msg("Found section\t\".text\"\n");
                msg("at offset\t0x%08x\n", pSh_table[i].sh_offset);
                msg("of size\t\t0x%08x\n", pSh_table[i].sh_size);

                break;
            }
        }

        fseek(pHFile->fp, pSh_table[i].sh_offset, SEEK_SET);

        if( !(pBuf_txt = malloc(pSh_table[i].sh_size)) )
        {
            err("malloc %d fail \n", pSh_table[i].sh_size);
            break;
        }

        len = fread(pBuf_txt, 1, pSh_table[i].sh_size, pHFile->fp);
        if( len != pSh_table[i].sh_size )
            err("read fail %ld/%ld\n", len, pSh_table[i].sh_size);

        {
            FILE    *fout = 0;
            if( !(fout = fopen("./text.S", "wb")) )
            {
                err("open %s fail \n", "text.S");
                break;
            }

            fwrite(pBuf_txt, 1, pSh_table[i].sh_size, fout);
            fclose(fout);
        }
    } while(0);

    if( pSh_str )   free(pSh_str);
    if( pBuf_txt )  free(pBuf_txt);

    return;
}

bool is_64bits(Elf32_Ehdr *pEh)
{
    return (pEh->e_ident[EI_CLASS] == ELFCLASS64);
}
