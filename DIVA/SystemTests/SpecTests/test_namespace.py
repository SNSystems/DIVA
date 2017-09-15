expected_txt = """\
           {InputFile} "namespace.o"

             {CompileUnit} "namespace.cpp"
               {PrimitiveType} -> "int"
                   - 4 bytes

  {Source} "namespace.cpp"
     3         {Namespace} "NS"
     4           {Namespace} "NS::Inner"
     5             {Variable} "g" -> "int"
     5         {Variable} "NS::Inner::g" -> "int"

"""


def test_txt(diva):
    assert diva('namespace.o --show-all --output=text') == expected_txt


expected_yaml = """\
input_file: "namespace.o"
output_version: "0.1"
objects:
  - object: "CompileUnit"
    name: "namespace.cpp"
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
          offset: 0x3d
          tag: "DW_TAG_base_type"
        attributes:
          size: 4
        children: []
      - object: "Namespace"
        name: "NS"
        type: null
        source:
          line: 3
          file: "namespace.cpp"
        dwarf:
          offset: 0x1d
          tag: "DW_TAG_namespace"
        attributes: {}
        children:
          - object: "Namespace"
            name: "NS::Inner"
            type: null
            source:
              line: 4
              file: "namespace.cpp"
            dwarf:
              offset: 0x27
              tag: "DW_TAG_namespace"
            attributes: {}
            children:
              - object: "Variable"
                name: "g"
                type: "int"
                source:
                  line: 5
                  file: "namespace.cpp"
                dwarf:
                  offset: 0x2e
                  tag: "DW_TAG_variable"
                attributes: {}
                children: []
      - object: "Variable"
        name: "NS::Inner::g"
        type: "int"
        source:
          line: 5
          file: "namespace.cpp"
        dwarf:
          offset: 0x44
          tag: "DW_TAG_variable"
        attributes: {}
        children: []
"""


def test_yaml(diva):
    assert diva('namespace.o --output=yaml') == expected_yaml
