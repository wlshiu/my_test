#!/bin/bash -
# Copyright (c) 2019, All Rights Reserved.
# @file    z_verify_prebuild_lib.sh
# @author  Wei-Lun Hsu
# @version 0.1

set -e

dev_path=/dev/ttyS0
baudrate=115200

sudo chmod 666 ${dev_path}

./out/bin/UartTerminal ${dev_path} ${baudrate}

