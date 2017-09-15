import re
import sys


def test(diva):
    actual = diva('--version')

    if sys.platform.startswith("win"):
        platform_regex = "Win(32|64)"
    else:
        platform_regex = "Linux(32|64)"
    expected_regex = (
        r"^DIVA - Debug Information Visual Analyzer - " +
        r"version (\d+\.\d+\.\d+\.\d+|DEV) - " +
        platform_regex + r"( \(Debug\))?\n" +
        r"Copyright \(C\) 20\d\d Sony Interactive Entertainment Inc\. "
        r"All Rights Reserved\.\n?"
    )
    assert re.match(expected_regex, actual), \
        "Version output doesn't match expected pattern."
