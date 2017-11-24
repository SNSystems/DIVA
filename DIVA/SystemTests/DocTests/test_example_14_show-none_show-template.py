expected = """\
{InputFile} "example_14.o"
    {CompileUnit} "example_14.cpp"

{Source} "example_14.cpp"
15    {Class} "foo<bar>"
          - Template
        {TemplateParameter} "foo<bar>::_Ty" <- "bar"
15    {Class} "foo<nsp_1::bar>"
          - Template
        {TemplateParameter} "foo<nsp_1::bar>::_Ty" <- "nsp_1::bar"
15    {Class} "foo<nsp_1::nsp_2::bar>"
          - Template
        {TemplateParameter} "foo<nsp_1::nsp_2::bar>::_Ty" <- "nsp_1::nsp_2::bar"
"""


def test(diva):
    assert diva('example_14.o --show-none --show-template') == expected
