import py


this_dir = py.path.local(__file__).dirpath()


def test_extra_ext(diva, tmpdir_autodel):
    this_dir.join('HelloWorld.o').copy(
        tmpdir_autodel.join('HelloWorld.o.elf'))
    assert diva('HelloWorld.o.elf', getelfs=False, cwd=tmpdir_autodel)


def test_other_dot(diva, tmpdir_autodel):
    this_dir.join('HelloWorld.o').copy(
        tmpdir_autodel.join('HelloWorld.nothing.o'))
    assert diva('HelloWorld.nothing.o', getelfs=False, cwd=tmpdir_autodel)
