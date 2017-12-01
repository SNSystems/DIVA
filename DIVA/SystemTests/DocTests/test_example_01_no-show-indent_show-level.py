expected = """\
    {InputFile} "example_01.o"
0      {CompileUnit} "example_01.cpp"

    {Source} "example_01.cpp"
1   2  {Function} "foo" -> "void"
           - No declaration
2   2  {Parameter} "c" -> "char"
2   4  {Variable} "i" -> "int"
"""


def test(diva):
    assert diva(
        '--no-show-indent --show-level example_01.o') == expected
