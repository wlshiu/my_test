
#include <stdio.h>
#include <assert.h>
//#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//#include <errno.h>
#include <stdbool.h>

#include "elf.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define DEBUG 1
//=============================================================================
//                  Macro Definition
//=============================================================================
#define debug(...)      do { if (DEBUG) printf("<debug>:"__VA_ARGS__); } while (0)

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct file_handle
{
    union {
        int     fd;
        FILE    *fp;
    };
} file_handle_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
//void disassemble(file_handle_t *pHFile, Elf32_Ehdr eh, Elf32_Shdr* sh_tbl);
//void disassemble64(file_handle_t *pHFile, Elf64_Ehdr eh, Elf64_Shdr* sh_tbl);

#if 0
void read_elf_header64(file_handle_t *pHFile, Elf64_Ehdr *elf_header);
bool is_ELF64(Elf64_Ehdr eh);
void print_elf_header64(Elf64_Ehdr elf_header);
void read_section_header_table64(file_handle_t *pHFile, Elf64_Ehdr eh, Elf64_Shdr sh_table[]);
char * read_section64(file_handle_t *pHFile, Elf64_Shdr sh);
void print_section_headers64(file_handle_t *pHFile, Elf64_Ehdr eh, Elf64_Shdr sh_table[]);
void print_symbol_table64(file_handle_t *pHFile,Elf64_Ehdr eh,Elf64_Shdr sh_table[],uint32_t symbol_table);
void print_symbols64(file_handle_t *pHFile, Elf64_Ehdr eh, Elf64_Shdr sh_table[]);
void save_text_section64(file_handle_t *pHFile, Elf64_Ehdr eh, Elf64_Shdr sh_table[]);
#endif

void
read_elf_header(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pElf_header);


bool is_ELF(Elf32_Ehdr *pEh);


void
print_elf_header(
    Elf32_Ehdr  *pElf_header);


void
read_section_header_table(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table);


uint8_t*
read_section(
    file_handle_t   *pHFile,
    Elf32_Shdr      *pSh);


void
print_section_headers(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table);


void
print_symbol_table(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table,
    uint32_t        symbol_table);


void
print_symbols(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table);


void
save_text_section(
    file_handle_t   *pHFile,
    Elf32_Ehdr      *pEh,
    Elf32_Shdr      *pSh_table);


bool is_64bits(Elf32_Ehdr *pEh);

