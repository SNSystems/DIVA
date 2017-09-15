import py

this_dir = py.path.local(__file__).dirpath()


def test(diva, tmpdir_autodel):
    subdir = tmpdir_autodel.join('sub')
    subdir.ensure(dir=True)

    subelf = subdir.join('HelloWorld.o')
    this_dir.join('HelloWorld.o').copy(subelf)

    assert diva(str(subelf), getelfs=False, cwd=tmpdir_autodel)
