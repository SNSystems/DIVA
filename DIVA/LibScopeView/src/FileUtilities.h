//===-- LibScopeView/FileUtilities.h ----------------------------*- C++ -*-===//
///
/// Copyright (c) Sony Interactive Entertainment Inc.
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
/// This file contains the declaration of several utility functions for working
/// with paths and file systems in a platform independent way.
///
//===----------------------------------------------------------------------===//

#ifndef FILE_UTILITIES_H
#define FILE_UTILITIES_H

#include <string>

namespace LibScopeView {

/// \brief Convert a file path to forward slash separators and then lower case
/// on Windows.
///
/// All the other functions in this header expect unified_paths.
std::string unifyFilePath(const std::string &Path);

/// \brief Transform the file path back from a unique file name.
std::string nativeFilePath(const std::string &UnifiedPath);

/// \brief Transform the file path into a unique file name.
std::string flattenFilePath(const std::string &UnifiedPath);

/// \brief get the current working directory.
std::string getCurrentDir(bool Unify = true);

/// \brief Convert a relative path to an absolute path.
std::string getAbsolutePath(const std::string &UnifiedPath, bool Unify = true);

/// \brief Return the filename from a path.
std::string getFileName(const std::string &UnifiedPath);

/// \brief Return the directory name from a path.
std::string getDirectoryName(const std::string &UnifiedPath);

/// \brief Create a directory if one does not exist.
///
/// Returns true if the directory was created or already existed.
bool makeDir(const std::string &UnifiedPath);

/// \brief Create all directories in a path if they do not exist.
///
/// Returns true if all the directories were created or already existed.
bool recursiveMakeDir(const std::string &UnifiedPath);

/// \brief Return true if the file exists.
bool doesFileExist(const std::string &FileLocation);

/// \brief Return true if the file is an elf.
bool isFileFormatElf(const std::string &FileLocation);

/// \brief RAII warpper around an int file descriptor.
class FileDescriptor {
public:
  FileDescriptor() : FD(-1) {}
  FileDescriptor(const std::string &UnifiedPath);
  ~FileDescriptor();

  const int &get() const { return FD; }
  const int &operator*() const { return get(); }

  friend void swap(FileDescriptor &A, FileDescriptor &B) {
    std::swap(A.FD, B.FD);
  }

  FileDescriptor(const FileDescriptor &Other) = delete;
  FileDescriptor &operator=(const FileDescriptor &Other) = delete;

  FileDescriptor(FileDescriptor &&Other) : FD(-1) { swap(*this, Other); }
  FileDescriptor &operator=(FileDescriptor &&Other);

private:
  int FD;
};

} // namespace LibScopeView

#endif // FILE_UTILITIES_H
