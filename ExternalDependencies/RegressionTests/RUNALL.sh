#!/bin/sh
#
# By default runs all 3 test sets.
# To run just one, add one  option, one of dd ddtodd2 or dd2

dodd=y
echo "Running all  tests"

chkres() {
if test $1 != 0
then
  echo "Test failure: $2"
  exit 2
fi
}

# We expect there to be 2 lines with the FAIL string
# in the message.
chkfail () {
  f=$1
  grep '^FAIL 0$' $f >junkck2 
  c=`wc -l <junkck2`
  if test $c -ne 2
  then
    rm -f junkck2
    echo "Failure $2"
    echo "We have $c lines saying FAIL 0 so something is wrong."
    echo "There should be two FAIL 0 lines if everything passed."
    echo "Here are the first few FAIL lines:"
    grep FAIL $f |head -n 5
    echo "Here are the last few lines:"
    tail -n 5 $f
    endt=`date`
    echo "exit with error at $endt"
    exit 2
  else
    echo "PASS $2"
  fi
  rm -f junkck2
}

if [ $dodd = "y" ]
then
  rm -f ALLdd 
fi
start=`date`
echo "start $start"
if [ $dodd = "y" ]
then
  echo begin test dd
  ./DWARFTEST.sh dd 2>ALLdd 1>&2
  chkres $? "Failure in DWARFTEST.sh. Possibly coredump new dwarfdump? "
  chkfail ALLdd "running test dd"
fi

echo "start $start"
endt=`date`
echo "end   $endt"
exit 0
