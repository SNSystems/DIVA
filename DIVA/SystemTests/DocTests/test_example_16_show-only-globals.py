expected = """\
{InputFile} "example_16.elf"
"""


def test(diva):
    assert diva('example_16.elf --show-only-globals') == expected
