
/* Test code to verify the correct functioning of the
   dwarf 'harmless error' interfaces
   in libdwarf.
  
   On an irregular basis it is wise to run this under
   valgrind (or the like) to verify there is no space leakage.
*/


#include <stdio.h>
#include <dwarf.h>
#include <libdwarf.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef int (*tfunc)(unsigned int, const char **out);
static char *harmless[] = {
"test 0",
"test 1",
"test 2",
"test 3",
"test 4",
"test 5",
"test 6",
"test 7",
"test 8",
"test 9",
"test 10",
0
};
static const char *hbuf[40];

static Dwarf_Debug dbg = 0;

static void
refill_hbuff()
{
   unsigned bytecount = sizeof(hbuf);
   memset(hbuf,1,bytecount);
}

static int
run_test0()
{
    unsigned totalct = 0;
    int res = DW_DLV_ERROR;
    res = dwarf_get_harmless_error_list(dbg,4,&hbuf[0],&totalct);
    if(res != DW_DLV_NO_ENTRY) {
        printf("Failure, line %d\n",__LINE__);
        return 1;
    }
    return 0;
}
static int
run_test1()
{
    unsigned totalct = 0;
    int res = DW_DLV_ERROR;
    dwarf_insert_harmless_error(dbg,harmless[0]);
    res = dwarf_get_harmless_error_list(dbg,4,&hbuf[0],&totalct);
    if(res != DW_DLV_OK) {
        printf("Failure, expected OK result line %d\n",__LINE__);
        return 1;
    }
    if(totalct != 1) {
        printf("Failure, 1 error totalct result line %d\n",__LINE__);
        return 1;
    }
    if(strcmp(harmless[0],hbuf[0]) ) {
        printf("Failure,  \"%s\" vs \"%s\" 1 error wrong string line %d\n",
           harmless[0],hbuf[0],__LINE__);
        return 1;
    }
    if(hbuf[1] != 0) {
        printf("Failure, 1 error bad terminator line %d\n",__LINE__);
        return 1;
    }
    return 0; 
}
static int
run_test2()
{
    unsigned totalct = 0;
    int res = DW_DLV_ERROR;
    dwarf_insert_harmless_error(dbg,harmless[0]);
    dwarf_insert_harmless_error(dbg,harmless[1]);
    dwarf_insert_harmless_error(dbg,harmless[2]);
    dwarf_insert_harmless_error(dbg,harmless[3]);
    dwarf_insert_harmless_error(dbg,harmless[4]);
    res = dwarf_get_harmless_error_list(dbg,3,&hbuf[0],&totalct);
    if(res != DW_DLV_OK) {
        printf("Failure, expected OK result line %d\n",__LINE__);
        return 1;
    }
    if(strcmp(harmless[1],hbuf[0]) ) {
        printf("Failure,  \"%s\" vs \"%s\" 1 error wrong string line %d\n",
           harmless[1],hbuf[0],__LINE__);
        return 1;
    }
    if(strcmp(harmless[2],hbuf[1]) ) {
        printf("Failure,  \"%s\" vs \"%s\" 1 error wrong string line %d\n",
           harmless[2],hbuf[1],__LINE__);
        return 1;
    }
    if(hbuf[2] != 0) {
        printf("Failure, 1 error bad terminator line %d\n",__LINE__);
        return 1;
    }
    if(totalct != 5) {
        printf("Failure, totalct %d expected 5 %d\n",totalct,__LINE__);
        return 1;
    }
    res = dwarf_get_harmless_error_list(dbg,2,&hbuf[0],&totalct);
    if(res != DW_DLV_NO_ENTRY) {
        printf("Failure, 1 error expected no new entry line %d\n",__LINE__);
        return 1;
    }
    return 0; 
}
static int
run_test3()
{
    unsigned totalct = 0;
    int res = DW_DLV_ERROR;
    dwarf_insert_harmless_error(dbg,harmless[0]);
    dwarf_insert_harmless_error(dbg,harmless[1]);
    dwarf_insert_harmless_error(dbg,harmless[2]);
    dwarf_insert_harmless_error(dbg,harmless[3]);
    dwarf_insert_harmless_error(dbg,harmless[4]);
    res = dwarf_get_harmless_error_list(dbg,8,&hbuf[0],&totalct);
    if(res != DW_DLV_OK) {
        printf("Failure, expected OK result line %d\n",__LINE__);
        return 1;
    }
    if(totalct != 5) {
        printf("Failure, totalct %d expected 5 %d\n",totalct,__LINE__);
        return 1;
    }
    if(strcmp(harmless[1],hbuf[0]) ) {
        printf("Failure,  \"%s\" vs \"%s\" 1 error wrong string line %d\n",
           harmless[1],hbuf[0],__LINE__);
        return 1;
    }
    if(strcmp(harmless[2],hbuf[1]) ) {
        printf("Failure,  \"%s\" vs \"%s\" 1 error wrong string line %d\n",
           harmless[2],hbuf[1],__LINE__);
        return 1;
    }
    if(strcmp(harmless[3],hbuf[2]) ) {
        printf("Failure,  \"%s\" vs \"%s\" 1 error wrong string line %d\n",
           harmless[3],hbuf[2],__LINE__);
        return 1;
    }
    if(strcmp(harmless[4],hbuf[3]) ) {
        printf("Failure,  \"%s\" vs \"%s\" 1 error wrong string line %d\n",
           harmless[4],hbuf[3],__LINE__);
        return 1;
    }
    if(hbuf[4] != 0) {
        printf("Failure, 1 error bad terminator line %d\n",__LINE__);
        return 1;
    }
    res = dwarf_get_harmless_error_list(dbg,2,&hbuf[0],&totalct);
    if(res != DW_DLV_NO_ENTRY) {
        printf("Failure, 1 error expected no new entry line %d\n",__LINE__);
        return 1;
    }
    return 0;
}


static int
run_test4()
{
    unsigned totalct = 0;
    int res = DW_DLV_ERROR;
    int oldsize = 0;
    dwarf_insert_harmless_error(dbg,harmless[0]);
    dwarf_insert_harmless_error(dbg,harmless[1]);
    dwarf_insert_harmless_error(dbg,harmless[2]);
    dwarf_insert_harmless_error(dbg,harmless[3]);
    dwarf_insert_harmless_error(dbg,harmless[4]);
    oldsize = dwarf_set_harmless_error_list_size(dbg,2);
    if(oldsize != DW_HARMLESS_ERROR_CIRCULAR_LIST_DEFAULT_SIZE) {
        printf("Failure, expected  old size as default line %d vs %d %d\n",
            oldsize,DW_HARMLESS_ERROR_CIRCULAR_LIST_DEFAULT_SIZE,__LINE__);
        return 1;
    }
    
    res = dwarf_get_harmless_error_list(dbg,8,&hbuf[0],&totalct);
    if(res != DW_DLV_OK) {
        printf("Failure, expected OK result line %d\n",__LINE__);
        return 1;
    }
    if(totalct != 5) {
        printf("Failure, totalct %d expected 5 %d\n",totalct,__LINE__);
        return 1;
    }
    if(strcmp(harmless[3],hbuf[0]) ) {
        printf("Failure,  \"%s\" vs \"%s\" 1 error wrong string line %d\n",
           harmless[3],hbuf[0],__LINE__);
        return 1;
    }
    if(strcmp(harmless[4],hbuf[1]) ) {
        printf("Failure,  \"%s\" vs \"%s\" 1 error wrong string line %d\n",
           harmless[4],hbuf[1],__LINE__);
        return 1;
    }
    if(hbuf[2] != 0) {
        printf("Failure, 1 error bad terminator line %d\n",__LINE__);
        return 1;
    }
    res = dwarf_get_harmless_error_list(dbg,2,&hbuf[0],&totalct);
    if(res != DW_DLV_NO_ENTRY) {
        printf("Failure, 1 error expected no new entry line %d\n",__LINE__);
        return 1;
    }
    return 0;
}


int main()
{
    int errcount = 0;
    int res = DW_DLV_ERROR;
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errarg = 0;
    Dwarf_Error error = 0;
    const char *filepath= "./test_harmless";
    int fd = open(filepath,O_RDONLY);
    if(fd < 0) {
        printf("Failure attempting to open %s\n",filepath);
        exit(1);
    }
    res = dwarf_init(fd,DW_DLC_READ,errhand,errarg, &dbg,&error);
    if(res != DW_DLV_OK) {
        printf("Giving up, cannot do DWARF processing\n");
        exit(1);
    }

    refill_hbuff();
    errcount += run_test0(); 
    refill_hbuff();
    errcount += run_test1(); 
    refill_hbuff();
    errcount += run_test2(); 
    refill_hbuff();
    errcount += run_test3(); 
    refill_hbuff();
    errcount += run_test4(); 
    refill_hbuff();

    res = dwarf_finish(dbg,&error);
    if(res != DW_DLV_OK) {
        printf("dwarf_finish failed!\n");
    }
    close(fd);
    if(errcount > 0) {
        printf("FAIL harmless test, %d errors\n",errcount); 
    } else {
        printf("PASS harmless test\n");
    }
    exit(errcount> 0 ? 3:0);
}
