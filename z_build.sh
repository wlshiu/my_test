#!/bin/bash

set -e 

gcc shell.c sh_user_io.c win_platform.c -o test_shell -I./
