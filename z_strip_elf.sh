#!/bin/bash

set -e

Red='\e[0;31m'
Yellow='\e[1;33m'
Green='\e[0;32m'
Cyan='\e[0;36m'
NC='\e[0m' # No Color

help()
{
    echo -e "${Yellow}'$0' is used to strip dummy section of elf file"
    echo -e "usage: $0 [elf] [new-elf]${NC}"
    exit 1;
}

if [ $# != 2 ]; then
    help
fi

OBJCOPY=arm-none-eabi-objcopy

elf_input=$1
elf_output=$2

${OBJCOPY} -S -I elf32-little -R .comment ${elf_input} ${elf_output}
