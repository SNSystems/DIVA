# Examples

This directory contains a set of example elf files (with corresponding source code) that can be used to explore the various features of DIVA.

A brief summary of what each example contains:

**helloworld**: The classic Hello, World
**example_01**: The bare minimum of debug information
**example_02**: Multidimensional arrays
**example_03**: A typedef, defined in a header file
**example_04**: A simple Class
**example_05**: A simple template
**example_06**: A typedef'd struct
**example_07**: Pointers
**example_08**: More complex templates and Classes
**example_09**: A typedef'd class
**example_10**: Combining multiple source files
**example_11**: A variety of types
**example_12**: Nested namespaces
**example_13**: An extern function and a for loop
**example_14**: Nested namespaces with structs, Classes, typedefs and templates
**example_15**: A method implementation
**example_16**: Local and global scope, and LTO
**good, broken, fixed**: Three versions of an object file, before during and after being broken
**scopes_org, scopes_mod**: The same source with some lines typedef'd out in scopes_mod for comparison purposes

## Rebuilding the examples

To rebuild the examples you will need Make and a C++ compiler (some examples require C++14 support):

```bash
make all
```

To rebuild the example_16 LTO example, you will need to use clang with a linker that supports Link Time Optimisation and then run:

```bash
make lto
```
