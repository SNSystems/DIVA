//===-- LibScopeView/FileUtilities.cpp --------------------------*- C++ -*-===//
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
/// This file contains the definitions of the file system utility functions
/// declared in LibScopeView/FileUtilities.h.
///
//===----------------------------------------------------------------------===//

#include "FileUtilities.h"
#include "Error.h"
#include "Platform.h"

#include <algorithm>
#include <array>
#include <assert.h>
#include <cctype>
#include <fcntl.h>
#include <fstream>
#include <iterator>
#include <vector>

#ifdef PLATFORM_WIN
// Stop Windows.h breaking std::min by defining a min macro
#define NOMINMAX
#include <Windows.h>
#include <io.h>
#elif defined(PLATFORM_LINUX)
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#error OS not supported
#endif // PLATFORM_WIN

using namespace LibScopeView;

namespace {

#ifdef PLATFORM_WIN
const std::string WinLongPrefix("\\\\?\\");
#endif

bool getBytesFromFile(std::vector<char> &Bytes, const std::string FileLocation,
                      const size_t SizeRequest) {
  // std::ios:ate sets the initial position to the end of the file
  std::ifstream FIn(nativeFilePath(FileLocation),
                    std::ios::binary | std::ios::ate);
  std::streampos ActualSize = FIn.tellg();
  if (ActualSize < 0)
    return false;

  // SizeToRead is the minimum of the file size, the number of bytes requested,
  // and the maximum number that can be passed to istream::read.
  const size_t SizeToRead = std::min(
      {static_cast<size_t>(ActualSize), SizeRequest,
       static_cast<size_t>(std::numeric_limits<std::streamsize>::max())});

  Bytes.resize(SizeToRead);
  FIn.seekg(0, std::ios_base::beg);
  FIn.read(&Bytes[0], static_cast<std::streamsize>(SizeToRead));
  return true;
}

} // End anonymous namespace.

std::string LibScopeView::unifyFilePath(const std::string &Path) {
  // Convert separators to '/' and remove duplicates.
  // Note: This needs to be fast as this function seems to be called a lot.
  std::string Result;
  Result.reserve(Path.size());
  bool LastWasSeperator = false;

  size_t StrPos = 0;
#ifdef PLATFORM_WIN
  // Remove Windows long path prefix (\\?\).
  StrPos = (Path.compare(0, WinLongPrefix.size(), WinLongPrefix) == 0) ? 4 : 0;
#endif

  for (; StrPos < Path.size(); ++StrPos) {
    char C = Path[StrPos];
    if (C != '/' && C != '\\') {
#ifdef PLATFORM_WIN
      Result += std::tolower(C);
#else
      Result += C;
#endif
      LastWasSeperator = false;
    } else if (!LastWasSeperator) {
      Result += '/';
      LastWasSeperator = true;
    }
  }

  return Result;
}

std::string LibScopeView::nativeFilePath(const std::string &UnifiedPath) {
#ifdef PLATFORM_WIN
  std::string Result;
  // If relative or long path then convert to long absolute.
  if (UnifiedPath.size() < 2 || UnifiedPath[1] != ':' ||
      UnifiedPath.size() > MAX_PATH) {
    Result += (UnifiedPath.size() < MAX_PATH ? "" : WinLongPrefix);
    if (UnifiedPath.size() < 2 || UnifiedPath[1] != ':') {
      Result += getCurrentDir(/*Unify*/ false);
      Result.push_back('\\');
    }
  }
  // Convert '/' to '\\'
  for (char C : UnifiedPath)
    Result.push_back(C == '/' ? '\\' : C);
  return Result;
#else
  return UnifiedPath;
#endif
}

std::string LibScopeView::flattenFilePath(const std::string &UnifiedPath) {
  std::string Result;
  auto flattenChar = [](char C) -> char {
    if (C == '.' || C == '/' || (C == ':' && platformIsWindows()))
      return '_';
    return C;
  };
  std::transform(UnifiedPath.cbegin(), UnifiedPath.cend(),
                 std::back_inserter(Result), flattenChar);
  return Result;
}

std::string LibScopeView::getCurrentDir(bool Unify) {
  bool Failed = false;
  std::string Result;

#ifdef _WIN32
  std::vector<char> Buff(MAX_PATH);
  auto Ret = GetCurrentDirectoryA(static_cast<DWORD>(Buff.size()), Buff.data());
  if (Ret > Buff.size()) {
    Buff.resize(Ret);
    Ret = GetCurrentDirectoryA(static_cast<DWORD>(Buff.size()), Buff.data());
  }
  Failed = (Ret == 0 || Ret > Buff.size());
  if (!Failed)
    Result = Buff.data();
#else
  char Buff[PATH_MAX];
  Failed = (getcwd(Buff, PATH_MAX) == nullptr);
  if (!Failed)
    Result = Buff;
#endif

  if (Failed)
    fatalError(LibScopeError::ErrorCode::ERR_FILEIO_GET_CWD);
  return Unify ? unifyFilePath(Result) : Result;
}

std::string LibScopeView::getAbsolutePath(const std::string &UnifiedPath,
                                          bool Unify) {
  assert(!UnifiedPath.empty());
  bool Failed = false;
  std::string Result;

#ifdef PLATFORM_WIN
  std::vector<char> Buff(MAX_PATH);
  auto Ret =
      GetFullPathNameA(UnifiedPath.c_str(), static_cast<DWORD>(Buff.size()),
                       Buff.data(), nullptr);
  if (Ret > Buff.size()) {
    Buff.resize(Ret);
    Ret = GetFullPathNameA(UnifiedPath.c_str(), static_cast<DWORD>(Buff.size()),
                           Buff.data(), nullptr);
  }
  Failed = (Ret == 0 || Ret > Buff.size());
  if (!Failed)
    Result = Buff.data();
#else
  char Buff[PATH_MAX];
  Failed = (realpath(UnifiedPath.c_str(), Buff) == nullptr);
  if (!Failed)
    Result = Buff;
#endif

  if (Failed)
    fatalError(LibScopeError::ErrorCode::ERR_FILEIO_ABS_PATH, UnifiedPath);
  return Unify ? unifyFilePath(Result) : Result;
}

std::string LibScopeView::getFileName(const std::string &UnifiedPath) {
  size_t SplitPos = UnifiedPath.rfind('/');
  if (SplitPos == std::string::npos)
    return UnifiedPath;
  else
    return UnifiedPath.substr(SplitPos + 1);
}

std::string LibScopeView::getDirectoryName(const std::string &UnifiedPath) {
  size_t SplitPos = UnifiedPath.rfind('/');
  if (SplitPos == std::string::npos)
    return "";
  return UnifiedPath.substr(0, SplitPos);
}

bool LibScopeView::makeDir(const std::string &UnifiedPath) {
#ifdef PLATFORM_WIN
  // CreateDirectory path size limit = 248.
  // Apparently this is (MAX_PATH minus enough room for a 8.3 filename).
  if (UnifiedPath.size() < 248) {
    // Relies on short-circuiting, getLastError is not called if CreateDirectory
    // returns true.
    return (CreateDirectory(UnifiedPath.c_str(), nullptr) ||
            GetLastError() == ERROR_ALREADY_EXISTS);
  }
  std::wstring LongPath(WinLongPrefix.begin(), WinLongPrefix.end());
  if (UnifiedPath.size() < 2 || UnifiedPath[1] != ':') {
    for (char C : getCurrentDir(/*Unify*/ false))
      LongPath.push_back(C);
    LongPath.push_back('\\');
  }
  for (char C : UnifiedPath)
    LongPath.push_back((C == '/') ? '\\' : C);

  return (CreateDirectoryW(LongPath.c_str(), nullptr) ||
          GetLastError() == ERROR_ALREADY_EXISTS);
#else
  struct stat SB;
  if (stat(UnifiedPath.c_str(), &SB) == 0 && S_ISDIR(SB.st_mode))
    return true;
  return (mkdir(UnifiedPath.c_str(), S_IRWXU) == 0 || errno == EEXIST);
#endif
}

bool LibScopeView::recursiveMakeDir(const std::string &UnifiedPath) {
  std::string Path(UnifiedPath);
  if (Path.empty() || Path.back() != '/')
    Path.append("/");
  // For the position of each '/' in the path.
  for (size_t Pos = Path.find('/'); Pos != std::string::npos;
       Pos = Path.find('/', Pos + 1)) {
    // Create the directory at the current path position.
    std::string PathSection(Path.substr(0, Pos));
    // PathSection can be empty e.g. /foo.
    if (PathSection.empty())
      continue;
    if (!makeDir(PathSection))
      return false;
  }
  return true;
}

bool LibScopeView::doesFileExist(const std::string &FileLocation) {
  std::ifstream IFS(nativeFilePath(FileLocation));
  return IFS.good();
}

bool LibScopeView::isFileFormatElf(const std::string &FileLocation) {
  static const std::array<char, 4> ElfMagic = {{0x7f, 0x45, 0x4c, 0x46}};

  std::vector<char> Bytes;
  if (!getBytesFromFile(Bytes, FileLocation, ElfMagic.size()))
    return false;

  return std::equal(Bytes.begin(), Bytes.end(), ElfMagic.begin());
}

FileDescriptor::FileDescriptor(const std::string &UnifiedPath) {
#ifdef PLATFORM_WIN
  _sopen_s(&FD, nativeFilePath(UnifiedPath).c_str(), _O_BINARY | _O_RDONLY,
           _SH_DENYWR, 0);
#else
  FD = open(UnifiedPath.c_str(), O_RDONLY);
#endif
  if (FD < 0)
    fatalError(LibScopeError::ErrorCode::ERR_FILEIO_OPEN_FAILURE, UnifiedPath);
}

FileDescriptor::~FileDescriptor() {
  if (FD >= 0) {
#ifdef PLATFORM_WIN
    _close(FD);
#else
    close(FD);
#endif
  }
}

FileDescriptor &FileDescriptor::operator=(FileDescriptor &&Other) {
  if (this == &Other)
    return *this;
  // Ensure the current file is closed now rather than when Other is destroyed.
  // i.e. Tmp gets 'this' and gets closed as we return, 'this' gets Other
  FileDescriptor Tmp;
  swap(Tmp, Other);
  swap(*this, Tmp);
  return *this;
}
