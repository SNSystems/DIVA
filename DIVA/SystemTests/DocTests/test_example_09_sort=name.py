expected = """\
{InputFile} "example_09.o"
    {CompileUnit} "example_09.cpp"

{Source} "example_09.cpp"
 4    {Class} "A"
 5      {Member} private "a" -> "int"
 2    {Alias} "CHAR" -> "char"
 8    {Variable} "a" -> "A"
10    {Function} "foo" -> "CHAR"
          - No declaration
12      {Variable} "c" -> "CHAR"
10      {Parameter} "p" -> "char *"
"""


def test(diva):
    assert diva('example_09.o --sort=name') == expected
