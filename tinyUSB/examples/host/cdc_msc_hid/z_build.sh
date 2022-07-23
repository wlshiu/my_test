#!/bin/bash

make clean
make BOARD=lpcxpresso18s37 SPEED=full all

rm -fr ./_build
