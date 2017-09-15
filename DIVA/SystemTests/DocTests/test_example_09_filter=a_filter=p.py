expected = """\
           {InputFile} "example_09.o"

  {Source} "example_09.cpp"
     5           {Member} private "a" -> "int"
     8         {Variable} "a" -> "A"
    10           {Parameter} "p" -> "char *"

"""


def test(diva):
    assert diva('example_09.o --filter=a --filter=p') == expected
