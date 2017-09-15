#!/bin/sh
trap "rm -f /tmp/dwxa.$$ ; rm -f /tmp/dwxb.$$ ; exit" 1 2 15
echo 'precheck for already running'
# Do this early before doing the build since
# the build will break any DWARFTEST.sh
# that is running.
rm -f /tmp/dwxa.$$
rm -f /tmp/dwxb.$$
ps -eaf |grep DWARF >/tmp/dwxa.$$
grep DWARFTEST.sh /tmp/dwxa.$$ > /tmp/dwxb.$$
ct=`wc -l </tmp/dwxb.$$`
echo "Number of DWARFTEST.sh running: $ct"
if [ $ct -gt 0 ]
then
  echo "Only one DWARFTEST.sh can run at a time on a machine"
  echo "Something is wrong, DWARFTEST.sh already running: $ct"
  echo "exit non-zero and stop"
  rm -f /tmp/dwxa.$$
  rm -f /tmp/dwxb.$$
  exit 1
fi
rm -f /tmp/dwxa.$$
rm -f /tmp/dwxb.$$
exit 0
