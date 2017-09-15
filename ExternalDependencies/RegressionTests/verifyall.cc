#include <iostream>
#include <string>
#include <string.h> // strncmp()
#include <stdlib.h> // exit()
#include <vector>
#include <ctype.h>

using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::vector;


// Read 'ALL' line by line
// which looks like this:
#if 0
=====  -a -R -v -v -v -v -v -v  mucci/stream.o
1006c1006
< arange starts at 0, length of 5000, cu_die_offset = 11 cuhdr 0
---
> arange starts at 0x0, length of 5000, cu_die_offset = 11 cuhdr 0
1021,1024c1021,1024
<     00000000:	<off cfa=00(r29) > 
<     00000004:	<off cfa=3312(r29) > 
<     00000014:	<off cfa=3312(r29) > <off r16=-32(cfa) > <off r28=-24(cfa) > <of
f r30=-16(cfa) > <off r31=-8(cfa) > 
<     00000018:	<off cfa=3312(r30) > <off r16=-32(cfa) > <off r28=-24(cfa) > <of
f r30=-16(cfa) > <off r31=-8(cfa) > 
---
>     0x00000000:	<off cfa=00(r29) > 
>     0x00000004:	<off cfa=3312(r29) > 
>     0x00000014:	<off cfa=3312(r29) > <off r16=-32(cfa) > <off r28=-24(cf
a) > <off r30=-16(cfa) > <off r31=-8(cfa) > 
>     0x00000018:	<off cfa=3312(r30) > <off r16=-32(cfa) > <off r28=-24(cf
a) > <off r30=-16(cfa) > <off r31=-8(cfa) > 
1042,1045c1042,1045
<     00000000:	<off cfa=00(r29) > 
<     00000004:	<off cfa=224(r29) > 
<     00000010:	<off cfa=224(r29) > <off r28=-32(cfa) > <off r30=-24(cfa) > <off
 r31=-16(cfa) > 
<     00000014:	<off cfa=224(r30) > <off r28=-32(cfa) > <off r30=-24(cfa) > <off
 r31=-16(cfa) > 
---
>     0x00000000:	<off cfa=00(r29) > 
>     0x00000004:	<off cfa=224(r29) > 
>     0x00000010:	<off cfa=224(r29) > <off r28=-32(cfa) > <off r30=-24(cfa
) > <off r31=-16(cfa) > 
>     0x00000014:	<off cfa=224(r30) > <off r28=-32(cfa) > <off r30=-24(cfa
) > <off r31=-16(cfa) > 
1056,1059c1056,1059
<     00000000:	<off cfa=00(r29) > 
<     00000004:	<off cfa=64(r29) > 
<     00000010:	<off cfa=64(r29) > <off r28=-32(cfa) > <off r30=-24(cfa) > <off 
r31=-16(cfa) > 
<     00000014:	<off cfa=64(r30) > <off r28=-32(cfa) > <off r30=-24(cfa) > <off 
r31=-16(cfa) > 
---
>     0x00000000:	<off cfa=00(r29) > 
>     0x00000004:	<off cfa=64(r29) > 
>     0x00000010:	<off cfa=64(r29) > <off r28=-32(cfa) > <off r30=-24(cfa)
 > <off r31=-16(cfa) > 
>     0x00000014:	<off cfa=64(r30) > <off r28=-32(cfa) > <off r30=-24(cfa)
 > <off r31=-16(cfa) > 
1072,1075c1072,1075
#endif

// Notice the difference is a single 0x pair is new in the > group

#define MYBUFSIZ 30000
char buf[MYBUFSIZ];
static void
showdiff(const string &ol,const string &nl,
   size_t index, size_t line , const char *t)
{
     cout << "Diff line " << line << " index " << index << " " << t << endl;
     cout << "< "<< ol << endl;
     cout << "> "<< nl << endl;
}

static int
docompare(vector<string> &olddata,vector<string>& newdata,int line)
{
    size_t os = olddata.size();
    size_t ns = newdata.size();
    size_t lim = (ns<os)? ns:os; 
    int err = 0;

    for(size_t i = 0; i < lim ; ++i) {
        string &ol = olddata[i];
        string &nl = newdata[i];
        size_t ols = ol.size();
        size_t nls = nl.size();
        if(nls != (ols +2)) {
             showdiff(ol,nl,i,line,"more than 2 chars different");
             ++err;
             continue;
        }
        size_t olsp = 0;
        size_t nlsp = 0;
        for ( size_t j = 0; ; ++j) {
//cout << "dadebug compare  old[" << olsp << "]=" << ol[olsp] << " new[" << nlsp << "]=" << nl[nlsp] << endl;
//cout << "dadebug \"" << nl.substr(nlsp,2) << "\"" << endl;
            if(string("0x") == nl.substr(nlsp,2)) {
               if(nl.substr(nlsp+2,1) == ol.substr(olsp,1) ) {
//cout << "dadebug skip 2 chars  pos << " << nlsp << endl;
                  nlsp+= 2;
                  continue;
               }
            }
            if(ol[olsp] != nl[nlsp]) {
                 cout << "char mismatch pos " << olsp << " " <<nlsp<< endl;
                 showdiff(ol,nl,i,line,"char mismatch");
                 ++err;
                 break;
            }
            nlsp++;
            olsp++;
            if(olsp == ols && nlsp == nls) {
                // Done with this line.
                //cout << "dadebug done with vec entry "<< i << endl;
                break;
            }
        } 
    }
    return err;
}

int main()
{
    vector<string> olddata;
    vector<string> newdata;
    bool inold = false;
    bool innew = false;
    int count = 0;
    int err = 0;
    for(;;) {
        cin.getline(buf,sizeof(buf));
        if(cin.eof()) {
            //cout << "eof" << endl;
            err += docompare(olddata,newdata,count);
            olddata.clear();
            newdata.clear();
            break;
        }
        if(cin.fail()) {
            cout << "input error" << endl;
            break;
        }
        ++count;
        //cout <<count << " " << buf <<endl;
        if(strncmp(buf,"=====",5) == 0) {
            olddata.clear();
            newdata.clear();
            innew = false;
            inold = false;
            continue;
        }
        if(strncmp(buf,"---",3) == 0) {
            if(inold) {
                inold = false;
                innew = true;
            }
            continue;
        }
        if(strncmp(buf,"< ",2) == 0) {
            if(innew) {
                cout << "BOTCH: Impossible, line " << count << endl;
                exit(1);
            }
            if(!inold) {
               inold=true;
            }
            olddata.push_back(&buf[2]);
            continue;
        }
        if(strncmp(buf,"> ",2) == 0) {
            if(inold) {
                cout << "BOTCH: impossible, line " << count << endl;
                exit(1);
            }
            if(!innew) {
               innew=true;
            }
            newdata.push_back(&buf[2]);
            continue;
        }
        //if(isdigit(buf[0])) {
        {
            if(!olddata.empty() || !newdata.empty()) {
                err += docompare(olddata,newdata,count);
                olddata.clear();
                newdata.clear();
            }
            innew = false;
            inold = false;
        }
    }
    if(err) {
        cout << " FAIL count " << err << " line count " << count << endl;
        exit(1);
    } 
    cout << "PASS (read " << count << " lines)." << endl;
    return 0;
}
