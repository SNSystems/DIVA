#!/bin/sh
# This should create testhipcoffset.o  with several
# instances of DW_AT_high_pc being a class constant (offset).
# The dwarfdump.o has all the high pc as class address.

#set -x
# arg 1 is dwarfgen name, 2 is dwarfdumpname 3 is simplereader
gen=$1
dd=$2
sim=$3

tx=junktesthipcoffset.o
ty=junktesthipcoffsetorig.o
t1=junk1.highpc
t2=junk2.highpc
t3=junk3.basehighpc3
t4=junk4.genout
t5=junkh.basehighpc1
t6=junk2h.basehighpc2
t7=junk.basehighpc4
t8=junk8.genout
rm -f $tx
rm -f $t1
rm -f $t2
rm -f $t3
rm -f $t4
rm -f $t5
rm -f $t6
rm -f $t7
rm -f $t8
rm -f $tx
rm -f $ty

if [ ! -x $gen ]
then
    echo "FAIL: Unable to find dwarfgen named $gen"
    exit 1
fi
if [ ! -x $dd ]
then
    echo "FAIL: Unable to find dwarfdump named $dd"
    exit 1
fi
if [ ! -x $sim ]
then
    echo "FAIL: Unable to find simplereader named $sim"
    exit 1
fi

$gen  -h -t obj -c 0 -o $tx dwarfdump.o  >$t4 2>/dev/null

# This one no conversion.
$gen   -t obj -c 0 -o $ty dwarfdump.o  >$t8 2>/dev/null

$dd -i -M dwarfdump.o >$t1 2> /dev/null

grep high_pc $t1 >$t5

diff  basehighpc1 $t5
if [ $? -ne 0 ]
then
    echo FAIL diff basehighpc1 $t5
    echo FAIL offsetfromlowpc/runtest.sh  incorrect list of high_pc data.
    echo "to update baseline do: mv $t5 basehipc1"
    exit 1
fi

$dd -i -M $tx  >$t2 2> /dev/null

grep high_pc $t2  >$t6 2>/dev/null

diff   basehighpc2 $t6
if [ $? -ne 0 ]
then
    echo "did: $gen  -h -t obj -c 0 -o $tx dwarfdump.o  >$t4 "
    echo "did: $dd -i -M $tx > $t6"
    echo "did: diff   basehighpc2 $t6"
    echo FAIL diff basehighpc2 $t6
    echo "to update baseline do: mv $t6 basehighpc2"
    echo FAIL offsetfromlowpc/runtest.sh  incorrect transformed high_pc offset
    exit 1
fi

$sim --check $tx >$t3  2>/dev/null

diff basehighpc3 $t3
if [ $? -ne 0 ]
then
    echo FAIL offsetfromlowpc/runtest.sh  mismatch transformation to offset
    echo "to update baseline do: mv $t3 basehighpc3"
    exit 1
fi

$sim --check $ty >$t7  2>/dev/null
diff basehighpc4 $t7
if [ $? -ne 0 ]
then
    echo FAIL offsetfromlowpc/runtest.sh  mismatch non-transformation to offset
    echo "to update baseline do: mv $t7 basehipc4"
    exit 1
fi

echo PASS offsetfromlowpc/runtest.sh
exit 0

