expected = """\
{InputFile} "example_12.o"
    {CompileUnit} "example_12.cpp"

{Source} "example_12.cpp"
 4        {Function} "A::B::foo" -> "void"
              - Is declaration
10        {Function} "A::C::bar" -> "void"
              - Is declaration
 4    {Function} "foo" -> "void"
          - Declaration @ example_12.cpp,4
10    {Function} "bar" -> "void"
          - Declaration @ example_12.cpp,10
"""


def test(diva):
    assert diva('example_12.o --show-none --show-function') == expected
