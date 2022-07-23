#!/bin/bash

make clean
make BOARD=lpcxpresso18s37 SPEED=full V=1 all 

# rm -fr ./_build
# arm-none-eabi-readelf --debug-dump=macro
# arm-none-eabi-objdump --dwarf=macro
