expected = """\

ERR_INVALID_FILE: Invalid input file 'invalidExtension.foo', please provide a file in a supported format.
"""


def test(diva):
    assert diva("invalidExtension.foo", nonzero=True) == (1, expected)
