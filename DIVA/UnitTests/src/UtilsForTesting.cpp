//===-- LibScopeView/UtilsForTesting.cpp -------------------------- C++ -*-===//
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
/// This file contains the definitions of some usefull testing functions.
///
//===----------------------------------------------------------------------===//

#include "UtilsForTesting.h"
#include "FileUtilities.h"

#include <fstream>
#include <exception>

namespace {

// UNIT_TEST_DIR comes from a definition in the UnitTest cmake file and is the
// path to the DIVA/UnitTests directory.
const std::string UnitTestDir(UNIT_TEST_DIR);
const std::string TestInputDir(UnitTestDir + "/TestInputs");
const std::string TestOutputDir(UnitTestDir + "/TestOutputs");

class UtilsForTestingException : public std::exception {
public:
  UtilsForTestingException(const std::string &Msg) : Msg(Msg) {}

private:
  const char *what() const throw() override { return Msg.c_str(); }
  std::string Msg;
};

}

const std::string &getTestInputDir() {
  return TestInputDir;
}

std::string getTestInputFilePath(const std::string FileName) {
  return getTestInputDir() + '/' + FileName;
}

const std::string &getTestOutputDir() {
  return TestOutputDir;
}

std::string getTestOutputFilePath(const std::string FileName) {
  return getTestOutputDir() + '/' + FileName;
}

std::string readTestOutputFile(const std::string FileName) {
  std::string Path(getTestOutputFilePath(FileName));
  std::ifstream InStream(Path);
  if (InStream.fail()) {
    std::string Err("Failed to open test output file ");
    Err += Path;
    throw UtilsForTestingException(Err);
  }

  std::string Result;
  std::getline(InStream, Result, '\0');
  return Result;
}

void clearTestOutputFile(const std::string FileName) {
  std::string Path(getTestOutputFilePath(FileName));
  if (LibScopeView::doesFileExist(Path)) {
    if (std::remove(Path.c_str()) != 0) {
      std::string Err("Failed to delete existing test output file ");
      Err += Path;
      throw UtilsForTestingException(Err);
    }
  }
}
