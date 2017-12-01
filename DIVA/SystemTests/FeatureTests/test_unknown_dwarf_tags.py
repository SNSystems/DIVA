"""
Test DIVA produces warnings for unknown DWARF tags (DW_TAG).

The test case unknown_tag.o was produced by modifying the byte at address
0x000000cd from 0x2e (DW_TAG_subprogram) to 0x0c (Reserved).
"""

expected = """\

Warning: Ignoring unknown/unsupported DWARF tag '0x000c'.
{InputFile} "unknown_tag.o"
   {CompileUnit} "helloworld.cpp"
"""


def test(diva):
    assert diva("unknown_tag.o") == expected
