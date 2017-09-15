expected = """\
           {InputFile} "example_04.o"

             {CompileUnit} "example_04.cpp"

  {Source} "debug.h"
    54         {Namespace} "__gnu_debug"

  {Source} "c++config.h"
   184         {Namespace} "std"

  {Source} "debug.h"
    48           {Namespace} "std::__debug"

"""


def test(diva):
    assert diva('--show-none --show-namespace example_04.o') == expected
