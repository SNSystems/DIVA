expected = """\
{InputFile} "example_13.o"
   {CompileUnit} "example_13.cpp"

{Source} "example_13.cpp"
4    {Function} "bar" -> "int"
         - No declaration
       {Block}
5        {Variable} "i" -> "int"
4      {Parameter} "A" -> "int *"
4      {Parameter} "X" -> "int"
"""


def test(diva):
    assert diva('example_13.o --show-block') == expected
