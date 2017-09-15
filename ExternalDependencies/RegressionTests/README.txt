This directory (dwarftest) is the base which is used for
regression testing of libdwarf.    It will probably only
be usable on a POSIX compliant system (Unix or Linux or
possibly Mac).

Its primary problem is a lack of testing of the
dwarf-generation routines.  The libdwarf consumer functions
are pretty well tested.

The basic approach is simple: run each test with a
newly-compiled dwarfdump and with the previous known-good
version.  Compare the resulting text outputs and evaluate
the differences.

That comparison is only meaningful if one actually has
confidence that the previous version of dwarfdump and
dwarfdump2 is present and working correctly, but in spite of
this deficiency these tests seem adequate.

For a few tests that comparison simply does not work.  In a few
cases a special build of libdwarf is needed to test specific
functionality (the test setups do that for you).

The test will not work unless the full libdwarf/dwarfdump
source distribution is visible to the regression tests.
In addition to the packages needed to build dwarfdump etc,
the GNU binutils-dev or equivalent must be installed to get
bfd.h in a visible standard header location (the 'dwarfextract'
test will fail without GNU binutils-dev).

To run a full test:
	./configure
	make

If you are using a system with no pre-existing dwarfdump[2].O
to work with you must build the current dwarfdump[2].O
and hope that is a decent test.  It seems advisable to
move to keeping a baseline-output for at least some tests.
Many of the tests involve large amounts of dwarfdump output,
so keeping a baseline good result of text for those would
make the regression test repository much much larger. So when
choosing what to switch to 'baseline testing' with known-good
dwarfdump output will involve some careful consideration.

To clean up:
       make clean
To clean out the configure cache etc, getting back to
a clean distribution:
       make distclean


======================================================================
What follows is some background detail.

The key scripts are PICKUPBIN RUNALL.sh DWARFTEST.sh CLEANUP

PICKUPBIN:  this builds libdwarf and dwarfdump
and stores the result in the dwarftest directory.
The script assumes the directory tree has the following
directories directly under a main directory (the main
directory name is not important):
   libdwarf
   dwarfgen
   dwarfdump
   dwarfdump2

README.txt:  This file.

DWARFTEST.sh:  Runs a specific test set.

RUNALL.sh:  Runs 3 tests, running DWARFTEST.sh 3 times.  
   First comparing dwarfdump.O output vs 
   the new dwarfdump.   Second comparing dwarfdump2.O output vs
   the new dwarfdump2.   Third comparing the new dwarfdump vs
   the new dwarfdump2.   This third comparison is perhaps
   conceptually not needed, but it seems worthwhile to keep
   dwarfdump2 and dwarfdump output identical.

CLEANUP:  Cleans up all the temporary results from tests.
   Does not clean out files created by configure here.
   Use 'make distclean' to clean out those files.


The 'zero' directory holds a utility application that
enables scrubbing sections of object files to all-zero-bytes.
This is not used for testing.
This application exists so seemingly proprietary objects can be added to
the test without revealing any of the compiler-generated
actual instructions.   Basically one emails a contributor
the zero.cc source and lets the contributor remove instruction
bytes from the object (readelf -S is a convenient way to
get the lists of object offsets and lengths of sections).
Then the contributor provides the modified object to the
libdwarf project and we add test code using it to DWARFTEST.sh
Since libdwarf never pays any attention
to sections other than those named .eh_frame or .debug_*
one can use 'zero' to set those other section contents to all-zero
and preserve proprietary code generation information.  Thus
making it easier for people to contribute object files.


David Anderson May 2010

