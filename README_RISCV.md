# RISC-V Overlay Example

This repo is a RISC-V overlay example.  This aims to be a quick reference to migrate overaly from ARM
toolchain to RISC-V GNU toolchain.

The example is modified from the 
[ARM overlay example](https://developer.arm.com/documentation/dui0773/i/Mapping-code-and-data-to-target-memory/Manual-overlay-support/Writing-an-overlay-manager-for-manually-placed-overlays).


| | ARM | GNU |
|-|-----|-----|
|Define output binary sections|scatter file|linker script|
|Define overlay section|Define `OVERLAY` region in scatter file|Define `OVERLAY` section in linker script|
|Patch symbol/Function wrapper|`$Super$$foo` and `$Sub$$foo` C functions|ld option `--wrap=<foo>` + `__wrap_foo` C function|


## Build

```
$ git clone https://github.com/sifive/riscv-overlay-example.git
$ cd riscv-overlay-example/src
$ make
```

