# DIVA - Debug Information Visual Analyzer

DIVA is a command line tool that processes DWARF debug information contained within ELF files and prints the semantics of that debug information. The DIVA output is designed to be understandable by software programmers without any low-level compiler or DWARF knowledge; as such, it can be used to report debug information bugs to the compiler provider. DIVA's output can also be used as the input to DWARF tests, to compare the debug information generated from multiple compilers, from different versions of the same compiler, from different compiler switches and from the use of different DWARF specifications (i.e. DWARF 3, 4 and 5). DIVA will be used on the LLVM project to test and validate the output of clang to help improve the quality of the debug experience.

## User Guide

For instructions on using DIVA please refer to the [user guide](./DIVA/Documentation/user_guide.md).

## Feature requests, bug reports and general feedback/comments

Please create an [issue in Github](https://github.com/SNSystems/DIVA/issues) for any bug reports, feature requests and feedback.

Please see the [projects list](https://github.com/SNSystems/DIVA/projects) for a backlog of ideas for feature items.

## Building DIVA

DIVA is built using CMake on Windows and Linux, so you will need to [install CMake first](https://cmake.org/install/). You will need version 3.2.2 or later.

### Linux

On Linux you will need either GCC or Clang installed (other compilers might work but haven't been tested). It must be a version that supports C++14.

```bash
cd DIVA
mkdir build
cd build
# OPTIONAL: to build with libc++ rather than libstdc++
export CXXFLAGS=-stdlib=libc++
# You can change the build type to "Release"
cmake .. -DCMAKE_BUILD_TYPE=Debug
# For subsequent builds you only need to run this second cmake command again
cmake --build .
```

You will find the resulting binary, and the shared libraries it needs, in `build/bin/`.

### Windows

On Windows you will need Visual Studio 2015 installed (later versions may work but haven't been tested).

```posh
cd DIVA
mkdir build
cd build
# You can change the -A argument to "x86" for a 32 bit build
cmake .. -A x64
# For subsequent builds you only need to run this second cmake command again
# You can change the --config argument to "Release"
cmake --build . --config Debug
```

You will find the resulting binary, and the DLLs it needs, in `build/bin/Debug/` (or `build/bin/Release`).

You can also build DIVA in the Visual Studio UI by running the first cmake command above and then opening the DIVA.sln solution generated in the build directory.

### Building with static libraries

By default DIVA dynamically links libdwarf, libelf and their dependent libraries. To statically link these add -DSTATIC_DWARF_LIBS=ON to your first cmake command.

## Running the tests

### Unit tests

In the same directory as the diva binary, you will also find a unittests binary which will run all the unit tests. They are built by cmake at the same time as Diva itself.

```
# Windows
build_path\bin\Debug\unittests.exe
build_path\bin\Release\unittests.exe

# Linux
build_path/bin/unittests
```

The unittests use the Google Test framework: https://github.com/google/googletest

### System tests

To run the system test suite, you will need Python 2.7:

```bash
cd DIVA/SystemTests
pip install -r requirements.txt
pytest --divadir=<path to diva bin dir>
```

The system tests use the pytest framework: https://docs.pytest.org

## Dependencies

DIVA uses libdwarf. Prebuilt libraries are included in the source for convenience, but they can be rebuilt via the CMake files in the root directory ExternalDependencies.

The same cmake commands are used as for DIVA itself. Afterwards, run the following command to copy the built libraries, binaries and headers to `build/Deploy`

```bash
cmake --build build --target DEPLOY
```

You can then copy these files over the existing ones in `DIVA/ExternalDependencies/DwarfDump`. Note that the DLLs, .sos and .libs need to be put into the correct platform, processor type and configuration sub folders. Eg. A 64 bit Linux debug .so file needs to go in `DIVA/ExternalDependencies/DwarfDump/Libraries/linux_x64_debug`.
