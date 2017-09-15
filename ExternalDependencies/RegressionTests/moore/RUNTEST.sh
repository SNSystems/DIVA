#!/bin/sh
# This is not really part of the test, it simply
# lets us check for the error in the current gcc.
gcc -c -gdwarf-2 simplec.c
dwarfdump -l simplec.o
dwarfdump -l -v -v -v simplec.o
