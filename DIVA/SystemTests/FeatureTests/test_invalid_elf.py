expected = """\

ERR_INVALID_FILE: Invalid input file 'not_an_elf.elf', please provide a file in a supported format.
"""


def test_not_a_file(diva):
    assert diva('not_an_elf.elf', nonzero=True) == (1, expected)
