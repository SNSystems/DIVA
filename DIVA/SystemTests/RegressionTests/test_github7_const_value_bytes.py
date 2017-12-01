"""
See https://github.com/SNSystems/DIVA/issues/7
"""

expected = """\

Warning: Ignoring unrecognised DW_AT, DW_FORM combination 'DW_AT_const_value', 'DW_FORM_block1'.
{InputFile} "repro7.o"
   {CompileUnit} "repro.cpp"

{Source} "repro.cpp"
4    {Class} "c<3>"
         - Template
       {TemplateParameter} "c<3>::b" <- 
5    {Struct} "e<a>"
         - Template
       {TemplateParameter} "e<a>::d" <- "a"
6    {Variable} "f" -> "e<a>"
7    {Struct} "a"
8      {Member} public "g" -> "c<3>"
"""


def test(diva):
    assert diva("repro7.o") == expected
