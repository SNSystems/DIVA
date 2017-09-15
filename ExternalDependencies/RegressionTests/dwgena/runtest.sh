#!/bin/sh
# Assumes a single argument, naming a dwarfdump.
# So it's testing the new dwarfgen and a new dwarfdump.

dd=$1

echo ../dwarfgen -t obj -c 0  -o junk1.bin ./dwarfgen-bin 
../dwarfgen -t obj -c 0  -o junk1.bin ./dwarfgen-bin >junkgen.out
echo $dd -a junk1.bin 
$dd -a junk1.bin >junk1.new
zcat test1.base.gz >test1.base
diff test1.base junk1.new
if [  $?  -ne 0 ]
then
    echo FAIL dwgena test 1
    echo "update via "
    echo "update via: mv junk1.new test1.base ; gzip test1.base"
    exit 1
fi

echo $dd -a -vvv junk1.bin
$dd -a -vvv junk1.bin >junk2.new
zcat test2.base.gz >test2.base
diff test2.base junk2.new
if [  $?  -ne 0 ]
then
    echo FAIL dwgena test 2
    echo "update via: mv junk2.new test2.base ; gzip test2.base"
    exit 1
fi
../dwarfgen -t obj -c 10 -o junk3.bin ./dwarfgen-bin >junkgen.out
echo $dd -a junk3.bin 
$dd -a junk3.bin >junk3.new
zcat test3.base.gz >test3.base
diff test3.base junk3.new
if [  $?  -ne 0 ]
then
    echo FAIL dwgena test 3
    echo "update via: mv junk3.new test3.base ; gzip test3.base"
    exit 1
fi

echo $dd -a -vvv junk3.bin 
$dd -a -vvv junk3.bin >junk4.new
zcat test4.base.gz >test4.base
diff test4.base junk4.new
if [  $?  -ne 0 ]
then
    echo FAIL dwgena test 4
    echo "update via: mv junk4.new test4.base ; gzip test4.base"
    exit 1
fi

# This has .debug_pubnames data.
echo ../dwarfgen -t obj -c 2 -o junk5.bin ./dwarfdump-bin 
../dwarfgen -t obj -c 2 -o junk5.bin ./dwarfdump-bin >junkgen.out
echo $dd -a -y -p junk5.bin 
$dd -a -y -p junk5.bin >junk5.new
zcat test5.base.gz >test5.base
diff test5.base junk5.new
if [  $?  -ne 0 ]
then
    echo FAIL dwgena test 5
    echo "update via: mv junk5.new test5.base ; gzip test5.base"
    exit 1
fi




echo PASS dwgena 
exit 0



