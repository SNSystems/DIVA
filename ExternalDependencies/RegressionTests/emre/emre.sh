#!/bin/bash
#=======================
# WARNING: Use a recent GCC version that supports the flags below. For
#instance,
# gcc version 4.7.2 should be ok.
GCC=gcc
OPTIONS="-fdebug-types-section -gdwarf-4 -g"
SRC="/tmp/input.c"
OUT="/tmp/input.o"

echo "
typedef struct {
  // empty
} c1_t;

// Ptr to empty struct in a typedef.
c1_t* cc;
" > $SRC

$GCC $OPTIONS -c $SRC -o $OUT

result=`readelf -w $OUT | grep "Length:.*0x1e "`

if [[ $result != "" ]]
then
  echo "Libdwarf is likely to choke on this object file due to a bug."
  echo "See the Compilation Unit of size 0x1e and typeoffset 0x1d."
fi
