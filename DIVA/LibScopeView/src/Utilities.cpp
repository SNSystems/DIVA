//===-- LibScopeView/Utilities.cpp ------------------------------*- C++ -*-===//
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
/// Definitions of utility functions.
///
//===----------------------------------------------------------------------===//

#include "Utilities.h"
#include "Platform.h"
#include "StringPool.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef PLATFORM_WIN
#include <Windows.h>
#include <psapi.h>
#else
#include <fstream>
#include <sstream>
#endif // PLATFORM_WIN

using namespace LibScopeView;

void LibScopeView::initialize() {
#if defined(PLATFORM_WIN) && defined(NDEBUG)
  // Disable annoying windows failure pop up window.
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif

  // Create the String Pool, to be used for all readers.
  StringPool::create();
}

void LibScopeView::terminate() {
  // Delete the String Pool.
  StringPool::destroy();
}

size_t  LibScopeView::getPeakMemoryUsage() {
  size_t Size = 0;

#ifdef PLATFORM_WIN
  PROCESS_MEMORY_COUNTERS PMC;
  if (GetProcessMemoryInfo(GetCurrentProcess(), &PMC, sizeof(PMC))) {
    Size = PMC.PeakWorkingSetSize;
  }
#else
  std::ifstream StatusStream("/proc/self/status", std::ios_base::in);
  std::string Ln;
  // Look through /proc/self/status for the line starting with "VmHWM:"
  // VmHWM is the peak physical memory used by the process.
  std::string MemPrefix("VmHWM:");
  while (std::getline(StatusStream, Ln)) {
    if (Ln.compare(0, MemPrefix.size(), MemPrefix) == 0) {
      // Read in the number after "VmHWM:" which is the usage in kB.
      std::stringstream LineStream(Ln.substr(MemPrefix.size()));
      LineStream >> Size;
      Size = LineStream.fail() ? 0 : Size * 1024;
      break;
    }
  }
#endif // PLATFORM_WIN

  return Size;
}

void LibScopeView::printMemoryUsage(size_t MemoryUsage) {
  // Breakup memory usage into different chunks (XXX,XXX,XXX kb).
  auto MemoryUsedkB = (MemoryUsage / 1000) % 1000;
  auto MemoryUsedmB = (MemoryUsage / 1000000) % 1000;
  auto MemoryUsedgB = (MemoryUsage / 1000000000);

  // Avoid using manipulators on std::cout directly.
  std::stringstream Result;
  Result << "\nPeak Memory Usage: ";
  if (MemoryUsedgB > 0) {
    Result << MemoryUsedgB << ',' << std::setw(3) << std::setfill('0')
           << MemoryUsedmB << ',' << std::setw(3) << std::setfill('0')
           << MemoryUsedkB;
  } else if (MemoryUsedmB > 0) {
    Result << MemoryUsedmB << ',' << std::setw(3) << std::setfill('0')
           << MemoryUsedkB;
  } else {
    Result << MemoryUsedkB;
  }
  Result << " KB\n";

  std::cout << Result.str();
}

std::chrono::steady_clock::time_point LibScopeView::getCurrentTime() {
  return std::chrono::steady_clock::now();
}

void LibScopeView::printTimeTaken(
    const std::chrono::steady_clock::time_point &StartTime,
    const std::chrono::steady_clock::time_point &EndTime) {
  typedef std::chrono::duration<double, std::ratio<1>> Seconds;
  double TimeTaken =
      std::chrono::duration_cast<Seconds>(EndTime - StartTime).count();
  std::cout << "\nTime taken: " << std::setprecision(2) << TimeTaken
            << " seconds\n";
}

std::string LibScopeView::trim(const std::string &text) {
  size_t first = text.find_first_not_of(' ');
  if (first == std::string::npos) {
    first = 0;
  }
  size_t last = text.find_last_not_of(' ');
  if (last == std::string::npos) {
    last = text.length() - 1;
  }
  return text.substr(first, (last - first + 1));
}
