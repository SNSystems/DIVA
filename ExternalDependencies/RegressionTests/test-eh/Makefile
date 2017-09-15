LIBELFDIR       = /usr
LIBELF_LIB      = $(LIBELFDIR)/lib
LIBELF_INC      = $(LIBELFDIR)/include

LIBDWARFDIR     = ../../../
LIBDWARF_LIB    =  ../../../libdwarf/libdwarf.a
LIBDWARF_INC    = ../../../include/

all : test-eh eh-frame

clean :
	rm -f test-eh eh-frame

test-eh : test-eh.c
	gcc -DUSE_ELF_POINTER -I$(LIBELF_INC) -I$(LIBDWARF_INC) -L$(LIBELF_LIB) -L$(LIBDWARF_LIB) -lelf -ldwarf -o $@ $< 

eh-frame : eh-frame.cc
	g++ -g -o $@ $^
