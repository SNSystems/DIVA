import py


this_dir = py.path.local(__file__).dirpath()

expected = """\
           {InputFile} "helloworld"

             {CompileUnit} "helloworld.cpp"

  {Source} "helloworld.cpp"
     5         {Function} "main" -> "int"
                   - No declaration

"""


def test(diva, tmpdir_autodel):
    this_dir.join('HelloWorld.o').copy(tmpdir_autodel.join('helloworld'))
    assert diva('helloworld', getelfs=False, cwd=tmpdir_autodel) == expected
