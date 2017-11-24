expected = """\
{InputFile} "all_objects.o"
    {CompileUnit} "all_objects.cpp"

     -------------------------------------
     Object                 Total  Printed
     -------------------------------------
     Alias                      1        0
     Block                      1        0
     Class                      1        0
     CodeLine                  27        0
     CompileUnit                1        1
     Enum                       1        0
     Function                   6        0
     Member                     3        0
     Namespace                  1        0
     Parameter                  9        0
     PrimitiveType              2        0
     Struct                     1        0
     TemplateParameter          1        0
     Union                      1        0
     Using                      2        0
     Variable                   7        0
     -------------------------------------
     Totals                    65        1

"""


def test(diva):
    assert diva("all_objects.o --show-summary --show-none") == expected
