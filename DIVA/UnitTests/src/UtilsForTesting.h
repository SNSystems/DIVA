//===-- LibScopeView/UtilsForTesting.h ---------------------------- C++ -*-===//
///
/// Copyright (c) 2017 by Sony Interactive Entertainment Inc.
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declarations of some usefull testing functions.
///
//===----------------------------------------------------------------------===//

#include <string>

/// \brief Get the path to the test input directory.
const std::string &getTestInputDir();

/// \brief Get the path to a test input file.
std::string getTestInputFilePath(const std::string FileName);

/// \brief Get the path to the test output directory.
const std::string &getTestOutputDir();

/// \brief Get the path to a test output file.
std::string getTestOutputFilePath(const std::string FileName);

/// \brief Get all the text in a test output file.
std::string readTestOutputFile(const std::string FileName);

/// \brief Delete a test output file if it exists.
///
/// Note: This is not thread safe so tests should not share output files.
void clearTestOutputFile(const std::string FileName);
