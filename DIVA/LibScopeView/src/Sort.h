//===-- LibScopeView/Sort.h -------------------------------------*- C++ -*-===//
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
/// Declarations relating to the sorting of DIVA's internal representation.
///
//===----------------------------------------------------------------------===//

#ifndef SORT_H
#define SORT_H

namespace LibScopeView {

/// \brief Object Sorting Mode.
enum SortMode {
  sr_none = 0, // No given sort.
  sr_kind,     // Sort by kind.
  sr_line,     // Sort by line.
  sr_name,     // Sort by name.
  sr_offset    // Sort by offset.
};

class Object;

typedef bool (*SortFunction)(const Object *LHS, const Object *RHS);

// Callback functions to sort objects.
SortFunction getSortFunction();
int compareKind(const Object *LHS, const Object *RHS);
int compareLine(const Object *LHS, const Object *RHS);
int compareName(const Object *LHS, const Object *RHS);
int compareOffset(const Object *LHS, const Object *RHS);
bool sortByKind(const Object *LHS, const Object *RHS);
bool sortByLine(const Object *LHS, const Object *RHS);
bool sortByName(const Object *LHS, const Object *RHS);
bool sortByOffset(const Object *LHS, const Object *RHS);

} // namespace LibScopeView

#endif // SORT_H
