expected_txt = """\
{InputFile} "block.o"
    {CompileUnit} "block.cpp"
      {PrimitiveType} -> "int"
          - 4 bytes

{Source} "block.cpp"
 3    {Function} "test" -> "void"
          - No declaration
        {Block}
 5        {Variable} "x" -> "int"
        {Block}
 8        {Variable} "y" -> "int"
        {Block}
12        {Variable} "x" -> "int"
"""


def test_txt(diva):
    assert diva('block.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "block.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "block.cpp"
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
          offset: 0xb1
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 3
          file: "block.cpp"
        dwarf:
          offset: 0x2d
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
          - object: "Block"
            name: null
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x4e
              tag: "DW_TAG_lexical_block"
            attributes:
              try: false
              catch: false
            children:
              - object: "Variable"
                name: "x"
                type: "int"
                source:
                  line: 5
                  file: "block.cpp"
                dwarf:
                  offset: 0x63
                  tag: "DW_TAG_variable"
                attributes: {}
                children: []
          - object: "Block"
            name: null
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x70
              tag: "DW_TAG_lexical_block"
            attributes:
              try: false
              catch: false
            children:
              - object: "Variable"
                name: "y"
                type: "int"
                source:
                  line: 8
                  file: "block.cpp"
                dwarf:
                  offset: 0x85
                  tag: "DW_TAG_variable"
                attributes: {}
                children: []
          - object: "Block"
            name: null
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x92
              tag: "DW_TAG_lexical_block"
            attributes:
              try: false
              catch: false
            children:
              - object: "Variable"
                name: "x"
                type: "int"
                source:
                  line: 12
                  file: "block.cpp"
                dwarf:
                  offset: 0xa3
                  tag: "DW_TAG_variable"
                attributes: {}
                children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 3
          file: "block.cpp"
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
          line: 5
          file: "block.cpp"
        dwarf:
          offset: 0x8
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
          line: 6
          file: "block.cpp"
        dwarf:
          offset: 0xf
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
          line: 7
          file: "block.cpp"
        dwarf:
          offset: 0x31
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
          line: 8
          file: "block.cpp"
        dwarf:
          offset: 0x39
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
          line: 7
          file: "block.cpp"
        dwarf:
          offset: 0x40
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
          line: 12
          file: "block.cpp"
        dwarf:
          offset: 0x45
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
          line: 14
          file: "block.cpp"
        dwarf:
          offset: 0x4c
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
          line: 14
          file: "block.cpp"
        dwarf:
          offset: 0x4f
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
    assert diva('block.o --output=yaml') == expected_yaml
