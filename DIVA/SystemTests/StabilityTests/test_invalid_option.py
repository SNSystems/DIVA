expected = """\

ERR_CMD_UNKNOWN_ARG: Unknown argument '--doesnotexist'.
"""


def test(diva):
    assert diva('dummy.elf --doesnotexist', nonzero=True, getelfs=False) == (
        1, expected)
