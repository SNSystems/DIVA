#!/bin/sh
libdw=$1
h="-I$libdw/libdwarf"
l="-L$libdw/libdwarf"
libs="-ldwarf -lelf"

if [ -f /usr/include/zlib.h ]
then
  cc $h cutest.c $l -o cutest $libs -lz
else
  cc $h cutest.c $l -o cutest $libs
fi
./cutest cutestobj.save
r=$?
if [ $r -ne 0 ]
then
   echo FAIL cutest, interface did not work.
   exit 1
fi
exit 0
