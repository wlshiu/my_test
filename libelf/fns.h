


#ifndef __fns_H_w9qZ7Kx8_lP6r_HWr9_sNur_uNhO5dWpEwc5__
#define __fns_H_w9qZ7Kx8_lP6r_HWr9_sNur_uNhO5dWpEwc5__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * print.c
 */
void printelf32ehdr(Elf32_Ehdr*, Fhdr*);
void printelf64ehdr(Elf64_Ehdr*, Fhdr*);
void printelf32shdr(Elf32_Shdr*, Fhdr*);
void printelf64shdr(Elf64_Shdr*, Fhdr*);
void printelf32phdr(Elf32_Phdr*, Fhdr*);
void printelf64phdr(Elf64_Phdr*, Fhdr*);

char* getstr(Fhdr*, uint32_t);

#ifdef __cplusplus
}
#endif

#endif
