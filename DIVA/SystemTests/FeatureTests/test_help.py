import pytest


help_tests = (
    ('--help', """\
Usage: Diva [options] input_file [input_file...]

General options
  -h  --help                   Display basic help information
      --help-more              Display more option information
      --help-advanced          Display advanced option information
  -v  --version                Display the version information
  -q  --quiet                  Suppress output to stdout

Output options
  -a  --show-all               Print all (expect advanced) objects and
                               attributes
  -b  --show-brief             Print all common objects and attributes (default)
  -t  --show-summary           Print the summary table
  -d  --output-dir[=<dir>]     Print the output into a directory with each
                               compile unit's output in a separate file. If no
                               dir is given, then diva will use the input_file
                               string to create an output directory.
      --output=<text|yaml>     A comma separated list of output formats.

Sort options
      --sort=<line|name|offset>
                               Key used when ordering the output objects within
                               the same block. By default the key is "line".

Filter options
      --filter=<text>          Only show objects with <text> as its instance
                               name. The output will only include exact matches,
                               Regex is accepted. Multiple filters can be given
                               to show objects that match either.
      --filter-any=<text>      Only show objects with <text> in their instance
                               name.
      --tree=<text>            Same as --filter, except the whole subtree of any
                               matching object will printed.
      --tree-any=<text>        Same as --filter-any with the whole subtree.
"""),
    ('--help-more', """\
Usage: Diva [options] input_file [input_file...]

More object options
      --show-none              Print no objects, use this to hide all default
                               objects and then individual objects can be added
                               to the output using the options below.

The following options can be used to show/hide specific objects and attributes.
If two or more options conflict, precedence is given to the latter of the
conflicting options.

To hide a specific object, insert 'no-' after the '--'.
Example: for "--show-block --no-show-block", DIVA will not print any block
         objects.
         for "--show-all --no-show-block", DIVA will show all common objects
         except blocks.
         for "--show-none --show-block", DIVA will only show blocks.

The options labeled with a star (*) are enabled by default or when
"--show-brief" is given and all the following options are enabled when
"--show-all" is given.

      --show-alias             Print alias*
      --show-block             Print blocks*
      --show-block-attributes  Print block attributes (e.g. try,catch)
      --show-class             Print classes*
      --show-enum              Print enums*
      --show-function          Print functions*
      --show-member            Print class members*
      --show-namespace         Print namespaces*
      --show-parameter         Print parameters*
      --show-primitivetype     Print primitives
      --show-struct            Print structures*
      --show-template          Print templates*
      --show-union             Print unions*
      --show-using             Print using instances*
      --show-variable          Print variables*
"""),
    ('--help-advanced', """\
Usage: Diva [options] input_file [input_file...]

Advanced object options

The following options can be used to add additional (advanced) attributes and
flags to the output. These options are only considered useful for toolchain-
developers who are working at the DWARF level, and they are not enabled by
"--show-brief" or "--show-all".

To disable an option, insert 'no-' after the '--'.

      --show-codeline          Print code lines
      --show-codeline-attributes
                               Print code line attributes (NewStatement,
                               PrologueEnd)
      --show-combined          Print combined scope attributes
      --show-DWARF-offset      Print DWARF offsets
      --show-DWARF-parent      Print parent object DWARF offsets
      --show-DWARF-tag         Print DWARF tag attribute
      --show-generated         Print compiler generated attributes
      --show-global            Print "global" attributes
      --show-indent            Print indentations to reflect context (default
                               on)
      --show-level             Print lexical block levels
      --show-only-globals      Print only "global" objects
      --show-only-locals       Print only "local" objects
      --show-qualified         Print "qualified name" attributes*
      --show-void              Print "void" type attributes (default on)
      --show-zero              Print zero line number attributes
"""),
)


@pytest.mark.parametrize('args, expected', help_tests)
def test(diva, args, expected):
    assert diva(args) == expected


expected_noargs = """\
Usage: Diva [options] input_file [input_file...]

General options
  -h  --help                   Display basic help information
      --help-more              Display more option information
      --help-advanced          Display advanced option information
  -v  --version                Display the version information
  -q  --quiet                  Suppress output to stdout
"""


def test_noargs(diva):
    assert diva([], nonzero=True) == (1, expected_noargs)
