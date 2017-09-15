#!/bin/sh
#
#
dd=$1
. ../BASEFILES
INCS="-I $libdw/libdwarf  -I /usr/local/include"
cc -g $INCS dwarfextract.c -o dwarfextract -L ../ -ldwarf -lelf
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
    exit 1
fi
$dd -a test1out >test1.new
diff test1.base test1.new
if [  $?  -ne 0 ] 
then
    echo FAIL dwarfextract test1
    exit 1
fi
echo PASS dwarfextract

cc -g -DPRODUCER_INIT_C=1 $INCS dwarfextract.c -o dwarfextractc -L .. -ldwarf -lelf
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
    exit 1
fi
$dd -a testcout >testc.new
diff testc.base testc.new
if [  $?  -ne 0 ]
then
    echo FAIL dwarfextract testc 
    exit 1
fi
echo PASS dwarfextract c
exit 0
