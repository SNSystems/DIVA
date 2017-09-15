#!/bin/sh
#  This is really a test of the new dwarf_get_form_class function.
l=$1
i=$2
if [ -f /usr/include/zlib.h ]
then
  cc -DWORKING=1 -I $i    test.c $l -lelf -lz -o test2
else
  cc -DWORKING=1 -I $i    test.c $l -lelf -o test2
fi

./test2 orig.a.out >out1
if [ $? != 0 ]
then
    echo FAIL dwarf_get_form_class, oops, in test-alex2
    exit 1
fi
grep "DW_FORM_CLASS_STRING" out1 >/dev/null
if [ $? != 0 ]
then
    echo FAIL dwarf_get_form_class in test-alex2
    exit 1
fi
exit 0
