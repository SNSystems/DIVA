#!/bin/bash
# Look for certain things to see what objects have them.
# Useful for finding an object with something we want to see.
# Like a particular attribute or form.
# One expects that modifications will be needed
# for each case of interest.

t=dwarfdump
temp=/tmp/lookfor.x
dfout=/tmp/lookfor.dfout

#for i in mutatee/test1.mutatee_gcc.exe
for i in * */*
do
    f=$i
    if [ ! -f $f ]
    then
       continue
    fi
    file $f | grep ELF >/dev/null
    r=$?
    if [ $r -ne 0 ] 
    then
       continue
    fi
    #echo $r $f
    rm -f $temp
    $t -i -M -G $f  > $temp 2>/dev/null
    #lf=DW_FORM_strp
    lf=DW_FORM_GNU_strp_alt
    grep $lf $temp  >$dfout
    if [ $? -eq 0 ]
    then
      echo ""
      head -4 $dfout
      echo "$f $lf"
      continue
    fi
    lf=DW_FORM_strp_sup
    grep $lf $temp  >/dev/null
    if [ $? -eq 0 ]
    then
      echo ""
      head -4 $dfout
      echo "$f $lf"
      continue
    fi
    rm -f $temp
done

