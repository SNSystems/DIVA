expected = """\
{InputFile} "example_15.o"
    {CompileUnit} "example_15.cpp"

{Source} "example_15.cpp"
 2    {Class} "bar"
 4      {Function} "bar::foo" -> "void"
            - Is declaration
          {Parameter} -> "bar *"
 4    {Function} "foo" -> "void"
          - Declaration @ example_15.cpp,4
        {Parameter} "this" -> "bar *"
11    {Variable} "b" -> "bar"
"""


def test(diva):
    assert diva('example_15.o --show-combined') == expected
