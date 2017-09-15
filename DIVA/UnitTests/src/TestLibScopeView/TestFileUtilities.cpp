//===-- UnitTests/TestLibScopeView/TestFileUtilities.cpp --------*- C++ -*-===//
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
/// Tests for the FileUtilities.
///
//===----------------------------------------------------------------------===//

#include "FileUtilities.h"
#include "UtilsForTesting.h"

#include "gtest/gtest.h"

using namespace LibScopeView;

// There are currently no unit tests for the following functions:
//   toNative
//   getAbsolutePath
//   getCurrentDir
//   makeDir
//   recursiveMakeDir
// This is because they use the Windows/Linux API and there is no good way to
// either mock that out, or to independantly verify the results.

namespace {

const std::string FileNotFoundError("Test file not found at ");

}

TEST(FileUtilities, unifyFilePath) {
  // Test already unified.
  std::string FailMessage("unifyFilePath changed an already unified path");
  EXPECT_EQ(unifyFilePath("foo"), "foo") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo/"), "foo/") << FailMessage;
  EXPECT_EQ(unifyFilePath("/foo"), "/foo") << FailMessage;
  EXPECT_EQ(unifyFilePath("c:/foo"), "c:/foo") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo/bar"), "foo/bar") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo/bar/"), "foo/bar/") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo/bar/this/is/a/longer/path"),
            "foo/bar/this/is/a/longer/path") << FailMessage;

  // Test backslashes.
  FailMessage = "unifyFilePath did not convert '\\' to '/' correctly";
  EXPECT_EQ(unifyFilePath("foo\\"), "foo/") << FailMessage;
  EXPECT_EQ(unifyFilePath("\\foo"), "/foo") << FailMessage;
  EXPECT_EQ(unifyFilePath("c:\\foo"), "c:/foo") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo\\bar"), "foo/bar") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo\\bar\\"), "foo/bar/") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo\\bar\\this\\is\\a\\longer\\path"),
            "foo/bar/this/is/a/longer/path") << FailMessage;

  // Test mixed.
  EXPECT_EQ(unifyFilePath("foo\\bar/quaz\\"), "foo/bar/quaz/") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo/bar\\quaz/"), "foo/bar/quaz/") << FailMessage;

  // Test repeated seperators.
  FailMessage = "unifyFilePath did not de-duplicate seperators correctly";
  EXPECT_EQ(unifyFilePath("foo//bar"), "foo/bar") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo///bar"), "foo/bar") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo/////bar"), "foo/bar") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo//bar//"), "foo/bar/") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo\\\\bar"), "foo/bar") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo\\\\bar\\\\\\"), "foo/bar/") << FailMessage;
  EXPECT_EQ(unifyFilePath("foo\\\\bar///quaz\\"), "foo/bar/quaz/")
      << FailMessage;

// Test path is set to all lower case on windows.
#if defined(_WIN32)
  std::string Expected("test/path/");
  FailMessage = "unifyFilePath did not convert to lower case on windows";
#else
  std::string Expected("Test/PATH/");
  FailMessage = "unifyFilePath did not preserve case on linux";
#endif
  EXPECT_EQ(unifyFilePath("Test\\PATH\\"), Expected) << FailMessage;

  // Test long file path prefix on windows.
#if defined(_WIN32)
  FailMessage = "unifyFilePath did not remove long file prefix on windows";
  EXPECT_EQ(unifyFilePath("\\\\?\\d:/foo"), "d:/foo") << FailMessage;
#endif
}

TEST(FileUtilities, flattenFilePath) {
  std::string FailMessage("flattenFilePath changed an already flat path");
  EXPECT_EQ(flattenFilePath("foo"), "foo") << FailMessage;
  EXPECT_EQ(flattenFilePath("foo_bar"), "foo_bar") << FailMessage;

  FailMessage = "flattenFilePath did not replace '.' '/' ':' with '_'";
  EXPECT_EQ(flattenFilePath("foo/bar"), "foo_bar") << FailMessage;
  EXPECT_EQ(flattenFilePath("foo/bar/"), "foo_bar_") << FailMessage;
  EXPECT_EQ(flattenFilePath("foo.txt"), "foo_txt") << FailMessage;
#ifdef _WIN32
  EXPECT_EQ(flattenFilePath("c:/foo"), "c__foo") << FailMessage;
  EXPECT_EQ(flattenFilePath("c:/foo/bar/test.c"), "c__foo_bar_test_c")
      << FailMessage;
#else
  EXPECT_EQ(flattenFilePath("c:/foo"), "c:_foo") << FailMessage;
  EXPECT_EQ(flattenFilePath("/foo/bar/test:test.c"), "_foo_bar_test:test_c")
      << FailMessage;
#endif
}

TEST(FileUtilities, getFileName) {
  EXPECT_EQ(getFileName("foo"), "foo");
  EXPECT_EQ(getFileName("foo.txt"), "foo.txt");
  EXPECT_EQ(getFileName("/foo/bar"), "bar");
  EXPECT_EQ(getFileName("/foo/bar.exe"), "bar.exe");
  EXPECT_EQ(getFileName("/foo/"), "");
  EXPECT_EQ(getFileName(""), "");
}

TEST(FileUtilities, getDirectoryName) {
  EXPECT_EQ(getDirectoryName("foo"), "");
  EXPECT_EQ(getDirectoryName("foo.txt"), "");
  EXPECT_EQ(getDirectoryName("/foo/bar"), "/foo");
  EXPECT_EQ(getDirectoryName("/foo/bar/file"), "/foo/bar");
  EXPECT_EQ(getDirectoryName("/foo/"), "/foo");
  EXPECT_EQ(getDirectoryName(""), "");
}

TEST(FileUtilities, FileDescriptor) {
#ifdef _WIN32
#define READ(FD, BUF, SIZE) _read(FD, BUF, SIZE)
#else
#define READ(FD, BUF, SIZE) read(FD, BUF, SIZE)
#endif

  const std::string FilePath = getTestInputFilePath("Test.txt");
  ASSERT_TRUE(doesFileExist(FilePath));

  FileDescriptor FD(FilePath);
  ASSERT_GE(*FD, 0);

  // File should contain 4 characters, try to read 5 to check.
  char Buf[5];
  auto BytesRead = READ(*FD, Buf, 5);
  EXPECT_EQ(BytesRead, 4);

  if (BytesRead == 4) {
    Buf[4] = '\0';
    EXPECT_STREQ(Buf, "Test");
  }

#undef READ
}

TEST(FileUtilities, fileDescriptorOpenFileThatDoesntExist) {
  const std::string FilePath = getTestInputFilePath("DoesntExist.bad");
  EXPECT_EXIT(
      { FileDescriptor FD(FilePath); }, testing::ExitedWithCode(1),
      ".*ERR_FILEIO_OPEN_FAILURE.*Unable to open file '.*DoesntExist.bad'.");
}

TEST(FileUtilities, doesFileExistFailsForFilesThatDontExist) {
  const std::string FileLocation = getTestInputFilePath("DoesntExist.elf");
  EXPECT_FALSE(doesFileExist(FileLocation));
}

TEST(FileUtilities, doesFileExistSucceedsForFileThatDoesExist) {
  const std::string FileLocation = getTestInputFilePath("3Bytes.o");
  EXPECT_TRUE(doesFileExist(FileLocation));
}

TEST(FileUtilities, isElfFailsOnLessThanFourBytes) {
  const std::string FileLocation = getTestInputFilePath("3Bytes.o");
  ASSERT_TRUE(doesFileExist(FileLocation))<< FileNotFoundError << FileLocation;
  EXPECT_FALSE(isFileFormatElf(FileLocation));
}

TEST(FileUtilities, isElfFailsOnNonElfs) {
  const std::string FileLocation = getTestInputFilePath("Test.txt");
  ASSERT_TRUE(doesFileExist(FileLocation))<< FileNotFoundError << FileLocation;
  EXPECT_FALSE(isFileFormatElf(FileLocation));
}

TEST(FileUtilities, isElfSucceedsOnElfs) {
  const std::string FileLocation = getTestInputFilePath("test.o");
  ASSERT_TRUE(doesFileExist(FileLocation))<< FileNotFoundError << FileLocation;
  EXPECT_TRUE(isFileFormatElf(FileLocation));
}

