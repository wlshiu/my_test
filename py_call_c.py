#!/usr/bin/env python3

import ctypes
import time
from random import seed
from random import randint

seed(int(time.time()))
dll = ctypes.cdll.LoadLibrary
lib = dll('./libpycall.so') # c-code
a = randint(1, 1000)
b = randint(1, 1000)

print("py layer: a= %d, b= %d\n" % (int(a), int(b)))
lib.foo(int(a), int(b))
