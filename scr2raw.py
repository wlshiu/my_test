#!/usr/bin/env python

import sys
import argparse
import struct
import re

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", type=str, help="input scr file")
parser.add_argument("-o", "--output", type=str, help="output raw file")
args = parser.parse_args()

out_file = open(args.output, 'w+b')

opcode_read         = 0x55A0
opcode_write        = 0x55A1
opcode_delay        = 0x55A2
opcode_wait_status  = 0x55A3
opcode_mask_write   = 0x55A4
opcode_mask_read    = 0x55A5
opcode_or           = 0x55A6


for line in open(args.input):
    program_line = line.split("#")[0].replace(" ", "")

    if len(program_line) > 1:
        if "mask_write" in program_line:
            register = program_line.split("(")[1].split(",")[0]
            value    = program_line.split(",", 1)[1].split(",", 2)[0]
            mask     = program_line.split(",", 2)[2].split(")")[0]

            register = int(register, 0)
            mask     = int(mask, 0)

            raw = struct.pack("III", opcode_mask_write, register, mask)
            out_file.write(raw)
            continue


        if "write" in program_line:
            register = program_line.split("(")[1].split(",")[0]
            value    = program_line.split(",", 1)[1].split(")")[0]

            register = int(register, 0)
            value    = int(value, 0)

            raw = struct.pack("III", opcode_write, register, value)
            out_file.write(raw)
            continue

        if "mask_read" in program_line:
            register = program_line.split("(")[1].split(",")[0]
            mask     = program_line.split(",", 1)[1].split(")")[0]

            register = int(register, 0)
            mask     = int(mask, 0)

            raw = struct.pack("III", opcode_mask_read, register, mask)
            out_file.write(raw)
            continue

        if "read" in program_line:
            register = program_line.split("(")[1].split(")")[0]

            register = int(register, 0)

            raw = struct.pack("II", opcode_read, register)
            out_file.write(raw)
            continue

        if "delay" in program_line:
            ticks = program_line.split("(")[1].split(")")[0]

            ticks = int(ticks, 0)

            raw = struct.pack("II", opcode_delay, ticks)
            out_file.write(raw)
            continue

        if "or" in program_line:
            value    = program_line.split(",", 1)[1].split(")")[0]

            value     = int(value, 0)

            raw = struct.pack("II", opcode_or, value)
            out_file.write(raw)
            continue

        if "wait_status" in program_line:
            register = program_line.split("(")[1].split(",")[0]
            value    = program_line.split(",", 1)[1].split(",", 2)[0]
            mask     = program_line.split(",", 2)[2].split(")")[0]

            register = int(register, 0)
            value    = int(value, 0)
            mask     = int(mask, 0)

            raw = struct.pack("IIII", opcode_wait_status, register, value, mask)
            out_file.write(raw)
            continue

out_file.close()



