expected = """\
           {InputFile} "example_14.o"

             {CompileUnit} "example_14.cpp"

  {Source} "example_14.cpp"
     2         {Struct} "bar"
     6           {Struct} "bar"
     9             {Struct} "bar"

"""


def test(diva):
    assert diva('example_14.o --show-none --show-struct') == expected
