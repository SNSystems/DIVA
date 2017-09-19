//===-- LibScopeView/Sort.cpp -----------------------------------*- C++ -*-===//
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
/// Implementations for the sorting of DIVA's internal representation.
///
//===----------------------------------------------------------------------===//

#include "Reader.h"

#include "Sort.h"

using namespace LibScopeView;

namespace {
template <typename T> int compare(const T &LHS, const T &RHS) {
  return (LHS < RHS) ? -1 : ((LHS > RHS) ? 1 : 0);
}
} // namespace

int LibScopeView::compareKind(const Object *LHS, const Object *RHS) {
  return compare(std::string(LHS->getKindAsString()),
                 std::string(RHS->getKindAsString()));
}

int LibScopeView::compareLine(const Object *LHS, const Object *RHS) {
  return compare(LHS->getLineNumber(), RHS->getLineNumber());
}

int LibScopeView::compareName(const Object *LHS, const Object *RHS) {
  return compare(std::string(LHS->getName()), std::string(RHS->getName()));
}

int LibScopeView::compareOffset(const Object *LHS, const Object *RHS) {
  return compare(LHS->getDieOffset(), RHS->getDieOffset());
}

bool LibScopeView::sortByLine(const Object *LHS, const Object *RHS) {
  // Use the line number as the first key.
  int Comp = compareLine(LHS, RHS);
  if (Comp != 0)
    return Comp < 0;
  // Lines are the same; use the names.
  Comp = compareName(LHS, RHS);
  if (Comp != 0)
    return Comp < 0;
  // Names are the same; use the kinds.
  Comp = compareKind(LHS, RHS);
  if (Comp != 0)
    return Comp < 0;
  // Kinds are the same; use the offset.
  return compareOffset(LHS, RHS) < 0;
}

bool LibScopeView::sortByName(const Object *LHS, const Object *RHS) {
  // Use the name as the first key.
  int Comp = compareName(LHS, RHS);
  if (Comp != 0)
    return Comp < 0;
  // Names are the same; use the lines.
  Comp = compareLine(LHS, RHS);
  if (Comp != 0)
    return Comp < 0;
  // Lines are the same; use the kinds.
  Comp = compareKind(LHS, RHS);
  if (Comp != 0)
    return Comp < 0;
  // Kinds are the same; use the offset.
  return compareOffset(LHS, RHS) < 0;
}

bool LibScopeView::sortByOffset(const Object *LHS, const Object *RHS) {
  return compareOffset(LHS, RHS) < 0;
}

SortFunction LibScopeView::getSortFunction() {
  switch (getReader()->getPrintSettings().SortKey) {
  case SortingKey::LINE:
    return sortByLine;
  case SortingKey::OFFSET:
    return sortByOffset;
  case SortingKey::NAME:
    return sortByName;
  }
  return nullptr;
}
