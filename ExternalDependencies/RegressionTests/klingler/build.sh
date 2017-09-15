#!/bin/bash
# How to build.
gcc -ggdb test.c -o test-with-zdebug

objcopy --compress-debug-sections test-with-zdebug
#objcopy --decompress-debug-sections test-with-zdebug
