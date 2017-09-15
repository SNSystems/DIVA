#!/bin/sh
../dwarfdump -a *.elf >junk.base
grep 'c:/programs/' <junk.base >junk.hasout
r=$?
if [ $r -ne 0 ]
then
   echo "FAIL sandnes2 missing path concat"
   exit 1
fi
../dwarfdumpW -a *.elf >junk.W
grep 'c:/temp/' <junk.W >junk.noout
r=$?
if [ $r -ne 0 ]
then
   echo "FAIL sandnes2 Failed to transform name"
   exit 1
fi
# success!
exit 0

