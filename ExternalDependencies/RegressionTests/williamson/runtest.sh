#!/bin/sh
# We expect a core file, we do not worry about that.
# Hopeless bogus executable.

dd=$1
. ../BASEFILES
# Avoid spurious differences because of the names of the
# various dwarfdump versions being tested.
# This only deals with names like /tmp*dwarfdump2 and /tmp*dwarfdump
# and .*/dwarfdump2 and .*/dwarfdump
unifyddname () {
  nstart=$1
  nend=$2
  t1=junku1
  t2=junku2
  t3=junku3
  sed -e 'sx\/tmp.*\/dwarfdump2xdwarfdumpx' < $nstart >$t1
  sed -e 'sx\..*\/dwarfdump2xdwarfdumpx' <$t1 >$t2
  sed -e 'sx\/tmp.*\/dwarfdumpxdwarfdumpx' < $t2 >$t3
  sed -e 'sx\..*\/dwarfdumpxdwarfdumpx' < $t3 >$nend
  rm -f $t1
  rm -f $t2
  rm -f $t3
}


m() {
  dwdumper=$1
  obj=$2
  test=$3
  base=$4
  $dwdumper -i -G $obj 1>junk1 2>&1
  unifyddname junk1 $test
  diff $base $test
  if test  $?  -ne 0
  then
      echo "FAIL test williamson/...exe mismatch base $base vs $test "
      exit 1
  fi
}

# The test_sibling_loop.o will not terminate unless
# dwarfdump[2] is from February 2013 or later.
t=heap_buffer_overflow.exe
m $dd $t newout baseout
rm -f core*
t=hbo_unminimized.exe
m $dd $t newunminout baseunminout
rm -f core*
echo PASS williamson tests
exit 0


