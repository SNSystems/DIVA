#!/bin/bash
#=======================
# WARNING: Use a recent GCC version that supports the flags below. 
# For instance,
# gcc version 4.7.2 should be ok.
GXX=g++
OPTIONS="-fdebug-types-section -gdwarf-4 -g -Wl,--gdb-index"
SRC="input2.c"
OUT="input2"

echo "
#include <stdio.h>
class MyClass {
 public:
  MyClass(int i) : i_(i) { }
 private:
  int i_;
};

int main() {
  MyClass obj1(1);
  MyClass obj2(2);
  return 0;
}
" > $SRC

$GXX $OPTIONS $SRC -o $OUT

readelf --debug-dump=gdb_index $OUT  >junk1
result=`grep " MyClass: T0" junk1`

if [ x$result != "x" ]
then
  echo "Success."
  echo "Use: "
  echo "  dwarfdump -I $OUT"
  echo "and see the type unit entry for MyClass with index TU[0]."
  echo "Note that symbols that refer to it use [4] as its index."
fi
