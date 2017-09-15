#! /usr/bin/python
# Intended to strip uninteresting differences out
# of the output of a test run.
# What is uninteresting changes all the time,
# so this should be considered a skeleton of an application,
# not a just-use-it thing.

import sys


#Thu Nov 26 14:01:10 PST 2015
#175c175
#< .debug_string
#---
#> .debug_str
#FAIL -a -R -v -x line5=std sparc/tcombined.o


# This gets two input files and produces
# A set of diffs, as output.
# It is to produce the changes in the email list.

# Copyright 2008-2014 Golden Gate Lotus Club
#

# file1:
# Find the email in each line, remember the email and the line.
# file2: For each line. find the email and see if the email
# exists from the first file.
import sys

def tailmatches(tail,windowlen):
  ct = len(tail)
  #for l in tail:
  #   print "tailmatches?",l
  if len(tail) < int(windowlen):
     #print "tailmatch FAIL short list",ct
     return "n"
  
  #print "dadebug",tail[0].startswith("Thu ")
  if tail[0].startswith("Wed ") == 0: 
     #print "dadebug",tail[0].startswith("Tue ")
     #print "tailmatch FAIL 0",tail[0]
     return "n"
  wds = tail[1].strip().split("d")
  if len(wds) != 2:
     #print "tailmatch FAIL 1a",wds
     return "n"
  #if wds[0].isdigit() == False:
  #   print "tailmatch FAIL 1b",wds
  #   return "n"
  if wds[1].isdigit() == 0:
     #print "dadebug isdigit:",wds[1].isdigit()
     #print "tailmatch FAIL 1c",wds
     return "n"
     
  if tail[2].startswith("< .debug_macinfo") == 0: 
     #print "tailmatch FAIL 2",tail[1]
     return "n"
  if tail[3].startswith("<") == 0: 
     #print "tailmatch FAIL 3",tail[2]
     return "n"
  if tail[4].startswith("FAIL") == 0: 
     #print "tailmatch FAIL 4",tail[4]
     return "n"
  #print "PASS tailmatches!"
  return "y"

def splitwin(win,taillen):
   if len(win) < int(taillen):
      return (win,[])
   i = 0
   head = []
   tail = []
   lasthead = len(win) - int(taillen) - 1
   for l in win:
     if i < int(lasthead):
       #print "add to head: ",l
       head += [l]
     else:
       #print "add to tail: ",l
       tail += [l]
     i = i + 1
   return(head,tail)

def readaline(file):
  try:
    l = file.readline().strip()
  except EOFError:
    return ("y","")
  if len(l) < 1:
    return ("y","")
  #print ("n",l)
  return ("n",l)

def fillwindow(file, win,windowlen):
  donehere = "n"
  while donehere == "n":
    (donehere,l) = readaline(file)
    if donehere == "y":
      return(donehere,win)
    if l.startswith("FAIL") == 1:
      win += [l]
      break;
    win += [l]
  # At FAIL line, end of a sequence.
  (head,tail) = splitwin(win,windowlen)
  if tailmatches(tail,windowlen) == "y":
    #print "tailmatch"
    for l in head:
      print l
  else:
    #print "NO tailmatch"
    for l in head:
      print l
    for l in tail:
      print l
  return (donehere,[])
  
      

def readinfile(fname):
  try:
    file = open(fname,"r")
  except IOError, message:
    print >> sys.stderr , "File could not be opened: ", fname, " ", message
    sys.exit(1)

  windowlen=4
  curwindow = []
  done = "n" 
  while done == "n":
    (done,curwindow) = fillwindow(file,curwindow,windowlen)
    for l in curwindow:
        print l
  # done
    
if __name__ == '__main__':
  #cur = 1
  #print cur
  #while  len(sys.argv) > cur:
  #  print "argv[",cur,"] = ", sys.argv[cur]
  #  v = sys.argv[cur]
  #  cur = int(cur) +1
  #  sys.exit(1)
  readinfile("testALLdd")
  #readinfile("ALLdd")
  
