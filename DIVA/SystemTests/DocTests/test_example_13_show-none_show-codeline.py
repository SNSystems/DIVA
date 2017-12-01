expected = """\
{InputFile} "example_13.o"
   {CompileUnit} "example_13.cpp"

{Source} "example_13.cpp"
4    {CodeLine}
5    {CodeLine}
6    {CodeLine}
5    {CodeLine}
9    {CodeLine}
9    {CodeLine}
"""


def test(diva):
    assert diva('example_13.o --show-none --show-codeline') == expected
