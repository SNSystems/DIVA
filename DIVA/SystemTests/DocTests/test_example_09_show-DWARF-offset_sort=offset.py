expected = """\
              {InputFile} "example_09.o"
[0x0000000b]      {CompileUnit} "example_09.cpp"

              {Source} "example_09.cpp"
[0x00000026]   8    {Variable} "a" -> [0x0000003b]"A"
[0x0000003b]   4    {Class} "A"
[0x00000043]   5      {Member} private "a" -> [0x00000051]"int"
[0x00000058]  10    {Function} "foo" -> [0x00000096]"CHAR"
                        - No declaration
[0x00000079]  10      {Parameter} "p" -> [0x000000a8]"char *"
[0x00000087]  12      {Variable} "c" -> [0x00000096]"CHAR"
[0x00000096]   2    {Alias} "CHAR" -> [0x000000a1]"char"
"""


def test(diva):
    assert diva('example_09.o --show-DWARF-offset --sort=offset') == expected
