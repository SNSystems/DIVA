expected = """\
           {InputFile} "example_04.o"

             {CompileUnit} "example_04.cpp"

  {Source} "example_04.cpp"
    15           {Variable} "p1" -> "Point"
    16           {Variable} "p2" -> "Point"

  {Source} "iostream"
    74         {Variable} "std::__ioinit" -> "std::ios_base::Init"
    74           {Variable} "__ioinit" -> "std::ios_base::Init"

"""


def test(diva):
    assert diva('--show-none --show-variable example_04.o') == expected
