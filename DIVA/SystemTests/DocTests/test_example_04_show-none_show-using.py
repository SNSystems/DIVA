expected = """\
           {InputFile} "example_04.o"

             {CompileUnit} "example_04.cpp"

  {Source} "example_04.cpp"
     3         {Using} namespace "std"

  {Source} "debug.h"
    56           {Using} namespace "__debug"

"""


def test(diva):
    assert diva('--show-none --show-using example_04.o') == expected
