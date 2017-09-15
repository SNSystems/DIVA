expected = """\
           {InputFile} "example_09.o"

             {CompileUnit} "example_09.cpp"

  {Source} "example_09.cpp"
     4         {Class} "A"

"""


def test(diva):
    assert diva('example_09.o --show-none --show-class') == expected
