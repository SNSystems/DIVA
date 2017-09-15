import py
import pytest
import sys
import subprocess

this_dir = py.path.local(__file__).dirpath()

expected = """\

ERR_VIEW_INVALID_OPEN: Unable to open file 'HelloWorld.o'.
"""


@pytest.mark.skipif(not sys.platform.startswith("win"), reason="Windows only")
def test(diva, tmpdir_autodel):
    cwd = tmpdir_autodel
    elf = cwd.join('HelloWorld.o')
    this_dir.join(elf.basename).copy(elf)

    icacls = "icacls {0}".format(elf)
    try:
        subprocess.check_call(icacls + " /deny Users:RX")
        assert (
            diva(elf.basename, getelfs=False, cwd=cwd, nonzero=True) ==
            (1, expected)
        )
    finally:
        subprocess.check_call(icacls + " /reset")
