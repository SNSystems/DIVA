#!/bin/sh
if [ -f /usr/include/zlib.h ]
then
  cc -I $1/libdwarf -DNEW frame_test.c ../libdwarf.a -lelf -lz -o frame_test1
else
  cc -I $1/libdwarf -DNEW frame_test.c ../libdwarf.a -lelf -o frame_test1
fi
./frame_test1
if [  $? -ne 0 ]
then
  echo FAIL frame CFA reg new
  exit 1
fi

if [ -f /usr/include/zlib.h ]
then
  cc -I $1/libdwarf -DOLD frame_test.c ../libdwoldframecol.a -lelf -lz -o frame_test2
else
  cc -I $1/libdwarf -DOLD frame_test.c ../libdwoldframecol.a -lelf -o frame_test2
fi
./frame_test2
if [  $? -ne 0 ]
then
  echo FAIL frame CFA reg old
  exit 1
fi
rm -f ./frame_test1
rm -f ./frame_test2
echo PASS legendre frame test
exit 0
