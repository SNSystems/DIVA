//===-- LibScopeView/Utilities.h --------------------------------*- C++ -*-===//
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
/// Declarations of utility functions.
///
//===----------------------------------------------------------------------===//

#ifndef UTILITIES_H
#define UTILITIES_H

#include <chrono>
#include <string>

namespace LibScopeView {

class CmdOptions;

/// \brief Initialization for LibScopeView.
void initialize();

/// \brief Termination for LibScopeView.
void terminate(const CmdOptions &Options);

/// \brief Get the peak memory usage of the current executable.
size_t getPeakMemoryUsage();

/// \brief Print program memory usage to std::cout.
void printMemoryUsage(size_t MemoryUsage);

/// \brief Get the current time.
std::chrono::steady_clock::time_point getCurrentTime();

/// \brief Print the program run time to std::cout.
void printTimeTaken(const std::chrono::steady_clock::time_point &StartTime,
                    const std::chrono::steady_clock::time_point &EndTime);

/// \brief Remove leading and trailing spaces.
std::string trim(const std::string &Text);

} // namespace LibScopeView

#endif // UTILITIES_H
