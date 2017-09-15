#!/usr/bin/python
#
#  Split ALLdd into groups of lines
#   with '====' at beginning of line, as head line.
#
#  If the word FAIL in caps appears, write the group to stdout.
# else move to next group.


import sys



def printblock(b,failcount_in):
  suppress = "y"
  failcount = failcount_in
  for l in b:
    if l.rfind("FAIL") != -1:
      failcount = int(failcount) +1
      suppress = "n"
      break
  if suppress == "n":
    for l in b:
      print l
  return failcount

if __name__ == '__main__':
  fn="ALLdd"
  count=0
  failcount = 0
  try:
    file = open(fn,"r")
  except IOError,message:
    print "FAIL open ",fn,"errormessage",message
    sys.exit(1)
  block = []
  while 1:
    try:
      rec = file.readline().strip()
    except EOFError:
        break
    if len(rec) < 1:
      # eof
      break
    if rec.startswith("====") == 1:
      failcount = printblock(block,failcount)
      count = int(count) +1
      block = []
      block += [rec]
    else:
      block += [rec]
  if len(block) > 0:
    failcount = printblock(block,failcount)

  print "testcount:",count
  print "failcount:",failcount
  





