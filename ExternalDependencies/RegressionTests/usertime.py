import sys
import os

class urec:
  def __init__(self,title):
    self._title = title;
    self._usecs = 0.0
    self._wsecs = 0.0
    self._ssecs = 0.0
  def setusecs(self,t):
    self._usecs = float(t);
  def setwsecs(self,t):
    self._wsecs = float(t);
  def setssecs(self,t):
    self._ssecs = float(t);
  def uprint(self,h):
    print "%s u%6.2f s%6.2f w%6.2f %s " %(h, self._usecs,self._ssecs,self._wsecs,self._title)

def sort_class(mydata):
  """ Sort the list of objects by  zip
  """
  auxiliary = [ (x._wsecs,x) for x in mydata ]
  auxiliary.sort()
  return [ x[1] for x in auxiliary ]


def processfile():
  if len(sys.argv) != 2:
    print "Usage: python useritime.py <usagetimepath>"
    sys.exit(1)
  tfilename = sys.argv[1]
  try:
    file = open(tfilename,"r")
  except IOError, message:
    print >> sys.stderr , "File could not be opened: ", message
    sys.exit(1)
  linecount = 0
  maxusecs = 0.0
  maxwsecs = 0.0
  totusecs = 0.0
  totssecs = 0.0
  totwsecs = 0.0
  nzcount = 0
  allthree = "n"
  tcount = 0
  intrec = "n"
  title=''
  crec = ''
  reclist = []
  while 1:
    try:
      rec = file.readline()
    except EOFError:
        break
    if len(rec) < 1:
      # eof
      break
    linecount += 1
    if rec.startswith("====== ") == 1:
      # Start a new record.
      inrec = "y"
      tcount = int(tcount) +1
      wds = rec.strip().split()
      title = ' '.join((wds[1:]))
      allthree = "n"
      if crec != '':
        reclist += [crec]
      crec = urec(title)
      continue
    if inrec == "n":
      continue
    if rec.startswith("Command exited with non-zero status") == 1:
      nzcount = int(nzcount) + 1
      continue
    wds = rec.split()
    if rec.startswith("real ") == 1:
      # %e  wall clock seconds and fraction.
      try:
        t = float(wds[1])
      except ValueError,message:
        print "Bad value, line ",linecount,"record: ",rec
        t = 0.0
      crec.setwsecs(t)
      totwsecs = float(totwsecs) + t
      continue

    if rec.startswith("user ") == 1:
      # %U  user cpu seconds and fraction.
      try:
        t = float(wds[1])
      except ValueError,message:
        print "Bad value, line ",linecount,"record: ",rec
        t = 0.0
      totusecs = float(totusecs) + t
      crec.setusecs(t)
      if float(t) > float(maxusecs):
        maxusecs = t
      continue

    if rec.startswith("sys ") == 1:
      # %S  system cpu seconds and fraction.
      try:
        t = float(wds[1])
      except ValueError,message:
        print "Bad value, line ",linecount,"record: ",rec
        t = 0.0
      crec.setssecs(t)
      totssecs = float(totssecs) + t
      allthree  = "y"
      inrec = "n"
      continue
    # Else continue    
  if crec != '':
    reclist += [crec]
  file.close()
  return (tcount,nzcount,totusecs,totssecs,totwsecs,maxusecs,maxwsecs,reclist)
 


def printtop(recs,max):
  i = 0;
  for r in recs:
    r.uprint("")
    i = int(i) +1
    if i >= max:
        return
    

if __name__ == '__main__':
  (tcount,nzcount,usecs,ssecs,wsecs,maxu,maxw,reclist) = processfile()
  
  recs = sort_class(reclist)
  recs.reverse()
  printtop(recs,10)

  print "Count %5d  Seconds: usr %6.2f sys %6.2f wallclock %6.2f " %( tcount,usecs,ssecs,wsecs)
  print "    Non zero status count %d maxu %6.2f maxw %6.2f " % (nzcount,maxu,maxw)
  

