import os
import py
import pytest
import shutil
import subprocess
import sys

@pytest.mark.skipif(not sys.platform.startswith("win"), reason="Windows only")
@pytest.mark.parametrize('output_type', ('text', 'yaml'))
@pytest.mark.parametrize('path_type', ('fwd_slash', 'back_slash', 'absolute'))
def test(diva, tmpdir_autodel, path_type, output_type):
    out_path = {
        'fwd_slash': 'long/{0}/{0}'.format('long_test_' * 13),
        'back_slash': 'long\\{0}\\{0}'.format('long_test_' * 13),
        'absolute': str(tmpdir_autodel) + '/long/{0}/{0}'.format(
            'long_test_' * 13),
    }[path_type]

    cmd = 'HelloWorld.o --output-dir={} --output={}'.format(out_path,
                                                            output_type)
    assert diva(cmd, cwd=tmpdir_autodel) == ''

    # Check the directory was created.
    full_dir_path = out_path.replace('/', '\\')
    if path_type != "absolute":
        full_dir_path = str(tmpdir_autodel) + '\\' + full_dir_path
    full_dir_path = '\\\\?\\' + full_dir_path
    assert os.path.isdir(full_dir_path)

    # Delete the directories that were added because the test framework can't.
    # Python 2.7 also can't run the following, so we call out to rmdir:
    # shutil.rmtree('\\\\?\\' + str(tmpdir_autodel.join('long')))
    subprocess.check_call(["rmdir", "/s", "/q",
                           str(tmpdir_autodel.join('long'))], shell=True)
