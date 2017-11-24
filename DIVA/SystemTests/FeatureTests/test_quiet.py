import pytest


@pytest.mark.parametrize('arg', ('-q', '--quiet'))
def test(diva, arg):
    assert diva("simple.o {}".format(arg)) == ''
