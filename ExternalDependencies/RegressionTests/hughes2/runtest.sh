#!/bin/sh
if [ $# -ne 2 ]
then
   echo "FAIL hughes2/simplereader two args required, not $#"
   exit 1
fi
r=$1
tobj=$2 

m() {
e=$1
opts=$2
t=$3
b=$4
bt=junk.$4
expcore=$5
rm -f *core*
$e $opts $t > $bt
if [ $expcore = 'y' ]
then
  if [ ! -f *core* ]  
  then
    echo "FAIL hughes2/simplereader expected  corefile! "
    exit 1
  fi
else
  if [ -f *core* ]  
  then
    echo "FAIL hughes2/simplereader got a corefile unexpectedly"
    exit 1Y
  fi
fi
diff $b $bt
if [ $? -ne 0 ]
then
    echo "FAIL hughes2/simplereader $e $opts $t" 
    echo "update with mv $bt $b"
    exit 1
fi

}

m "$r" "--passnullerror"                  $tobj ne.base  y
m "$r" "--simpleerrhand --passnullerror"  $tobj hne.base n


echo "PASS hughes2/simplereader (coredump was expected, ignore it)"
exit 0
