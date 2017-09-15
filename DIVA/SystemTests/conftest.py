import os
import py
import pytest
import re
import subprocess

from _pytest.config import UsageError

this_dir = py.path.local(__file__).dirpath()
object_re = re.compile(r'\.(o|elf|foo|cpp)$')


def pytest_addoption(parser):
    """
    Add a command line argument to provide DIVA's location
    """
    parser.addoption("--divadir", "--dd", action="store",
                     default=None, help="Set path to diva bin directory")


def pytest_collection_modifyitems(config):
    diva_path = config.getoption('--divadir')
    if diva_path:
        os.environ['PATH'] += os.pathsep + os.path.abspath(diva_path)

    try:
        subprocess.check_call(['diva', '--version'])
    except (subprocess.CalledProcessError, OSError):
        msg = "Can't find diva on PATH"
        if diva_path is None:
            msg += " and --divadir not provided"
        else:
            msg += " or in {} (provided with --divadir)".format(diva_path)
        raise UsageError(msg)


@pytest.fixture()
def diva(tmpdir_autodel, request):
    def _diva(command_line, nonzero=False, getelfs=True, cwd=None):
        if cwd is None:
            cwd = tmpdir_autodel
        try:
            command = ['diva'] + command_line
        except TypeError:
            command = ['diva'] + command_line.strip().split(' ')

        if getelfs:
            for name in command[1:]:
                if not object_re.search(name):
                    continue
                # Look for object in test's directory, then in Examples
                # request is a pytest fixture that provides metadata about the
                # currently running test
                test_dir = py.path.local(request.fspath).dirpath()

                object_path = test_dir.join(name)
                if not object_path.check(file=True):
                    object_path = this_dir.dirpath('Examples', name)
                assert object_path.check(),\
                    "Can't find {}".format(name)
                object_path.copy(cwd.join(name))

        proc = subprocess.Popen(
            command,
            cwd=str(cwd),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
        )
        stdout, _ = proc.communicate()
        if nonzero:
            return (proc.returncode, stdout)

        assert proc.returncode == 0, "Process output:\n" + stdout
        return stdout

    return _diva


@pytest.fixture()
def tmpdir_autodel(tmpdir):
    yield tmpdir
    tmpdir.remove()
