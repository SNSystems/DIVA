expected_txt = """\
           {InputFile} "struct.o"

             {CompileUnit} "struct.cpp"

  {Source} "struct.cpp"
     3         {Struct} "A"
     5         {Struct} "B"
                   - public "A"
     7         {Struct} "C"
                   - private "A"
     9         {Struct} "D"
                   - public "B"
                   - protected "C"
    11         {Function} "test" -> "void"
                   - No declaration
    12           {Variable} "a" -> "A"
    13           {Variable} "b" -> "B"
    14           {Variable} "c" -> "C"
    15           {Variable} "d" -> "D"

"""


def test_txt(diva):
    assert diva('struct.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "struct.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "struct.cpp"
    type: null
    source:
      line: null
      file: null
    dwarf:
      offset: 0xb
      tag: "DW_TAG_compile_unit"
    attributes: {}
    children:
      - object: "Struct"
        name: "A"
        type: null
        source:
          line: 3
          file: "struct.cpp"
        dwarf:
          offset: 0x2d
          tag: "DW_TAG_structure_type"
        attributes:
          is_template: false
          inherits_from: []
        children: []
      - object: "Struct"
        name: "B"
        type: null
        source:
          line: 5
          file: "struct.cpp"
        dwarf:
          offset: 0x33
          tag: "DW_TAG_structure_type"
        attributes:
          is_template: false
          inherits_from:
            - parent: "A"
              access_specifier: "public"
        children: []
      - object: "Struct"
        name: "C"
        type: null
        source:
          line: 7
          file: "struct.cpp"
        dwarf:
          offset: 0x44
          tag: "DW_TAG_structure_type"
        attributes:
          is_template: false
          inherits_from:
            - parent: "A"
              access_specifier: "private"
        children: []
      - object: "Struct"
        name: "D"
        type: null
        source:
          line: 9
          file: "struct.cpp"
        dwarf:
          offset: 0x56
          tag: "DW_TAG_structure_type"
        attributes:
          is_template: false
          inherits_from:
            - parent: "B"
              access_specifier: "public"
            - parent: "C"
              access_specifier: "protected"
        children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 11
          file: "struct.cpp"
        dwarf:
          offset: 0x6e
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
            name: "a"
            type: "A"
            source:
              line: 12
              file: "struct.cpp"
            dwarf:
              offset: 0x8b
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "b"
            type: "B"
            source:
              line: 13
              file: "struct.cpp"
            dwarf:
              offset: 0x97
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "c"
            type: "C"
            source:
              line: 14
              file: "struct.cpp"
            dwarf:
              offset: 0xa3
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "d"
            type: "D"
            source:
              line: 15
              file: "struct.cpp"
            dwarf:
              offset: 0xaf
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 11
          file: "struct.cpp"
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
          line: 16
          file: "struct.cpp"
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
          line: 16
          file: "struct.cpp"
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
    assert diva('struct.o --output=yaml') == expected_yaml
