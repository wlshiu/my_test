#!/bin/bash

set -e


gcc gen_app.c gen_img.c gen_prebuild.c gen_toolchain.c gen_dev.c gen_ld.c gen_test.c main.c -o gen_kconfig
