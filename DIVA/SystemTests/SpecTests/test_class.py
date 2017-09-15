expected_txt = """\
           {InputFile} "class.o"

             {CompileUnit} "class.cpp"

  {Source} "class.cpp"
     3         {Class} "A"
     5         {Class} "B"
                   - public "A"
     7         {Class} "C"
                   - private "A"
     9         {Class} "D"
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
    assert diva('class.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "class.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "class.cpp"
    type: null
    source:
      line: null
      file: null
    dwarf:
      offset: 0xb
      tag: "DW_TAG_compile_unit"
    attributes: {}
    children:
      - object: "Class"
        name: "A"
        type: null
        source:
          line: 3
          file: "class.cpp"
        dwarf:
          offset: 0x2d
          tag: "DW_TAG_class_type"
        attributes:
          is_template: false
          inherits_from: []
        children: []
      - object: "Class"
        name: "B"
        type: null
        source:
          line: 5
          file: "class.cpp"
        dwarf:
          offset: 0x33
          tag: "DW_TAG_class_type"
        attributes:
          is_template: false
          inherits_from:
            - parent: "A"
              access_specifier: "public"
        children: []
      - object: "Class"
        name: "C"
        type: null
        source:
          line: 7
          file: "class.cpp"
        dwarf:
          offset: 0x45
          tag: "DW_TAG_class_type"
        attributes:
          is_template: false
          inherits_from:
            - parent: "A"
              access_specifier: "private"
        children: []
      - object: "Class"
        name: "D"
        type: null
        source:
          line: 9
          file: "class.cpp"
        dwarf:
          offset: 0x56
          tag: "DW_TAG_class_type"
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
          file: "class.cpp"
        dwarf:
          offset: 0x6f
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
              file: "class.cpp"
            dwarf:
              offset: 0x8c
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "b"
            type: "B"
            source:
              line: 13
              file: "class.cpp"
            dwarf:
              offset: 0x98
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "c"
            type: "C"
            source:
              line: 14
              file: "class.cpp"
            dwarf:
              offset: 0xa4
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "d"
            type: "D"
            source:
              line: 15
              file: "class.cpp"
            dwarf:
              offset: 0xb0
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 11
          file: "class.cpp"
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
          file: "class.cpp"
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
          file: "class.cpp"
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
    assert diva('class.o --output=yaml') == expected_yaml
