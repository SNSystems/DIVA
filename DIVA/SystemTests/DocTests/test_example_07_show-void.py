expected = """\
           {InputFile} "example_07.o"

             {CompileUnit} "example_07.cpp"

  {Source} "example_07.cpp"
     2         {Variable} "pv" -> "void *"
     3         {Variable} "pi" -> "int *"
     4         {Function} "foo" -> "void"
                   - No declaration

"""


def test(diva):
    assert diva('--show-void example_07.o') == expected
