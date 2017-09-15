/*
  Copyright (c) 2010 David Anderson.
  All rights reserved.
 
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the example nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 
  THIS SOFTWARE IS PROVIDED BY David Anderson ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL David Anderson BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*/

// zero.cc  is a trivial application which enables one to take
// an arbitrary file (normally an object file used for testing DWARF)
// and replace areas of the file with binary zeros.
//
// The point of the replacement is to remove irrelevant and possibly
// proprietary instructions and data from the object files, leaving
// the Elf headers and the file size unchanged.
//
// This app is used once, typically before committing a new test object,
// to clean out the irrelevant .text and .data section contents.
// This app assumes some other application was used to determine
// what areas to zero out (the other app would usually be an object header
// dumper like readelf -S).

// Usage:
// zero -s <startoffset> -l <length> ... filename

#include <fcntl.h> // For open
#include <iostream>
#include <string>
#include <string>
#include <vector>
#include <errno.h>
#include <stdlib.h> // For exit()
#include <unistd.h> // For lseek

using std::string;
using std::cout;
using std::endl;
using std::vector;

int fd = -1;

typedef unsigned long counter;

#define ZEROSIZE 128000
char zerobuf[ZEROSIZE];

struct range {
    range(unsigned long o, unsigned long l) {offset = o;len = l; };
    range() {offset = 0; len = 0;};
    ~range() {};
    counter offset;
    counter len;
};

string filepath;

vector<range> ranges;

static void
usage()
{
   cout << "Usage: " << " zero -s <offset>  -l <len> ... filepath " <<endl;
   cout << " where ... means repeat the -s <offset> -l <len> quads as often as needed." 
        <<endl;
   cout << " Offset/length values may be decimal or with 0x prefix for hex" << endl; 
   cout << " Example: ./zero -s 0x1 -l 2 testdata" << endl;
}

counter getval(const string s)
{
       char *ep = 0;
       errno = 0;
       unsigned long long v = strtoull(s.c_str(),&ep,0);
       if (errno != 0) {
            cout << "The offset " << s << " is outside the representable value set. " <<endl;
            exit(1);
       }
       return v; 
}

/* Args are:
   appname
   -s start  -l len
   // the -s -l repeat as many times as wanted 
   filepath
*/
static void
validate_args(int argc, char **argv)
{
    int nextarg = 1;
    while( nextarg < (argc-1)) {
        string a = argv[nextarg];
        if( a != "-s") {
             cout << "Expected -s, got " << a <<endl;
             usage();
             exit(1);
        }
        ++nextarg;
        if (nextarg >= (argc-1)) {
             cout << "Expected start offset, got no argument " <<endl;
             usage();
             exit(1);
        }
        counter off = getval(argv[nextarg]);

        ++nextarg;
        if (nextarg >= (argc-1)) {
             cout << "Expected -l, got  no argument" << endl;
             usage();
             exit(1);
        }
        string l = argv[nextarg];
        if( l != "-l") {
             cout << "Expected -l, got " << l <<endl;
             usage();
             exit(1);
        }
        ++nextarg;
        if (nextarg >= (argc-1)) {
             cout << "Too few arguments." << endl;
             usage();
             exit(1);
        }
        counter len = getval(argv[nextarg]);
        range r(off,len);
        ranges.push_back(r);
        ++nextarg;
    }
    if (nextarg != (argc-1)) {
        usage();
        cout << "Expected file path , got no argument " <<endl;
             exit(1);
    }
    filepath = argv[nextarg];
};

static void
write_some_zeros(counter o, counter len)
{
     errno = 0;
     off_t res  = lseek(fd,o,SEEK_SET);
     if(res == (off_t)-1) {
          cout << "lseek  failed on " << filepath << " to offset " << o << endl;
          exit(1);
     }
     size_t remaining = len;
     cout << "Zeroing " << filepath << " offset " << o << " length " << len << endl;
     while( remaining) {
          size_t writesize = ZEROSIZE;
          if (writesize > remaining) {
             writesize = remaining;
          }
          errno = 0;
          ssize_t w = write(fd,zerobuf,writesize);
          if ( w < 0 ) {
              cout << "Write failed on " << filepath << " of " << writesize << 
                  " bytes, errno is " << errno << endl;
              exit(1);
          }
          if (w == 0) {
              cout << "Write produced no progress on " << filepath << " of " << writesize << 
                  " bytes, errno is " << errno << endl;
              exit(1);
          }
          remaining -= w;
     }
}

static void
write_all_zeros()
{
    for(unsigned i = 0; i < ranges.size(); ++i) {
         counter o = ranges[i].offset;
         counter l = ranges[i].len;
         if(l > 0) {
             write_some_zeros(o,l);
         }
    }
};

int 
main(int argc, char **argv)
{
    validate_args(argc,argv);
    errno = 0;
    fd = open(filepath.c_str(), O_RDWR);
    if(fd < 0) {
         cout << "Open failed on " << filepath << "  errno is " << errno<< endl;
         exit(1);
    }
    write_all_zeros();
    close(fd);
    return 0;
}

