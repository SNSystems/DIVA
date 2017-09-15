//===-- LibScopeView/Platform.h ---------------------------------*- C++ -*-===//
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
/// Preprocessor definitions for differentiating between platforms.
///
//===----------------------------------------------------------------------===//

#ifndef PLATFORM_H_
#define PLATFORM_H_

// Set one of the following definitions (or none if unknown):
// PLATFORM_WIN
// PLATFORM_LINUX
// Set one of the following definitions (or none if unknown):
// PLATFORM_32BIT
// PLATFORM_64BIT

#if defined(_WIN32)
#define PLATFORM_WIN 1
#if defined(_WIN64)
#define PLATFORM_64BIT 1
#else
#define PLATFORM_32BIT 1
#endif // _WIN64
#elif defined(__linux__)
#define PLATFORM_LINUX 1
#if defined(__x86_64__)
#define PLATFORM_64BIT 1
#elif defined(__i386__)
#define PLATFORM_32BIT 1
#endif // __x86_64__
#endif // _WIN32

namespace LibScopeView {

inline static constexpr bool platformIsWindows() {
#if defined(PLATFORM_WIN)
    return true;
#else
    return false;
#endif
}

}

#endif // PLATFORM_H_
