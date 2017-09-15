//===-- StringPool.h --------------------------------------------*- C++ -*-===//
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
/// Definition of the StringPool class.
///
//===----------------------------------------------------------------------===//

#ifndef STRINGPOOL_H_
#define STRINGPOOL_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#define HASHTABLE_NUM_BUCKETS 8192

namespace LibScopeView {

class CmdOptions;

/// \brief This class implements a String Pool for deduplicating strings.
///
/// The deduplicated strings are stored in one contiguous bit of memory, and a
/// hash table is then used to index them.
class StringPool {
public:
  StringPool(StringPool const &) = delete;
  StringPool &operator=(StringPool const &) = delete;

  /// \brief Add strings to the String Pool.
  static size_t getStringIndex(const char *Str);
  static size_t getStringIndex(const std::string &Str);
  static const char *getStringValue(size_t Index);

  static void create();
  static void destroy(const CmdOptions &Options);

public:
  /// \brief Inserts a string in the pool, if required, and then returns an
  /// index to it.
  size_t getIndex(const char *Str);

  /// \brief Looks for a string inside the String Pool.
  ///
  /// If the string is present then its index is returned and the boolean flag
  /// is set to true, otherwise 0 is returned and the boolean flag is set to
  /// false.
  size_t lookup(const char *Str, bool &Found);

  /// \brief Returns the string stored for the given index.
  const char *getString(size_t Index);

  /// \brief Dump the whole String Pool contents.
  void dump(const char *Title = "String Pool Table:");

  /// \brief Dump statistics on String Pool contents.
  void info(const char *Title = "String Pool Info:");

protected:
  // Destructor can only be called by destroy().
  virtual ~StringPool();

private:
  // Create an empty String Pool.
  StringPool();

  // The hashing function for the String Pool.
  uint32_t strHash(const char *Str);

  // Looks for a specific string inside the String Pool.
  size_t lookup(const char *Str, size_t Bucket, bool &Found);

  // Creates a specific string in this String Pool and returns it's index.
  size_t insert(const char *Str, size_t Bucket);

  // Display information requested by the user.
  void printInfo(const CmdOptions &Options);

private:
  // All the strings in the pool, as null-terminated char sequences.
  std::vector<char> TheStrings;

  // The hash table indexing the strings in the pool.
  std::vector<size_t> HashTable[HASHTABLE_NUM_BUCKETS];

  // Track hits and misses.
  size_t Hits;
  size_t Misses;
};

} // namespace LibScopeView

#endif // STRINGPOOL_H_
