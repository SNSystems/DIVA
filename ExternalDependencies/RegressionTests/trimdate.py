#!/usr/bin/python
#
# Any line starting with a day number (as abbrev)
# gets 'DATE' substitutied. Makes comparisons
# of some kinds easier.

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

def isdateitem(rec):
  if rec.startswith("old start "):
    return "y"
  if rec.startswith("old done "):
    return "y"
  if rec.startswith("new start "):
    return "y"
  if rec.startswith("new done "):
    return "y"
  if rec.startswith("Mon "):
    return "y"
  if rec.startswith("Tue "):
    return "y"
  if rec.startswith("Wed "):
    return "y"
  if rec.startswith("Thu "):
    return "y"
  if rec.startswith("Fri "):
    return "y"
  if rec.startswith("Sat "):
    return "y"
  if rec.startswith("Sun "):
    return "y"
  return "n"


if __name__ == '__main__':
  fn="ALLdd"
  if len(sys.argv) >1:
    fn = sys.argv[1]
  count=0
  failcount = 0
  print "trimdate on file",fn
  try:
    file = open(fn,"r")
  except IOError,message:
    print "FAIL open ",fn,"errormessage",message
    sys.exit(1)
  print "Start stripping dates"
  while 1:
    try:
      rec = file.readline().strip()
    except EOFError:
        break
    if len(rec) < 1:
      # eof
      break
     
    if isdateitem(rec) == "y":
       print "DATE:xxx"
    else:
       print rec
  print "Done stripping dates"




