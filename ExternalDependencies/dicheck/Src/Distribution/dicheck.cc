/*
    Copyright (c) 2011 David Anderson.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    *   Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
    *   Neither the name of the example nor the
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
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
    OF SUCH DAMAGE.
*/

// David's  Indent Checker.
// It's unsophisticated and does no real parsing.
// It does not change the target source. It just
// reads the list of files named and reports on all lines
// that do not have a certain format to the beginnings of lines.
//
// It reports any lines not following:
//   Lines start with space characters, no tabs.
//   Non-space characters begin on column 0,4,8,12, etc.
//     (or think of these as 1,5,9,13, etc.)
//   Indentation increases by 4 if it increases.
//   Indentation decreases by any amount.
//   Any line with trailing whitespace.
//
// To build try:
//      g++ dicheck.cc -o dicheck
// or some equivalent C++ compile command.
// It will not work on Macintosh OSX files
// unless you modify this source as it does not
// treat the '\r' (CarriageReturn) character as end-of-line.
// It should work on  CRLF or POSIX LF line endings
// without change.


#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>  // for exit()
#include <stdlib.h>
//#include <libgen.h> /* for basename */

using std::ofstream;
using std::ifstream;
using std::string;
using std::cout;
using std::endl;
using std::cin;

// Usage:  dicheck  file ...

unsigned indentamount = 4;
bool showtrailing (false);
bool dotrailing   (false);
unsigned errcount = 0;

static string
mybasename(char *in)
{
    //string out(basename(in));
    string out(in);
    return out;
}

static void
processfile(string &path, ifstream * mystream,ofstream *fileout)
{
    char c;
    char outbuf[2000];
    unsigned inpos = 0;
    unsigned outpos = 0;
    unsigned trailingwhitespace = 0;
    unsigned line = 1;
    unsigned lastlineindent = 0;
    bool skipping = false;
    bool lastlinemacro = false;
    bool curlinemacro = false;
    bool blankline = true;
    while ( mystream->get(c)) {
        switch(c) {
        case '\r':
            // Does not handle MacOS line ends.
            inpos++;
            outbuf[outpos++] = c;
            break;
        case '\n':
            if(!blankline) {
                lastlinemacro = curlinemacro;
            }
            if(dotrailing) {
                fileout->write(outbuf,outpos-trailingwhitespace);
                fileout->put('\n');
            } else if (showtrailing  && trailingwhitespace) {
                cout << line << ":" << inpos << " of " << path <<
                    " has " << trailingwhitespace <<
                    " whitespace chars on the end. " << endl;
                errcount++;
            }
            skipping = false;
            line++;
            inpos = 0;
            outpos = 0;
            blankline = true;
            curlinemacro = false;
            trailingwhitespace = 0;
            break;
        case '\v': // Vertical tab.
            trailingwhitespace += 1;
            outbuf[outpos++] = c;
            inpos++;
            break;
        case ' ':
            trailingwhitespace += 1;
            inpos++;
            outbuf[outpos++] = c;
            break;
        case '\t':
            trailingwhitespace += 1;
            if(skipping) {
                inpos++;
                outbuf[outpos++] = c;
                break;
            }
            if(!fileout) {
                cout << line << ":" << inpos << " of " << path <<
                    " is a tab. " << endl;
            }
            inpos++;
            outbuf[outpos++] = c;
            lastlineindent = inpos;
            skipping = true;
            break;
        case '#':
            trailingwhitespace = 0;
            blankline = false;
            if(skipping) {
                inpos++;
                outbuf[outpos++] = c;
                break;
            }
            if(inpos == 0) {
                // Macro of some kind. Indent 0 ok.
                // Do not count this indent, really.
                curlinemacro = true;
            } else {
                lastlineindent = indentamount;
            }
            inpos++;
            outbuf[outpos++] = c;
            skipping = true;
            break;
        default:
            trailingwhitespace = 0;
            blankline = false;
            if(skipping) {
                inpos++;
                outbuf[outpos++] = c;
                break;
            }
            if(inpos%indentamount) {
                if(inpos == 2 || inpos == 3) {
                    // For our copyright and other comment blocks.
                    lastlineindent = indentamount;
                    inpos++;
                    outbuf[outpos++] = c;
                    skipping = true;
                    break;
                }
                if(!fileout) {
                    cout << line << ":" << inpos << " of " << path <<
                        " has a bad indent. " << endl;
                    errcount++;
                }
            } else {
                if(!fileout) {
                    if (inpos == lastlineindent) {
                        // OK.
                    } else if ( inpos < lastlineindent) {
                        // OK. we allow skipping back a fair amount due
                        // to function calls adding nesting.
                    } else if ( (inpos >= indentamount) &&
                        (((inpos - indentamount) == lastlineindent) ||
                            (lastlinemacro)) ) {
                        // OK.
                    } else {
                        cout << line << ":" << inpos << " of " << path <<
                            " has a bad indent change, last indent  " <<
                            lastlineindent << "  cur indent " << inpos << endl;
                        errcount++;
                    }
                }
            }
            lastlineindent = inpos;
            inpos++;
            outbuf[outpos++] = c;
            skipping = true;
            break;
        }
    }
    return;
}

int
main(int argc, char**argv)
{
    string basenam = mybasename(argv[0]);
    if(basenam == "trimtrailing") {
        dotrailing = true;
        showtrailing = false;
    }
    if(argc == 1) {
        if(dotrailing) {
            cout << "dicheck [-t] file ..." << endl;
            cout << "  where -t means notice trailing whitespace" << endl;
            cout << "Named files required as arguments" << endl;
            cout << "See also trimtrailing" << endl;
        } else {
            cout << "trimtrailing  file" << endl;
            cout << "Named files required as arguments" << endl;
            cout << " which copies file to file.out dropping trailing whitespace" << endl;
            cout << "See also dicheck" << endl;
        }
    } else {
        int i = 1;
        for(; i < argc; ++i) {
            string f(argv[i]);
            if (f == "-t") {
                if( dotrailing) {
                    cout << "Option -t not possible in trimtrailing" << endl;
                    exit(1);
                }
                showtrailing = true;
                ++i;
                break;
            }
            break;
        }
        for(; i < argc; ++i) {
            string f(argv[i]);
            ifstream ist(f.c_str());
            if(!ist) {
                cout << "Cannot open " << f << endl;
                exit(1);
            }
            if (dotrailing) {
                string outname = f + ".out";
                ofstream fout(outname.c_str());
                if(!fout) {
                    cout << "Cannot open output " << outname << endl;
                    exit(1);
                }
                processfile(f,&ist,&fout);
                fout.flush();
            }else {
                processfile(f,&ist,0);
            }
        }

    }
    if (errcount) {
        exit(1);
    }
    exit(0);
}

