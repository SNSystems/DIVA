expected = """\
           {InputFile} "example_01.o"

             {CompileUnit} "example_01.cpp"

  {Source} "example_01.cpp"
     2         {Function} "foo" -> "void"
                   - No declaration
     2           {Parameter} "c" -> "char"
     4           {Variable} "i" -> "int"

           {InputFile} "example_02.o"

             {CompileUnit} "example_02.cpp"

  {Source} "example_02.cpp"
     2         {Variable} "BLOCK" -> "char [10][4]"
"""

def test(diva):
    assert diva('example_01.o example_02.o ')
