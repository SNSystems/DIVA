expected = """\
{InputFile} "example_03.o"
   {CompileUnit} "example_03.cpp"

{Source} "example_03.cpp"
2    {Alias} "INTEGER" -> "int"
4    {Function} "foo" -> "void"
         - No declaration
4      {Parameter} "c" -> "char"
6      {Variable} "i" -> "INTEGER"
"""


def test(diva):
    assert diva('--show-alias example_03.o') == expected
