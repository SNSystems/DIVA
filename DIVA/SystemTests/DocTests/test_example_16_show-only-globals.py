expected = """\

"""


def test(diva):
    assert diva('example_16.elf --show-only-globals') == expected
