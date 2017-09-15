#!/bin/sh
# execute as  runtest.sh <old dd name> <new dd name> 
# unpack the executable  (lots of zeroes, so it compresses well).
# zcat is the same as gunzip -c  

# The problem here is that it is dwarf2 and elf64 yet 32bit address
# so things go wrong. We need to specify address size is 4.
# So we need a dwarfdump.conf flag here.
set -x
zcat frame_problem.elf.gz > junk.frame_problem.elf
if [ $# -ne 2 ]
then
    echo Wrong number of args
    exit 1
fi

olddd=../$1
newdd=../$2
commonopts="-x name=../dwarfdump.conf -x abi=ppc32bitaddress"
#   -f runs in about a minute.
$newdd -f  $commonopts junk.frame_problem.elf >junk.f.out
if [ $? -ne 0 ] 
then
    echo FAIL error -f enciso4/junk.frame_problem.elf
    exit 1
fi
# The  -F  version runs over 20 minutes, so we skip that.
# eh_frame is pretty big, 0x1ab0e0 bytes.
#$newdd -F  $commonopts junk.frame_problem.elf >junk.F.out
#if [ $? -ne 0 ] 
#then
#    echo FAIL error -F enciso4/junk.frame_problem.elf
#    exit 1
#fi
echo PASS dumped enciso4/junk.frame_problem.elf
exit 0
