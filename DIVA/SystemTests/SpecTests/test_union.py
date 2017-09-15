expected_txt = """\
           {InputFile} "union.o"

             {CompileUnit} "union.cpp"
               {PrimitiveType} -> "char"
                   - 1 bytes
               {PrimitiveType} -> "int"
                   - 4 bytes
               {PrimitiveType} -> "long int"
                   - 8 bytes

  {Source} "union.cpp"
     3         {Union} "U"
     4           {Member} public "a" -> "int"
     5           {Member} public "b" -> "char"
     6           {Member} public "c" -> "long int"
     9         {Function} "test" -> "void"
                   - No declaration
    10           {Variable} "u" -> "U"

"""


def test_txt(diva):
    assert diva('union.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "union.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "union.cpp"
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
        type: "char"
        source:
          line: null
          file: null
        dwarf:
          offset: 0x5a
          tag: "DW_TAG_base_type"
        attributes:
          size: 1
        children: []
      - object: "PrimitiveType"
        name: null
        type: "int"
        source:
          line: null
          file: null
        dwarf:
          offset: 0x53
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "PrimitiveType"
        name: null
        type: "long int"
        source:
          line: null
          file: null
        dwarf:
          offset: 0x61
          tag: "DW_TAG_base_type"
        attributes:
          size: 8
        children: []
      - object: "Union"
        name: "U"
        type: null
        source:
          line: 3
          file: "union.cpp"
        dwarf:
          offset: 0x2d
          tag: "DW_TAG_union_type"
        attributes:
          is_template: false
        children:
          - object: "Member"
            name: "a"
            type: "int"
            source:
              line: 4
              file: "union.cpp"
            dwarf:
              offset: 0x37
              tag: "DW_TAG_member"
            attributes:
              access_specifier: "public"
            children: []
          - object: "Member"
            name: "b"
            type: "char"
            source:
              line: 5
              file: "union.cpp"
            dwarf:
              offset: 0x40
              tag: "DW_TAG_member"
            attributes:
              access_specifier: "public"
            children: []
          - object: "Member"
            name: "c"
            type: "long int"
            source:
              line: 6
              file: "union.cpp"
            dwarf:
              offset: 0x49
              tag: "DW_TAG_member"
            attributes:
              access_specifier: "public"
            children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 9
          file: "union.cpp"
        dwarf:
          offset: 0x68
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
            name: "u"
            type: "U"
            source:
              line: 10
              file: "union.cpp"
            dwarf:
              offset: 0x85
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 9
          file: "union.cpp"
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
          line: 11
          file: "union.cpp"
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
          line: 11
          file: "union.cpp"
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
    assert diva('union.o --output=yaml') == expected_yaml
