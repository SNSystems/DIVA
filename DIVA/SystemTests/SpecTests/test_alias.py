expected_txt = """\
{InputFile} "alias.o"
   {CompileUnit} "alias.cpp"
     {PrimitiveType} -> "int"
         - 4 bytes

{Source} "alias.cpp"
3    {Alias} "INTEGER" -> "int"
5    {Function} "test" -> "void"
         - No declaration
6      {Variable} "x" -> "INTEGER"
"""


def test_txt(diva):
    assert diva('alias.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "alias.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "alias.cpp"
    type: null
    source:
      line: null
      file: null
    dwarf:
      offset: 0xb
      tag: "DW_TAG_compile_unit"
    attributes: {}
    children:
      - object: "PrimitiveType"
        name: null
        type: "int"
        source:
          line: null
          file: null
        dwarf:
          offset: 0x38
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Alias"
        name: "INTEGER"
        type: "int"
        source:
          line: 3
          file: "alias.cpp"
        dwarf:
          offset: 0x2d
          tag: "DW_TAG_typedef"
        attributes: {}
        children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 5
          file: "alias.cpp"
        dwarf:
          offset: 0x3f
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: false
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "Variable"
            name: "x"
            type: "INTEGER"
            source:
              line: 6
              file: "alias.cpp"
            dwarf:
              offset: 0x5c
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 5
          file: "alias.cpp"
        dwarf:
          offset: 0x0
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 7
          file: "alias.cpp"
        dwarf:
          offset: 0x4
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 7
          file: "alias.cpp"
        dwarf:
          offset: 0x7
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
"""


def test_yaml(diva):
    assert diva('alias.o --output=yaml') == expected_yaml
