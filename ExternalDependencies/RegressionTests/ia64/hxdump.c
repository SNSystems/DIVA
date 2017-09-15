/*
 This is the hex dump format I prefer.
  David B. Anderson
  Public Domain software.

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __linux__
#include <unistd.h>
extern char *optarg;
extern int optind, opterr, optopt;
#endif


#define OPTSTR  "ds:e:l:"

static int errflag;

char *Usage = 
{
        "Usage: hxdump [-d] [-s startaddr] [-e endaddr] [-l length] file ...\n"
        " -s supply a starting offset to use, for example, -s 0x1000\n"
        " -e supply an end address for the dump.\n"
        " -l supply a length for the dump\n"
        " -d display data only, not file offsets"
        "    (If both -e and -l supplied, take the shortest dump implied)\n"
        "    (dump length etc are not reset between files)\n"
        "    (use the usual C number conventions for numeric values)\n"
};

FILE *fin;
static void dumpfile(FILE *f,long offset,long endoffset);

int shownooffsets;

int
main(int argc, char **argv)
{
        long startoffset = 0;
        long endoffset = 0;
        long length = 0;
        long finalendoffset;
        int c;

        if(argc == 1) {
                printf("%s",Usage);
                exit(1);
        }

        while ((c = getopt(argc, argv, OPTSTR)) != EOF)
        {
                switch(c) {
                case 'e':
                          {
                            char *istr = optarg;
                            char *ostr = 0;
                            endoffset = strtoul(istr,&ostr,0);
                            if(endoffset == 0 && ostr == istr) {
                                printf("Error understanding -e value %s\n",
                                        istr);
                                return 1;
                            }
                        
                          }
                          break;
                case 'd':
                        shownooffsets = 1;
                        break;
                case 'l':
                          {
                            char *istr = optarg;
                            char *ostr = 0;
                            length = strtoul(istr,&ostr,0);
                            if(length == 0 && ostr == istr) {
                                printf("Error understanding -l value %s\n",
                                        istr);
                                return 1;
                            }
                        
                          }
                          break;
                case 's':
                        {
                         char *istr = optarg;
                         char *ostr = 0;
                         startoffset = strtoul(istr,&ostr,0);
                         if(startoffset == 0 && ostr == istr) {
                                printf("Error understanding offset %s\n",
                                        istr);
                                return 1;
                         }
                        }         
                        break;
                case '?':
                        return 1;
                }
        }
        for(; optind < argc;optind++)
        {
                struct stat statbuf;
                int res;
                if( (fin = fopen(argv[optind],"r")) == NULL)
                {
                        printf("Open of %s failed!\n",argv[optind]);
                        ++errflag;
                        continue;
                }
                res = fstat(fileno(fin), &statbuf);
                if(res != 0) {
                  printf("fstat of %s failed!\n",argv[0]);
                  fclose(fin);  
                  ++errflag;
                  continue;
                }
                if(statbuf.st_size <= startoffset) {
                  printf("Only %lu bytes in the file. "
                        "Starting at %lu (decimal) impossible!\n",
                        (unsigned long)statbuf.st_size,
                        (unsigned long)startoffset);
                  fclose(fin);
                  ++errflag;
                  continue;
                }
                res =fseek(fin,startoffset,SEEK_SET);
                if(res) {
                        printf("fseek to %ld failed!\n",(long)startoffset);
                        fclose(fin);
                        ++errflag;
                        continue;
                }
                finalendoffset = statbuf.st_size;
                if(endoffset != 0 && endoffset < finalendoffset){
                        finalendoffset = endoffset;
                }
                if(length != 0 && (startoffset + length) < finalendoffset) {
                        finalendoffset = startoffset + length;
                }
                dumpfile(fin,startoffset,finalendoffset);
                fclose(fin);
        }
        return(0);
} 


static void
dumpfile(FILE *f,long offset,long endoffset)
{
        char buf[100];
        int c;
        int i;
        int j;
        char *bp;
        int stop = 0;

        while(stop == 0 && feof(f) ==0)
        {
                if(shownooffsets) {
                   printf("       ");
                } else {
                   printf("%7lx",offset);
                }
                bp = buf;
                *bp = 0;
                for( i = 0; i < 4 && stop == 0; i++)
                {
                        printf(" ");
                        for(j = 0; j < 4; j++,offset++)
                        {
                                if(feof(f)) {
                                        printf("  ");
                                        stop = 1;
                                        break;
                                } else if (offset >= endoffset) {
                                        printf("  ");
                                        stop = 1;
                                        break;
                                } else {
                                  c = getc(f);
                                  if(c  != EOF) {
                                        printf("%02x",c);
                                        if( c < ' ' || c > '~')
                                                *bp = '.';
                                        else
                                                *bp = c;
                                        bp++;
                                        *bp = 0;
                                  } else {
                                        printf("  ");
                                  }
                                }
                        }
                }
                printf("  %s\n",buf);
        }
}

