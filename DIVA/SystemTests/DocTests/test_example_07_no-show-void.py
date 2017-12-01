expected = """\
{InputFile} "example_07.o"
   {CompileUnit} "example_07.cpp"

{Source} "example_07.cpp"
2    {Variable} "pv" -> "*"
3    {Variable} "pi" -> "int *"
4    {Function} "foo" -> ""
         - No declaration
"""


def test(diva):
    assert diva('--no-show-void example_07.o') == expected
