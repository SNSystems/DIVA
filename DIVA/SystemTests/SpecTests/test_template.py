expected_txt = """\
           {InputFile} "template.o"

             {CompileUnit} "template.cpp"
               {PrimitiveType} -> "int"
                   - 4 bytes

  {Source} "template.cpp"
     3         {Class} "A"
     6         {Class} "C<int>"
                   - Template
                   - public "A"
                 {TemplateParameter} "C<int>::T" <- "int"
     9         {Struct} "S<int>"
                   - Template
                   - protected "A"
                 {TemplateParameter} "S<int>::T" <- "int"
    12         {Function} "func<int>" -> "void"
                   - No declaration
                   - Template
                 {TemplateParameter} "T" <- "int"
    14         {Function} "test" -> "void"
                   - No declaration
    15           {Variable} "c" -> "C<int>"
    16           {Variable} "s" -> "S<int>"

"""


def test_txt(diva):
    assert diva('template.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "template.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "template.cpp"
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
          offset: 0xc8
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Class"
        name: "A"
        type: null
        source:
          line: 3
          file: "template.cpp"
        dwarf:
          offset: 0x29
          tag: "DW_TAG_class_type"
        attributes:
          is_template: false
          inherits_from: []
        children: []
      - object: "Class"
        name: "C<int>"
        type: null
        source:
          line: 6
          file: "template.cpp"
        dwarf:
          offset: 0x2f
          tag: "DW_TAG_class_type"
        attributes:
          is_template: true
          inherits_from:
            - parent: "A"
              access_specifier: "public"
        children:
          - object: "TemplateParameter"
            name: "C<int>::T"
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x42
              tag: "DW_TAG_template_type_parameter"
            attributes:
              types:
                - "int"
            children: []
      - object: "Struct"
        name: "S<int>"
        type: null
        source:
          line: 9
          file: "template.cpp"
        dwarf:
          offset: 0x4a
          tag: "DW_TAG_structure_type"
        attributes:
          is_template: true
          inherits_from:
            - parent: "A"
              access_specifier: "protected"
        children:
          - object: "TemplateParameter"
            name: "S<int>::T"
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0x5d
              tag: "DW_TAG_template_type_parameter"
            attributes:
              types:
                - "int"
            children: []
      - object: "Function"
        name: "func<int>"
        type: "void"
        source:
          line: 12
          file: "template.cpp"
        dwarf:
          offset: 0x9f
          tag: "DW_TAG_subprogram"
        attributes:
          declaration:
            file: null
            line: null
          is_template: true
          static: false
          inline: false
          is_inlined: false
          is_declaration: false
        children:
          - object: "TemplateParameter"
            name: "T"
            type: null
            source:
              line: null
              file: null
            dwarf:
              offset: 0xc0
              tag: "DW_TAG_template_type_parameter"
            attributes:
              types:
                - "int"
            children: []
      - object: "Function"
        name: "test"
        type: "void"
        source:
          line: 14
          file: "template.cpp"
        dwarf:
          offset: 0x65
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
            name: "c"
            type: "C<int>"
            source:
              line: 15
              file: "template.cpp"
            dwarf:
              offset: 0x86
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
          - object: "Variable"
            name: "s"
            type: "S<int>"
            source:
              line: 16
              file: "template.cpp"
            dwarf:
              offset: 0x92
              tag: "DW_TAG_variable"
            attributes: {}
            children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 14
          file: "template.cpp"
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
          line: 17
          file: "template.cpp"
        dwarf:
          offset: 0x8
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
          line: 18
          file: "template.cpp"
        dwarf:
          offset: 0xd
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
          line: 18
          file: "template.cpp"
        dwarf:
          offset: 0x10
          tag: null
        attributes:
          NewStatement: true
          PrologueEnd: false
          EndSequence: true
          BasicBlock: false
          Discriminator: true
          EpilogueBegin: false
        children: []
      - object: "CodeLine"
        name: null
        type: null
        source:
          line: 12
          file: "template.cpp"
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
          line: 12
          file: "template.cpp"
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
          line: 12
          file: "template.cpp"
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
    assert diva('template.o --output=yaml') == expected_yaml
