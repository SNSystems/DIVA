#!/bin/sh
l=$1
i=$2
if [ -f /usr/include/zlib.h ]
then
  gcc -DWORKING=1 -I $i    test.c $l -lelf -lz -o test1
  gcc -I $i    test.c  $l -lelf -lz -o test2
else
  gcc -DWORKING=1 -I $i    test.c $l -lelf -o test1
  gcc -I $i    test.c  $l -lelf -o test2
fi

./test1 orig.a.out >out1
./test2 orig.a.out >out2
diff out1 out2 >outdiffs
if [ $? != 0 ]
then
	echo FAIL alex-s test.
	exit 1
fi
exit 0
