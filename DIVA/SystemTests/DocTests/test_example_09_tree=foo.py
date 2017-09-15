expected = """\
           {InputFile} "example_09.o"

             {CompileUnit} "example_09.cpp"

  {Source} "example_09.cpp"
    10         {Function} "foo" -> "CHAR"
                   - No declaration
    10           {Parameter} "p" -> "char *"
    12           {Variable} "c" -> "CHAR"

"""


def test(diva):
    assert diva('example_09.o --tree=foo') == expected
