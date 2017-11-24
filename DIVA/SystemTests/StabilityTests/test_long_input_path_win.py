import os
import py
import pytest
import shutil
import subprocess
import sys

this_dir = py.path.local(__file__).dirpath()

expected = """\
{InputFile} "<PATH>"
   {CompileUnit} "helloworld.cpp"

{Source} "helloworld.cpp"
5    {Function} "main" -> "int"
         - No declaration
"""

@pytest.mark.skipif(not sys.platform.startswith("win"), reason="Windows only")
@pytest.mark.parametrize('path_type', ('fwd_slash', 'back_slash', 'absolute'))
def test(diva, tmpdir_autodel, path_type):
    in_path = {
        'fwd_slash': 'long/{0}/{0}'.format('long_test_' * 13),
        'back_slash': 'long\\{0}\\{0}'.format('long_test_' * 13),
        'absolute': str(tmpdir_autodel) + '/long/{0}/{0}'.format(
            'long_test_' * 13),
    }[path_type]

    full_dir_path = in_path.replace('/', '\\')
    if path_type != "absolute":
        full_dir_path = str(tmpdir_autodel) + '\\' + full_dir_path
    full_dir_path = '\\\\?\\' + full_dir_path

    os.makedirs(full_dir_path)
    this_dir.join('HelloWorld.o').copy(py.path.local(full_dir_path))

    cmd_path = os.path.join(in_path, 'HelloWorld.o')
    cmd = [cmd_path]
    assert (diva(cmd, cwd=tmpdir_autodel, getelfs=False) ==
            expected.replace("<PATH>", cmd_path))

    # Delete the directories that were added because the test framework can't.
    # Python 2.7 also can't run the following, so we call out to rmdir:
    # shutil.rmtree('\\\\?\\' + str(tmpdir_autodel.join('long')))
    subprocess.check_call(["rmdir", "/s", "/q",
                           str(tmpdir_autodel.join('long'))], shell=True)
