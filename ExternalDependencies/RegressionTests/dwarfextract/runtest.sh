#!/bin/sh
#
#
dd=$1
. ../BASEFILES
INCS="-I $libdw/libdwarf  -I /usr/local/include"
if [ -f /usr/include/zlib.h ]
then
  cc -g $INCS dwarfextract.c -o dwarfextract -L ../ -ldwarf -lelf -lz
else
  cc -g $INCS dwarfextract.c -o dwarfextract -L ../ -ldwarf -lelf
fi
# Use precompiled test1.c test2.c for test consistency.
#cc -g test1.c test2.c -o test1
./dwarfextract test1 test1out >basestdout
if [  $?  -ne 0 ] 
then
    echo FAIL dwarfextract test0
    exit 1
fi
diff basestdout basestdout.base
if [  $?  -ne 0 ] 
then
    echo FAIL dwarfextract dwex-1
    echo "Fix with: mv basestdout basestdout.base"
    exit 1
fi
$dd -a test1out >test1.new
diff test1.base test1.new
if [  $?  -ne 0 ] 
then
    echo FAIL dwarfextract test1
    echo "Fix with: mv test1.new test1.base"
    exit 1
fi
echo PASS dwarfextract

if [ -f /usr/include/zlib.h ]
then
  cc -g -DPRODUCER_INIT_C=1 $INCS dwarfextract.c -o dwarfextractc -L .. -ldwarf -lelf -lz
else
  cc -g -DPRODUCER_INIT_C=1 $INCS dwarfextract.c -o dwarfextractc -L .. -ldwarf -lelf
fi
./dwarfextractc test1 testcout >basecstdout
if [  $?  -ne 0 ]
then
    echo FAIL dwarfextract looking for error
    exit 1
fi
diff basecstdout basecstdout.base
if [  $?  -ne 0 ] 
then
    echo FAIL dwarfextract dwexc-1
    echo "Fix with: mv basecstdout basecstdout.base"
    exit 1
fi
$dd -a testcout >testc.new
diff testc.base testc.new
if [  $?  -ne 0 ]
then
    echo FAIL dwarfextract testc 
    echo "Fix with: mv testc.new testc.base"
    exit 1
fi
echo PASS dwarfextract c
exit 0
