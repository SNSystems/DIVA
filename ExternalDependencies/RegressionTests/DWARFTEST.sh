#!/bin/sh
#
echo 'Starting DWARFTEST.sh' `date`
ps -eaf |grep DWARF >/tmp/dwa.$$
echo "dwa.$$"
cat /tmp/dwa.$$
grep DWARFTEST.sh /tmp/dwa.$$ > /tmp/dwb.$$
ct=`wc -l </tmp/dwb.$$`
echo "Number of DWARFTEST.sh running: $ct"
echo "dwb.$$"
cat /tmp/dwb.$$
if [ $ct -gt 1 ]
then
  echo "Only one DWARFTEST.sh can run at a time on a machine"
  echo "Something is wrong, DWARFTEST.sh already running: $ct"
  echo "dwb.$$ contains:"
  echo /tmp/dwb.$$
  echo "do        : rm /tmp/dwa* /tmp/dwb* "
  echo "Check with: ps -eaf |grep DWARF"
  exit 1
fi

goodcount=0
failcount=0
. ./BASEFILES
cbase=$libdw
if [ $# -eq 0 ]
then
  d1=./dwarfdump.O
  d2=./dwarfdump
else
case $1 in
  dd ) d1=./dwarfdump.O ;  d2=./dwarfdump ; type=dd ;;
  dd2 ) d1=./dwarfdump2.O ; d2=./dwarfdump2 ; type=dd2 ;;
  ddtodd2 ) d1=./dwarfdump ; d2=./dwarfdump2 ; type=ddtodd2 ;;
  * )  echo BAD OPTION NO TEST RUN ; exit 1 ;;
esac
fi
echo "old is $d1"
echo "new is $d2"
dwlib=$cbase/libdwarf/libdwarf.a
dwinc=$cbase/libdwarf

baseopts='-F'
baseopts='-b -c  -f -F -i -l -m -o -p -r -s -ta -tf -tv -y -w  -N'

kopts="-ka -kb -kc -ke -kf -kF -kg  -kl -km -kM -kn -kr -kR -ks -kS -kt -kx -ky -kxe -kD -kG -ku -kuf"

# These accumulate times so we can print actual dwarfdump
# user, sys, clock times at the end (see usertime.py).
bdir=`pwd`
. ./RUNTIMEFILES
rm -f $otimeout 
rm -f $ntimeout
exit 1

chkres () {
  if [ $1 = 0 ]
  then
	goodcount=`expr $goodcount + 1`
  else
          echo FAIL  $2
          failcount=`expr $failcount + 1`
  fi
}

#ia32/libpt_linux_x86_r.so.1  -f -F runs too long.
filepaths='moshe/hello
klingler/dwarfgen_zdebug
klingler/test_with_zdebug
diederen2/pc_dwarf_bad_attributes.elf
diederen2/pc_dwarf_bad_sibling2.elf
diederen2/pc_dwarf_bad_reloc_empty_debug_info.elf
diederen2/pc_dwarf_bad_sibling.elf
diederen2/pc_dwarf_bad_reloc_section_link.elf
diederen2/pc_dwarf_bad_string_offset.elf
diederen3/pc_dwarf_bad_name3.elf
emre3/a.out.dwp
emre3/foo.dwo
emre3/main
emre3/foo.o
emre3/main.o 
diederen/hello
hughes/libkrb5support.so.0.1.debug
shopov1/main.exe
k10m/main_knc.o
pasztory/a.out64
relocerr64/tls32.o
relocerr64/tls64.o
relocerr64/armv5tel-32-tls.o
relocerr64/ia64-64-tls.o
relocerr64/mips-32-tls.o
relocerr64/ppc64-32-tls.o
relocerr64/ppc64-64-tls.o
relocerr64/s390x-32-tls.o
relocerr64/s390x-64-tls.o
relocerr64/sh4a-32-tls.o
relocerr64/sparc64-32-tls.o
relocerr64/sparc64-64-tls.o
enciso6/ranges.o
dwgenb/dwarfgen
moshe/a.out.t
marinescu/stream.o.test
simonian/test-gcc-4.3.0  
simonian/test-gcc-4.5.1
enciso3/test.o
dwarf4/dd2g4.5dwarf-4	
dwarf4/ddg4.5dwarf-4
x86/dwarfdumpv4.3 
allen1/todd-allen-gcc-4.4.4-bin.exe
wynn/unoptimised.axf
shihhuangti/tcombined.o
kartashev/combined.o
macinfo/a.out3.4 
macinfo/a.out4.3 
arm/armcc-test-dwarf2  
arm/armcc-test-dwarf3 
sun/sunelf1 
val_expr/libpthread-2.5.so 
ia64/hxdump.ia64 
ia64/mytry.ia64 
ia32/mytry.ia32  
ia32/libc.so.6 
x86-64/x86_64testcase.o 
testcase/testcase 
Test-eh/eh-frame.386 
test-eh/test-eh.386 
mutatee/test1.mutatee_gcc.exe 
cristi2/libpthread-2.4.so 
ia32/preloadable_libintl.so 
ia32/libpfm.so.3 
modula2/write-fixed
cristi3/cristibadobj
liu/divisionbyzero02.elf
liu/divisionbyzero.elf
liu/free_invalid_address.elf
liu/heapoverflow01b.elf
liu/heapoverflow01.elf
liu/HeapOverflow0513.elf
liu/infinitloop.elf
liu/null01.elf
liu/null02.elf
liu/NULLdereference0519.elf
liu/NULLderefer0505_01.elf
NULLdereference0522.elf
liu/OOB0505_01.elf
liu/OOB0505_02_02.elf
liu/OOB0505_02.elf
liu/OOB0517_01.elf
liu/OOB0517_02.elf
liu/OOB0517_03.elf
liu/OOB_read3_02.elf
liu/OOB_read3.elf
liu/OOB_read4.elf
liu/OOB_READ0519.elf
liu/outofbound01.elf
liu/outofboundread2.elf
liu/outofboundread.elf
irix64/libc.so 
irixn32/libc.so 
irixn32/dwarfdump' 



stripx() {
    #x=`echo $* | sed -e 's/-g//'`
    x=`echo $*`
    echo $x 
}
# Avoid spurious differences because of the names of the
# various dwarfdump versions being tested.
unifyddname () {
  nstart=$1
  nend=$2
  t1=tmpuerr1
  t2=tmpuerr2
  sed -e 'sx.\/dwarfdump.Ox.\/dwarfdumpx' < $nstart >$t1
  sed -e 'sx.\/dwarfdump2.Ox.\/dwarfdumpx' <$t1 >$t2
  sed -e 's/.\/dwarfdump2/.\/dwarfdump/' <$t2 >$nend
  rm -f $t1 $t2 $t3
}

runtest () {
	olddw=$1
	newdw=$2
	targ=$3
	shift
	shift
	shift
	
        echo "=====START  $*  $targ"
        rm -f core
        rm -f tmp1 tmp2 tmp3
        rm -f tmp1err tmp2err tmp3err 
        rm -f tmp1errb tmp1errc
        rm -f tmp1berr tmp2berr
        # OFo OFn are targets of dwarfdump -O file=path
        #
        rm -f OFo OFn  OFo1 OFn1 
        rm -f OFo2 OFn2
        rm -f OFo3 OFn3
	rm -f OFobkup

        # Running an old one till baselines established.
        echo "old start " `date`
        tmplist="$*"
        #echo "dadebug tmplist baseline line5 difference-fix ",$tmplist 
        # dadebug temp strip -x
        #tmplist2=`stripx "$*"`
        # echo "dadebug tmplist2 ",$tmplist2 
        echo "======" $tmplist $targ >> $otimeout
        $wrtimeo $olddw $tmplist  $targ 1>tmp1a 2>tmp1erra
        #$olddw $*  $targ 1>tmp1a 2>tmp1erra
        echo "old done " `date`
        unifyddname tmp1a tmp1
        unifyddname tmp1erra tmp1err
        if [ -f core ]
        then
           echo corefile in  $olddw '(old dwarfdump)'
           rm core
        fi
        if [ -f OFo ] 
        then
           unifyddname OFo OFo1
           grep -v Usage   OFo1 >OFo2
           # Delete date on first line
           sed '1d' OFo2 >OFo3
           cp OFo OFobkup
        fi

        echo "new start " `date`
        echo "======" $tmplist $targ >> $ntimeout
        $wrtimen $newdw  $* $targ  1>tmp2a 2>tmp2erra
        echo "new done " `date`
        unifyddname tmp2a tmp2
        unifyddname tmp2erra tmp2err
        date
        if [ -f core ]
        then
           echo corefile in  $newdw
           exit 1
        fi
        cat tmp2  >tmp3
        if [ -f OFo ] 
        then
           # Here OFo is really OFn
           unifyddname OFo OFn1
           cp OFo OFnbkup
           grep -v Usage   OFn1 >OFn2
           # Delete date on first line
           sed '1d' OFn2 >OFn3
        fi

        if [ -f OFn3 -o -f OFo3 ]
        then
          touch OFn3 OFo3
          diff OFo3 OFn3
          if [ $? = 0 ]
          then
            goodcount=`expr $goodcount + 1`
          else
            echo "FAIL -O file=path"  $* $targ
            failcount=`expr $failcount + 1`
          fi
        fi

        # 
        diff tmp1 tmp3
        if [ $? = 0 ]
        then
          goodcount=`expr $goodcount + 1`
        else
          echo FAIL  $* $targ
          failcount=`expr $failcount + 1`
        fi

        grep -v Usage   tmp1err >tmp1berr
        grep -v Usage   tmp2err >tmp2berr
        diff tmp1berr tmp2berr
        if [ $? = 0 ]
        then
          goodcount=`expr $goodcount + 1`
        else
          echo FAIL Usage  $* $targ
          failcount=`expr $failcount + 1`
        fi
        rm -f core
        rm -f tmp1 tmp2 tmp3
        rm -f tmp1err tmp2err tmp3err 
        rm -f tmp1errb tmp1errc
        rm -f tmp1berr tmp2berr
	rm -f OFobkup
        rm -f OFo OFn  OFo1 OFn1 
        rm -f OFo2 OFn2
        rm -f OFo3 OFn3
}
# end 'runtest'

echo "=====START   hughes2 runtest.sh"
cd hughes2
sh runtest.sh ../simplereader ../corruptdwarf-a/simplereader.elf
chkres $?  hughes2
cd ..

# Testing SHF_COMPRESSED .debug* section reading.
runtest $d1 $d2  klingler2/compresseddebug.amd64 -i
runtest $d1 $d2  klingler2/compresseddebug.amd64 -a
# .eh_frame is not actually compressed...
runtest $d1 $d2  klingler2/compresseddebug.amd64 -F

# The object has a bad ELF section type. So should
# generate an error.  Should not coredump.
runtest $d1 $d2  xqx/awbug6.elf -i 

# This test has odd abbreviation codes. See that we notice them.
runtest $d1 $d2  xqx-b/aw.elf -kb 
runtest $d1 $d2  xqx-b/aw.elf -ka 
runtest $d1 $d2  xqx-b/aw.elf -a 
runtest $d1 $d2  xqx-b/aw.elf -f 
runtest $d1 $d2  xqx-b/aw.elf -F 
runtest $d1 $d2  xqx-b/awbug5.elf -kb 
runtest $d1 $d2  xqx-b/awbug5.elf -ka 
runtest $d1 $d2  xqx-b/awbug5.elf -a 
runtest $d1 $d2  xqx-b/awbug5.elf -f 
runtest $d1 $d2  xqx-b/awbug5.elf -F 

# A corrupted object file
runtest $d1 $d2  liu/NULLdeference0522c.elf -a 

#  For line table variants checking.
for x in '-x line5=std' '-x line5=s2l' '-x line5=orig' '-x line5=orig2l'
do
runtest $d1 $d2  sparc/tcombined.o -a -R  -v -v -v -v -v -v $x
runtest $d1 $d2  sparc/tcombined.o -a -R   -v  $x
runtest $d1 $d2  sparc/tcombined.o -a -R    $x
runtest $d1 $d2  x86/dwarfdumpv4.3 -a -R  -v -v -v -v -v -v $x
runtest $d1 $d2 legendre/libmpich.so.1.0 -ka  $x
runtest $d1 $d2 legendre/libmpich.so.1.0 -a  $x
runtest $d1 $d2 legendre/libmpich.so.1.0 -l  $x
runtest $d1 $d2 irixn32/dwarfdump -f -x name=./dwarfdump.conf -x abi=mips-irix2 $x
runtest $d1 $d2  ppc2/powerpc-750-linux-gnu-hello-static -a   -R -v -v -v -v -v -v  $x
runtest $d1 $d2  mucci/main.o -c -R -ka  -v -v -v -v -v -v $x
done

runtest $d1 $d2 augmentation/a.out -f 
runtest $d1 $d2 augmentation/a.out -f  -vvv
runtest $d1 $d2 corruptdwarf-a/simplereader.elf -i  
runtest $d1 $d2 corruptdwarf-a/simplereader.elf -a  
runtest $d1 $d2 corruptdwarf-a/simplereader.elf -a  -vvv

runtest $d1 $d2 irixn32/dwarfdump -i -x name=./dwarfdump.conf -x abi=mips-irix2 -g
runtest $d1 $d2 comdatex/example.o -i -g

# Test support for DW_FORM_GNU_strp_alt
runtest $d1 $d2 hughes/libkrb5support.so.0.1.debug -i  -l -M -x tied=hughes/krb5-1.11.3-38.fc20.x86_64 

# for two-level line tables 
runtest $d1 $d2 emre4/test19_64_dbg -l
runtest $d1 $d2 emre4/test19_64_dbg -a
runtest $d1 $d2 emre4/test3_64_dbg -l
runtest $d1 $d2 emre4/test3_64_dbg -a
runtest $d1 $d2 emre4/test19_64_dbg -l -vvv
runtest $d1 $d2 emre4/test19_64_dbg -a -vvv
runtest $d1 $d2 emre4/test3_64_dbg -l -vvv
runtest $d1 $d2 emre4/test3_64_dbg -a -vvv
runtest $d1 $d2 emre4/test19_64_dbg -l -v
runtest $d1 $d2 emre4/test19_64_dbg -a -v
runtest $d1 $d2 emre4/test3_64_dbg -l -v
runtest $d1 $d2 emre4/test3_64_dbg -a -v


# This one has .debug_cu_index
# Some duplication with generic test loop.
runtest $d1 $d2 emre3/a.out.dwp -i
runtest $d1 $d2 emre3/a.out.dwp -i -v
runtest $d1 $d2 emre3/a.out.dwp -i -d
runtest $d1 $d2 emre3/a.out.dwp -i -d -v
runtest $d1 $d2 emre3/a.out.dwp -I

runtest $d1 $d2 duplicatedattr/duplicated_attributes.o -i -O file=./OFo
runtest $d1 $d2 duplicatedattr/duplicated_attributes.o -kD
runtest $d1 $d2 duplicatedattr/duplicated_attributes.o -kG
runtest $d1 $d2 duplicatedattr/duplicated_attributes.o -ku
runtest $d1 $d2 duplicatedattr/duplicated_attributes.o -kuf
#
# These are testing  some mangled objects for
# sensible output. We do not want a core dump.
runtest $d1 $d2 williamson/heap_buffer_overflow.exe -i -G
runtest $d1 $d2 williamson/hbo_unminimized.exe -i -G
runtest $d1 $d2 williamson/heap_buffer_overflow_01.exe -i
# The following has a bad info value in a rela. Coredumped dwarfdump.
runtest $d1 $d2 williamson/heap_vulnerability_20150201 -i

# duplicatedattr test dir has stuff to test. FIXME 

# This should not coredump dwarfdump. Did as of Jan 1, 2015
runtest $d1 $d2 comdatex/example.o -i

# This is a .dwp file with .debug_cu_index and .debug_tu_index.
# Results are so large (500MB) it is unwise to run all options
runtest $d1 $d2 debugfissionb/ld-new.dwp -I -v -v -v
runtest $d1 $d2 debugfissionb/ld-new.dwp -i -v -v -v
runtest $d1 $d2 debugfissionb/ld-new.dwp -ka
runtest $d1 $d2 debugfissionb/ld-new.dwp -i -x tied=debugfissionb/ld-new
runtest $d1 $d2 debugfissionb/ld-new.dwp -a -x tied=debugfissionb/ld-new

# A very short debug_types file. Used to result in error due to bug.
runtest $d1 $d2 emre/input.o -a

# Has a type unit so we can see the index for such.
runtest $d1 $d2 emre2/emre.ex -I

runtest $d1 $d2  -i -x tied=emre5/test33_64_opt_fpo_split emre5/test33_64_opt_fpo_split

# This has a .gdb_index   file print
# Unwise to run all options.
runtest $d1 $d2  debugfissionb/ld-new -I
runtest $d1 $d2  debugfissionb/ld-new -I  -v -v -v
runtest $d1 $d2  debugfissionb/ld-new -a  
runtest $d1 $d2  debugfissionb/ld-new -ka  

cd baddie1
sh runtest.sh ../$d2 
chkres $?  baddie1
cd ..

cd offsetfromlowpc
sh runtest.sh ../dwarfgen ../$d2  ../simplereader
chkres $?  offsetfromlowpc
cd ..

echo "=====START   debugfissionb runtest.sh"
cd debugfissionb
sh runtest.sh  ../simplereader
chkres $?  debugfissionb-simplreader
cd ..

echo "=====START   debugfission runtest.sh"
cd debugfission
sh runtest.sh  ../$d2 
chkres $?  debugfission
cd ..

# This validates standard-based handling of DW_FORM_ref_addr
runtest $d1 $d2 diederen/hello -i

# this coredumps libdwarf 20121130
runtest $d1 $d2 libc6fedora18/libc-2.16.so.debug -a

# Testing the wasted-space from not using LEB.
runtest $d1 $d2 enciso5/sample_S_option.o  -kE

# These print object header (elf) information.
runtest $d1 $d2 enciso5/sample_S_option.o  -E
runtest $d1 $d2 enciso5/sample_S_option.o  -Ea
runtest $d1 $d2 enciso5/sample_S_option.o  -Eh
runtest $d1 $d2 enciso5/sample_S_option.o  -El
runtest $d1 $d2 enciso5/sample_S_option.o  -Ei
runtest $d1 $d2 enciso5/sample_S_option.o  -Ep
runtest $d1 $d2 enciso5/sample_S_option.o  -Er
runtest $d1 $d2 enciso5/sample_S_option.o  -Er -g
runtest $d1 $d2 irixn32/dwarfdump  -Ef
# Following finds no debug_loc.
runtest $d1 $d2 enciso5/sample_S_option.o  -Eo
# Following finds a debug_loc.
runtest $d1 $d2 mucci/main.gcc -Eo
#Following has no .debug_ranges
runtest $d1 $d2 enciso5/sample_S_option.o  -ER
#Following has .debug_ranges
runtest $d1 $d2 mucci/main.gcc  -ER
runtest $d1 $d2 mucci/main.gcc  -ER -g

# AARCH64 Arm 64bit.
runtest $d1 $d2 juszkiewicz/t1.o -a
runtest $d1 $d2 juszkiewicz/t2.o -a -v
runtest $d1 $d2 juszkiewicz/tcombined.o -a

runtest $d1 $d2 enciso5/sample_S_option.o  -Es
# The Et does nothing, we do not seem to have 
# a .debug_pubtypes (IRIX specific) section anywhere.
runtest $d1 $d2 enciso5/sample_S_option.o  -Et
runtest $d1 $d2 enciso5/sample_S_option.o  -Ex
runtest $d1 $d2 enciso5/sample_S_option.o  -Ed

# Prints DIEs with the name or value DW_AT_low_pc
runtest $d1 $d2 enciso5/sample_S_option.o  -S match=DW_AT_low_pc
runtest $d1 $d2 enciso5/sample_S_option.o  -S match=DW_AT_low_pc -W
runtest $d1 $d2 enciso5/sample_S_option.o  -S vmatch=DW_AT_low_pc
runtest $d1 $d2 enciso5/sample_S_option.o  -S vmatch=DW_AT_low_pc -W

# Prints DIEs with the name or value DW_AT_high_pc
runtest $d1 $d2 enciso5/sample_S_option.o  -S match=DW_AT_high_pc
# Prints DIEs with the name or value 0x0000001c
runtest $d1 $d2 enciso5/sample_S_option.o  -S match=0x0000001c 
# Prints DIEs with the name or value value DW_OP_plus
runtest $d1 $d2 enciso5/sample_S_option.o  -S any=DW_OP_plus
# Following is for URI testing.
runtest $d1 $d2 enciso5/sample_S_option.o  -S any=DW%5fOP_plus
# The following should work with a space but
# does not because the shell strips quotes and getopt() won't process
# the space-containing single option properly even if quoted.
# So we use uri-style.
# Following is for URI testing.
runtest $d1 $d2 enciso5/sample_S_option.o  -S match=DW_OP_plus_uconst%208
# The following prints any DIE with name or value containing  anything
# in the range reg0 through reg9.
runtest $d1 $d2 enciso5/sample_S_option.o  -S regex="reg[0-9]"
# Following is for URI testing
runtest $d1 $d2 enciso5/sample_S_option.o  -S regex="reg%5b0-9]"
# The following prints a single DIE, just basic information about it.
runtest $d1 $d2 enciso5/sample_W_option.o  -S match=gg
# With -W, both parent and child data about the DIE is printed.
runtest $d1 $d2 enciso5/sample_W_option.o  -S match=gg -W 
# With -W, just parent data about the DIE is printed.
runtest $d1 $d2 enciso5/sample_W_option.o  -S match=gg -Wp 
# With -W, just children data about the DIE is printed.
runtest $d1 $d2 enciso5/sample_W_option.o  -S match=gg -Wc 

# The next are on the clang compiler v 2.9, which is making some mistakes.
runtest $d1 $d2 vlasceanu/a.out -i -M
runtest $d1 $d2 vlasceanu/a.out -a 
runtest $d1 $d2 vlasceanu/a.out -ka 

#Put uri in name
# Following is for URI testing
runtest $d1 $d2 moshe%2fhello  -i
#Put uri in name, let it fail as no translate done.
# Following is for URI testing
runtest $d1 $d2 moshe%2fhello  -U -i
#Put uri in name, do not mention the uri translateion
# Following is for URI testing
runtest $d1 $d2 moshe%2fhello  -q -i

# The -h option does not exist. Try it anyway!
runtest $d1 $d2  moshe/hello -h 
runtest $d1 $d2  moshe/hello -a -vvv -R -M
runtest $d1 $d2  moshe/hello -a -vvv -R -M -g
runtest $d1 $d2  moshe/hello -ka -vvv -R -M
runtest $d1 $d2  moshe/a.out.t -a -vvv -R -M
runtest $d1 $d2  moshe/a.out.t -ka -vvv -R -M
runtest $d1 $d2  dwarf4/dd2g4.5dwarf-4 -a  -vvv -R -M
runtest $d1 $d2  dwarf4/dd2g4.5dwarf-4 -ka -vvv -R -M
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -a  -vvv -R -M
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -ka -vvv -R -M
# ka p, where we test a warning message is generated (p is printing option)
# And should run like just -p (as of Jan 2015 erroneously did many checks).
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -ka -p  -R -M 
# normal ka, full checks
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -ka  -R -M 
# ka P, where P means print CU names per compiler.
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -ka -P  -R -M 
# ka P kd, where so print CU names and error summary per compiler
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -ka -kd -P  -R -M 
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -i  
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -i -g
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -i -d
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -i -v
runtest $d1 $d2  dwarf4/ddg4.5dwarf-4 -i -d -v

runtest $d1 $d2  marinescu/hello.original -ka -x abi=ppc
# Following is for URI testing
runtest $d1 $d2  marinescu/hello%2eoriginal -ka -x abi=pp%63
runtest $d1 $d2  marinescu/hello.original -a -x abi=ppc
runtest $d1 $d2  marinescu/armcc-test-dwarf2.original -ka -x abi=ppc
runtest $d1 $d2  marinescu/armcc-test-dwarf2.original -a -x abi=ppc
runtest $d1 $d2  marinescu/tcombined.o.div -a
runtest $d1 $d2  marinescu/tcombined.o.seg -a
runtest $d1 $d2  marinescu2/dwarfdump-invalid-read -a
runtest $d1 $d2  marinescu2/hello.o.div -a

# The following 3 print the lines  differently.
# The first prints a shorthand for file path.
# The second a filepath for each line
# The third prints details on the line operators.
# In these 3 the name is long, so -l and -l -v print differently.
runtest $d1 $d2  wynn/unoptimised.axf -l 
runtest $d1 $d2  wynn/unoptimised.axf -l -v
runtest $d1 $d2  wynn/unoptimised.axf -l -vvv
# These have short names, so -l -v prints like -l  prints.
runtest $d1 $d2  mucci/stream.o -l 
runtest $d1 $d2  mucci/stream.o -l  -v
runtest $d1 $d2  mucci/stream.o -l  -vvv

# currently generates DW_DLE_LINE_FILE_NUM_BAD
# due to gcc bug in line output.
runtest $d1 $d2  moore/djpeg.v850 -l -R

runtest $d1 $d2  enciso2/template.elf -a -vvv -R -M
runtest $d1 $d2  enciso2/template.elf -a -R -M
runtest $d1 $d2  enciso2/template.elf -a -R -M -g
runtest $d1 $d2  enciso2/template.elf -ka -R -M
runtest $d1 $d2  enciso2/template.elf -ka -kxe -R -M
runtest $d1 $d2  enciso2/template.elf -kxe -R -M
runtest $d1 $d2  enciso2/test_templates.o  -a -R -M
runtest $d1 $d2  enciso2/test_templates.o  -a -R 
runtest $d1 $d2  x86/dwarfdumpv4.3 -S match=main 
runtest $d1 $d2  x86/dwarfdumpv4.3 -S any=leb 
runtest $d1 $d2  x86/dwarfdumpv4.3 -S 'regex=u.*leb' 
runtest $d1 $d2  wynn/unoptimised.axf  -f   -x abi=arm
runtest $d1 $d2  wynn/unoptimised.axf  -kf  -x abi=arm
runtest $d1 $d2  arm/armcc-test-dwarf2 -f   -x abi=arm 
runtest $d1 $d2  arm/armcc-test-dwarf2 -ka  -x abi=arm 
runtest $d1 $d2  arm/armcc-test-dwarf2 -ka -kxe -x abi=arm 
runtest $d1 $d2  arm/armcc-test-dwarf2 -kxe -x abi=arm 
runtest $d1 $d2  arm/armcc-test-dwarf3 -f  -x abi=arm
runtest $d1 $d2  arm/armcc-test-dwarf3 -ka  -x abi=arm
runtest $d1 $d2  arm/armcc-test-dwarf3 -ka  -kxe -x abi=arm
runtest $d1 $d2  arm/armcc-test-dwarf3 -kxe -x abi=arm
runtest $d1 $d2  lloyd/arange.elf  -r
runtest $d1 $d2  lloyd/arange.elf  -kr

# With mips abi and C dwarfdump we get error 
# DW_DLE_FRAME_REGISTER_UNREPRESENTABLE.  That is normal, expected.
# It is not MIPS and MIPS is just wrong here.  Testing it anyway!
if [ ! $type = "ddtodd2" ]
then
 runtest $d1 $d2  val_expr/libpthread-2.5.so -x abi=mips -F -v -v -v
fi

echo "=====START   findcu runtest"
cd findcu 
sh runtest.sh $cbase >testoutput
chkres $? 'findcu/cutest-of-a-libdwarf-interface'
cd ..


echo "=====START   test_harmless"
if [ -f /usr/include/zlib.h ]
then
  cc -Wall -I  $cbase/libdwarf test_harmless.c  -o test_harmless $cbase/libdwarf/libdwarf.a -lelf -lz
else
  cc -Wall -I  $cbase/libdwarf test_harmless.c  -o test_harmless $cbase/libdwarf/libdwarf.a -lelf
fi
./test_harmless >testoutput
chkres $? 'check harmless-error functionality'


echo "=====START   test gennames -t and -s same output"
# Testing that gennames -t and -s generate the same results.
./gennames -s  -i $cbase/libdwarf -o .
mv dwarf_names.c dwarfnames-s.c
cc -Wall -I $cbase/libdwarf test_dwarfnames.c dwarfnames-s.c -o dwarfnames-s
./dwarfnames-s > dwn_s_out
./gennames -t  -i $cbase/libdwarf -o .
mv dwarf_names.c dwarfnames-t.c
cc -Wall -I  $cbase/libdwarf test_dwarfnames.c dwarfnames-t.c -o dwarfnames-t
./dwarfnames-t > dwn_t_out

diff dwn_s_out dwn_t_out
chkres $?  dwarfnames-switch-table-check
rm -f dwarf_names.c dwarfnames-s.c
rm -f  dwarfnames-t.c
rm -f dwarf-names-s dwarfnames-t dwn_s_out dwn_t_out
rm -f dwarf_names_enum.h dwarf_names.h  dwarf_names_new.h 

echo "=====START   dwgena/runtest.sh"
cd dwgena
sh runtest.sh ../$d2
chkres $? 'dwgena/runtest.sh'
cd ..

echo "=====START   frame1/runtest.sh"
cd frame1
sh runtest.sh $cbase
chkres $? frame1
cd ..

echo "=====START   dwarfextract/runtest.sh"
cd dwarfextract
rm -f dwarfextract
sh runtest.sh ../$d2
chkres $?  dwarfextract
cd ..

echo "=====START   sandnes2/runtest.sh"
cd sandnes2
sh runtest.sh
chkres $?  sandnes2
cd ..

echo "=====START   legendre/runtest.sh"
cd legendre
sh runtest.sh $cbase
chkres $?  legendre
cd ..

echo "=====START   enciso4/runtest.sh"
cd enciso4
sh runtest.sh $d1 $d2 
chkres $?  enciso4
cd ..

runtest $d1 $d2 irixn32/dwarfdump -g  dwconf.c -x name=dwarfdump.conf  -x abi=mips-irix
runtest $d1 $d2 irixn32/dwarfdump -u  dwconf.c -x name=dwarfdump.conf  -x abi=mips-irix
#The following is for URI style test completeness
runtest $d1 $d2 irixn32/dwarfdump -u  dwconf%2ec -x name=dwarfdump%2econf  -x abi=mips-irix
runtest $d1 $d2 irixn32/dwarfdump -u  /xlv44/6.5.15m/work/irix/lib/libc/libc_n32_M3/csu/crt1text.s  -x name=dwarfdump.conf -x abi=mips-irix

#Following tests -c and URI, the one restricted to GNU AS 
# only checks and reports on errors found in CUs with producer GNU AS.
runtest $d1 $d2 modula2/write-fixed -ka -cGNU%20AS  -M -R
runtest $d1 $d2 modula2/write-fixed -ka -M -R





runtest $d1 $d2  sparc/tcombined.o -a -R  -v -v -v -v -v -v
runtest $d1 $d2  sparc/tcombined.o -ka -R  -v -v -v -v -v -v
runtest $d1 $d2  kartashev2/combined.o -a -R  -v -v -v -v -v -v
runtest $d1 $d2  kartashev2/combined.o -ka -R  -v -v -v -v -v -v
runtest $d1 $d2  x86/dwarfdumpv4.3 -a -R  -v -v -v -v -v -v
runtest $d1 $d2  x86/dwarfdumpv4.3 -ka -R -v -v -v -v -v -v
runtest $d1 $d2  mucci/stream.o -a -R   -v -v -v -v -v -v
runtest $d1 $d2  mucci/stream.o -ka -R   -v -v -v -v -v -v
runtest $d1 $d2  mucci/stream.o -c -R -ka  
runtest $d1 $d2  mucci/stream.o -c -R -ka  -v -v -v -v -v -v
runtest $d1 $d2  mucci/main.o -a -R   -v -v -v -v -v -v
runtest $d1 $d2  mucci/main.o -ka -R   -v -v -v -v -v -v
runtest $d1 $d2  mucci/main.o -c -R -ka  -v -v -v -v -v -v
runtest $d1 $d2  mucci/main.o -c -R -ka  
runtest $d1 $d2 mucci/stream.o -a -R -M
runtest $d1 $d2 mucci/stream.o -i -e
runtest $d1 $d2 legendre/libmpich.so.1.0 -f -F 
runtest $d1 $d2 legendre/libmpich.so.1.0 -ka 

for i in cell/c_malloc.o moore/simplec.o \
   enciso2/test_templates.o enciso3/test.o kartashev/combined.o \
   linkonce/comdattest.o louzon/ppcobj.o  mucci/main.o \
   mucci/main.o.gcc mucci/main.o.pathcc  \
   mucci/stream.o  saurabh/augstring.o \
   shihhuangti/t1.o shihhuangti/t2.o shihhuangti/tcombined.o \
   sparc/tcombined.o  atefail/ig_server  cell/c_malloc.o
do
  runtest $d1 $d2 $i -o
  for o in -oi -ol -op -or -of -oo -oR
  do
    runtest $d1 $d2 $i $o
  done
done





if [  $type = "dd2" ]
then
  # Running dd on this file is too slow. Only dd2 ok.
  # The issue is the number of registers and which
  # frame reg access interface is used for -f and -F.
  runtest $d1 $d2  ia32/libpt_linux_x86_r.so.1 -a -vvv -R -F
  runtest $d1 $d2  ia32/libpt_linux_x86_r.so.1 -c -vvv -R -F
fi

echo "=====START   test-alex1/runtest"
cd test-alex1
sh runtest.sh $dwlib $dwinc
chkres $?  test-alex1
cd ..

echo "=====START   test-alex2/runtest $dwlib $dwinc"
cd test-alex2
sh runtest.sh $dwlib $dwinc
chkres $?  test-alex1
cd ..

# We need this to not do all DIE printing. FIXME
runtest $d1 $d2 macro5/dwarfdump-g3  -m
runtest $d1 $d2 macro5/dwarfdump-g3  -m -vvv
runtest $d1 $d2 macro5/dwarfdump-g3  -m -v
#Here we ask for DIE printing.
runtest $d1 $d2 macro5/dwarfdump-g3 -i -m
runtest $d1 $d2 macro5/dwarfdump-g3 -i -m -vvv
runtest $d1 $d2 macro5/dwarfdump-g3 -i -m -v

#Following 2 show some DW_AT_MIPS_fde difference. So -C works.
runtest $d1 $d2  irix64/libc.so -ka   -x name=dwarfdump.conf -x abi=mips-simple3
runtest $d1 $d2  irix64/libc.so -i    -x name=dwarfdump.conf -x abi=mips-simple3
#Following shows -C differences, see ia64/README. So -C works.
runtest $d1 $d2  ia64/mytry.ia64 -i
runtest $d1 $d2  ia64/mytry.ia64 -ka 
runtest $d1 $d2  ia64/mytry.ia64 -i -C
runtest $d1 $d2  ia64/mytry.ia64 -ka  -C

runtest $d1 $d2  ref_addr/ELF3.elf -R -a -v -v -v -v -v -v 
runtest $d1 $d2  ref_addr/ELF3.elf -R -a -M  -v -v -v -v -v -v 
runtest $d1 $d2  ref_addr/ELF3.elf -R -ka -M -v -v -v -v -v -v 

# This one fails,return add reg num 108.
runtest $d1 $d2  ppc2/powerpc-750-linux-gnu-hello-static -a  -v -v -v -v -v -v 
runtest $d1 $d2  ppc2/powerpc-750-linux-gnu-hello-static -ka  -v -v -v -v -v -v 
# This one works, return address reg 108 allowed.
runtest $d1 $d2  ppc2/powerpc-750-linux-gnu-hello-static -a   -R -v -v -v -v -v -v 
runtest $d1 $d2  ppc2/powerpc-750-linux-gnu-hello-static -c   -R -v -v -v -v -v -v 
# This one works, return address reg 108 allowed.
runtest $d1 $d2  ppc2/powerpc-750-linux-gnu-hello-static -a -R -v -v -v -v -v -v -x name=./dwarfdump.conf  -x abi=ppc 
runtest $d1 $d2  ppc2/powerpc-750-linux-gnu-hello-static -c -R -v -v -v -v -v -v -x name=./dwarfdump.conf  -x abi=ppc 

runtest $d1 $d2  louzon/ppcobj.o -a  -v -v -v -v -v -v -x name=./dwarfdump.conf  -x abi=ppc
runtest $d1 $d2  louzon/ppcobj.o -ka -v -v -v -v -v -v -x name=./dwarfdump.conf  -x abi=ppc
runtest $d1 $d2  linkonce/comdattest.o -ka -v -v -v -v -v -v -x name=./dwarfdump.conf  -x abi=ppc
runtest $d1 $d2  linkonce/comdattest.o -a -v -v -v -v -v -v -x name=./dwarfdump.conf  -x abi=ppc
runtest $d1 $d2 atefail/ig_server -kt 
# Testing old interface to libdwarf with -g. IRIX only.
runtest $d1 $d2 irixn32/dwarfdump -f -g 
runtest $d1 $d2 irixn32/dwarfdump -f -x name=./dwarfdump.conf -x abi=mips-simple3  
runtest $d1 $d2 irixn32/dwarfdump -f -n -x name=./dwarfdump.conf -x abi=mips-simple3  
runtest $d1 $d2 ia32/mytry.ia32 -F -x name=dwarfdump.conf -x abi=x86
runtest $d1 $d2 ia64/mytry.ia64 -F -x name=dwarfdump.conf -x abi=ia64 
# The following is a misspelling of abi. Checks for error spelling so leave it in.
runtest $d1 $d2  irixn32/dwarfdump -f -x name=./dwarfdump.conf -x abi=mips-simple 
runtest $d1 $d2  irixn32/dwarfdump -f -x name=./dwarfdump.conf -x abi=mips-simple3 
runtest $d1 $d2  irixn32/dwarfdump -f -x name=./dwarfdump.conf -x abi=mips-irix
# Restrict to a single fde report
runtest $d1 $d2 irixn32/dwarfdump -f -H 1  -x name=./dwarfdump.conf -x abi=mips-irix
# Restrict to a single fde report with no function names shown
runtest $d1 $d2 irixn32/dwarfdump -f -H 1 -n  -x name=./dwarfdump.conf -x abi=mips-irix
# mips-irix2 is the new name for what was mips-simple.
runtest $d1 $d2  irixn32/dwarfdump -f -x name=./dwarfdump.conf -x abi=mips-irix2
runtest $d1 $d2  irixn32/dwarfdump -f -x name=./dwarfdump.conf -x abi=mips
runtest $d1 $d2 irixn32/dwarfdump -f -x name=./dwarfdump.conferr1 -x abi=mips
runtest $d1 $d2 val_expr/libpthread-2.5.so -f -v -v -x name=dwarfdump.conf -x abi=x86_64 
runtest $d1 $d2 irixn32/dwarfdump -i -G 
# restrict to a single CU
runtest $d1 $d2 irixn32/dwarfdump -i -H 1 
runtest $d1 $d2 irixn32/dwarfdump -ka
runtest $d1 $d2 irixn32/dwarfdump -i -G -d  

# Using old loclist call. Without -v nothing prints, so use -v.
# Adding -D is useless since then the attributes don't print at all so
# one cannot see the removal of offset from the loclist.
runtest $d1 $d2 irixn32/libc.so -g  -v -x name=dwarfdump.conf  -x abi=mips-irix
runtest $d1 $d2 ia32/mytry.ia32 -i -G  
runtest $d1 $d2 ia32/mytry.ia32 -i -G -d  
runtest $d1 $d2 ia32/mytry.ia32 -ka -G -d 
runtest $d1 $d2 cristi2/libc-2.5.so -F -x name=dwarfdump.conf -x abi=x86
runtest $d1 $d2 ia32/libc.so.6 -F -f -x name=dwarfdump.conf -x abi=x86
# Do not find function names (using -n)
runtest $d1 $d2 ia32/libc.so.6 -F -f -n  -x name=dwarfdump.conf -x abi=x86
# Restrict to single fde
runtest $d1 $d2 ia32/libc.so.6 -F -f -H 1 -x name=dwarfdump.conf -x abi=x86
# Restrict to single fde, single CIE
runtest $d1 $d2 ia32/libc.so.6 -F -f -H 1 -v -x name=dwarfdump.conf -x abi=x86
runtest $d1 $d2 ia32/libc.so.6 -F -f -n -x name=dwarfdump.conf -x abi=x86
runtest $d1 $d2 cristi2/libpthread-2.4.so -F -v -v -v -x name=dwarfdump.conf -x abi=x86
runtest $d1 $d2 cristi2/libpthread-2.4.so -ka -P -x name=dwarfdump.conf -x abi=x86
runtest $d1 $d2 cristi2/libpthread-2.4.so -ka -kd -P -x name=dwarfdump.conf -x abi=x86
runtest $d1 $d2 cristi2/libpthread-2.4.so -R -F  -v -v -v
runtest $d1 $d2 cristi2/libpthread-2.4.so -R -ka  -v -v -v
runtest $d1 $d2 cristi3/cristibadobj -m 
runtest $d1 $d2 cristi3/cristibadobj -m  -v -v -v

echo PASS $goodcount
echo FAIL $failcount

for i in $filepaths
do
   echo  "===== $i all options"
   for xtra in "" "-v" "-vv" "-vvv"  "-D" "-H 2" 
   do
     for k in  $baseopts "-i -M" 
     do
	runtest $d1 $d2 $i $k $xtra
     done
   done
done
for i in $filepaths
do
   echo ===== $i all checking options
   # -kd ensures we report the test statistics
   for xtra in "" "-kd"  "-ki" 
   do
     for k in  $kopts
     do
       runtest $d1 $d2 $i $k $xtra
     done
   done
done
rm -f /tmp/dwa.$$
rm -f /tmp/dwb.$$
echo "base dwarfdump times"
python usertime.py $otimeout
echo "new  dwarfdump times"
python usertime.py $ntimeout
echo PASS $goodcount
echo FAIL $failcount

