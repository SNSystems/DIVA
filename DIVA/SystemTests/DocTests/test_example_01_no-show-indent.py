expected = """\
{InputFile} "example_01.o"
   {CompileUnit} "example_01.cpp"

{Source} "example_01.cpp"
2  {Function} "foo" -> "void"
       - No declaration
2  {Parameter} "c" -> "char"
4  {Variable} "i" -> "int"
"""


def test(diva):
    assert diva('--no-show-indent example_01.o') == expected
