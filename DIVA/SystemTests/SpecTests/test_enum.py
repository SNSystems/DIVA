expected_txt = """\
{InputFile} "enum.o"
    {CompileUnit} "enum.cpp"
      {PrimitiveType} -> "int"
          - 4 bytes
      {PrimitiveType} -> "short int"
          - 2 bytes
      {PrimitiveType} -> "unsigned int"
          - 4 bytes

{Source} "enum.cpp"
 3    {Enum} "E1" -> "unsigned int"
          - "A" = 0
          - "B" = 1
          - "C" = 2
 9    {Enum} "E2" -> "short int"
          - "G" = -1
          - "H" = 0
          - "I" = 1
15    {Enum} class "E3" -> "int"
          - "D" = 1
          - "E" = 2
          - "F" = 3
21    {Function} "test" -> "void"
          - No declaration
22      {Variable} "e1" -> "E1"
23      {Variable} "e2" -> "E2"
24      {Variable} "e3" -> "E3"
"""


def test_txt(diva):
    assert diva('enum.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "enum.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "enum.cpp"
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
          offset: 0x8f
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "PrimitiveType"
        name: null
        type: "short int"
        source:
          line: null
          file: null
        dwarf:
          offset: 0x6c
          tag: "DW_TAG_base_type"
        attributes:
          size: 2
        children: []
      - object: "PrimitiveType"
        name: null
        type: "unsigned int"
        source:
          line: null
          file: null
        dwarf:
          offset: 0x49
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Enum"
        name: "E1"
        type: "unsigned int"
        source:
          line: 3
          file: "enum.cpp"
        dwarf:
          offset: 0x2d
          tag: "DW_TAG_enumeration_type"
        attributes:
          class: false
          enumerators:
            - enumerator: "A"
              value: 0
            - enumerator: "B"
              value: 1
            - enumerator: "C"
              value: 2
        children: []
      - object: "Enum"
        name: "E2"
        type: "short int"
        source:
          line: 9
          file: "enum.cpp"
        dwarf:
          offset: 0x50
          tag: "DW_TAG_enumeration_type"
        attributes:
          class: false
          enumerators:
            - enumerator: "G"
              value: -1
            - enumerator: "H"
              value: 0
            - enumerator: "I"
              value: 1
        children: []
      - object: "Enum"
        name: "E3"
        type: "int"
        source:
          line: 15
          file: "enum.cpp"
        dwarf:
          offset: 0x73
          tag: "DW_TAG_enumeration_type"
        attributes:
          class: true
          enumerators:
            - enumerator: "D"
              value: 1
            - enumerator: "E"
              value: 2
            - enumerator: "F"
              value: 3
        children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 21
          file: "enum.cpp"
        dwarf:
          offset: 0x96
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
            name: "e1"
            type: "E1"
            source:
              line: 22
              file: "enum.cpp"
            dwarf:
              offset: 0xb3
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "e2"
            type: "E2"
            source:
              line: 23
              file: "enum.cpp"
            dwarf:
              offset: 0xbd
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "e3"
            type: "E3"
            source:
              line: 24
              file: "enum.cpp"
            dwarf:
              offset: 0xc7
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 21
          file: "enum.cpp"
        dwarf:
          offset: 0x0
          tag: null
        attributes:
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 25
          file: "enum.cpp"
        dwarf:
          offset: 0x4
          tag: null
        attributes:
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: false
          BasicBlock: false
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 25
          file: "enum.cpp"
        dwarf:
          offset: 0x7
          tag: null
        attributes:
          Discriminator: 0
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          EpilogueBegin: false
        children: []
"""


def test_yaml(diva):
    assert diva('enum.o --output=yaml') == expected_yaml
