#!/bin/bash

if [[ -d "out" ]]; then
    rm -fr out
fi

root_dir=$(pwd)

mkdir out

cd out

# compile assembly
# arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Wall -ffunction-sections -g -O0 -c -DSTM32F429ZI -DSTM32F429xx ${root_dir}/startup_stm32f429xx.s
# arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -g -nostartfiles -Wl,-Map=AssemblyCode.map -O0 -Wl,--gc-sections -Wl,-T${root_dir}/stm32f429zi_flash.ld -g -o AssemblyCode.elf startup_stm32f429xx.o

# comile mix c and assembly
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -g -Wall -O0 -c -DSTM32F429ZI -DSTM32F429xx ${root_dir}/main.c ${root_dir}/startup_stm32f429xx.s
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -g -Wl,-Map=App.map -O0 -Wl,--gc-sections -Wl,-T${root_dir}/stm32f429zi_flash.ld -g -o App.elf startup_stm32f429xx.o main.o


# generate disassembly
arm-none-eabi-objdump -d -S App.elf > Code.disa
