DIVA - Debug information visual analyzer
----------------------------------------
* Diva is in beta.
* We will be regularly updating DIVA, with new features and bug fixes, so please watch this github project to be notified when these updates are available.

Introduction
------------
DIVA is a command line tool that processes DWARF debug information contained within ELF files and prints the semantics of that debug information. The DIVA output is designed with an aim to be understandable by software programmers without any low-level compiler or DWARF knowledge; as such, it can be used to report debug information bugs to the compiler provider. DIVA's output can also be used as the input to DWARF tests, to compare the debug information generated from multiple compilers, from different versions of the same compiler, from different compiler switches and from the use of different DWARF specifications (i.e. DWARF 3, 4 and 5). DIVA will be used on the LLVM project to test and validate the output of clang to help improve the quality of the debug experience.

Directories
-----------
* 3rd-party-libs - Source code of the 3rd party open source libraries that is used in Diva
* win64 - Windows build of DIVA, including examples and the user guide
* linux - An Ubuntu Linux (14.04) build of DIVA, including examples and the user guide (only tested on Ubuntu 14.04)

Feature requests, bug reports and general feedback/comments
-----------------------------------------------------------
Please create an "issue" in Github for any bug reports, feature requests and feedback:
	https://github.com/SNSystems/DIVA/issues

Features in the pipeline
------------------------
* YAML output
* Location lists for variables, parameters, members
* Support DWARF 5 input - for verification of the new DWARF 5 standard
* Open sourcing the DIVA code base - to allow open source LLVM compiler engineers to use. 
* Port to use the LLVM elf and dwarf libraries
* Support for 'golden' attributes

Known issues
------------
* The current DIVA implementation does not have the ability to mark individual DIVA objects as Golden. 
* For some specific namespace combinations, the DIVA output serialization fails, and a stack overflow error is seen.
* The sample for the --show-generated is invalid, as it does have compiler generated objects.
* The built-in Comparison Module will ignore DIVA objects with no line information will be ignored; this can therefore give false positives. A workaround is to use an external diff program if a more precise information is required.




