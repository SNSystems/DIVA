//===-- UnitTests/TestLibScopeView/TestStringPool.cpp -----------*- C++ -*-===//
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
/// Tests for LibScopeView::StringPool.
///
//===----------------------------------------------------------------------===//

#include "StringPool.h"

#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(StringPool, DeduplicateStrings) {
  StringPool Pool;
  std::string Foo("foo");
  std::string Bar("bar");
  std::string Baz("baz");

  StringPoolRef FooRef = Pool.get(Foo);
  StringPoolRef BarRef = Pool.get(Bar);
  StringPoolRef BazRef = Pool.get(Baz);

  EXPECT_EQ(*FooRef, Foo);
  EXPECT_EQ(*BarRef, Bar);
  EXPECT_EQ(*BazRef, Baz);

  EXPECT_EQ(FooRef, Pool.get(Foo));
  EXPECT_EQ(BarRef, Pool.get(Bar));
  EXPECT_EQ(BazRef, Pool.get(Baz));
}
