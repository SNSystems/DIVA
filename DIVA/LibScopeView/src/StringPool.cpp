//===-- StringPool.cpp ------------------------------------------*- C++ -*-===//
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
/// Implementation of the StringPool class.
///
//===----------------------------------------------------------------------===//

#include "StringPool.h"

#include <assert.h>
#include <iomanip>
#include <math.h>
#include <string.h>

using namespace LibScopeView;

namespace {
// Current instance to handle the String Pool.
StringPool *GlobalStringPool = nullptr;
} // namespace

void StringPool::create() {
  // Do not create another pool if we have already one.
  if (GlobalStringPool) {
    return;
  }

  GlobalStringPool = new StringPool();
  assert(GlobalStringPool);

  // Hash the empty string immediately so it gets to index 0.
  GlobalStringPool->getIndex("");
}

void StringPool::destroy() {
  if (GlobalStringPool) {
    delete GlobalStringPool;
    GlobalStringPool = nullptr;
  }
}

void StringPool::dumpPool(std::ostream &Out) {
  if (GlobalStringPool)
    GlobalStringPool->dump(Out);
}

void StringPool::poolInfo(std::ostream &Out) {
  if (GlobalStringPool)
    GlobalStringPool->info(Out);
}

size_t StringPool::getStringIndex(const char *Str) {
  assert(GlobalStringPool);
  return GlobalStringPool->getIndex(Str);
}

size_t StringPool::getStringIndex(const std::string &Str) {
  return getStringIndex(Str.c_str());
}

const char *StringPool::getStringValue(size_t Index) {
  assert(GlobalStringPool);
  return GlobalStringPool->getString(Index);
}

StringPool::StringPool() {
  Hits = 0;
  Misses = 0;

  TheStrings.push_back('\0');
}

StringPool::~StringPool() {}

size_t StringPool::getIndex(const char *Str) {
  // Index to string.
  size_t Index = 0;
  // The NULL string is equivalent to the empty string.
  if ((Str != nullptr) && (Str[0] != '\0')) {

    // Any other string is hashed and looked up.
    uint32_t Hash = strHash(Str);
    size_t bucket = Hash % HASHTABLE_NUM_BUCKETS;

    // Lookup.
    bool Found = false;
    Index = lookup(Str, bucket, Found);

    if (!Found) {
      // Create the string in the table.
      Index = insert(Str, bucket);
    }
  }

  return Index;
}

size_t StringPool::lookup(const char *Str, bool &Found) {
  // Any other string is hashed and looked up.
  uint32_t Hash = strHash(Str);
  size_t Bucket = Hash % HASHTABLE_NUM_BUCKETS;

  // Lookup.
  return lookup(Str, Bucket, Found);
}

size_t StringPool::lookup(const char *Str, size_t Bucket, bool &Found) {
  // Index to string.
  for (auto Index : HashTable[Bucket]) {
    const char *stored_str = &(TheStrings[Index]);
    if (strcmp(Str, stored_str) == 0) {
      Hits++;
      Found = true;
      return Index;
    }
  }

  Found = false;
  return 0;
}

size_t StringPool::insert(const char *Str, size_t Bucket) {
  size_t Index = TheStrings.size();
  while (*Str) {
    TheStrings.push_back(*Str++);
  }
  TheStrings.push_back('\0');

  HashTable[Bucket].push_back(Index);
  Misses++;

  return Index;
}

const char *StringPool::getString(size_t Index) {
  if (Index > TheStrings.size()) {
    throw std::logic_error("Invalid string index in String Pool.\n");
  }
  return &(TheStrings[Index]);
}

void StringPool::info(std::ostream &Out, const char *Title) {
  size_t MinEntries = static_cast<size_t>(-1);
  size_t MaxEntries = 0;
  double Average = (static_cast<double>(Misses) /
                    static_cast<double>(HASHTABLE_NUM_BUCKETS));
  double SumSquareErr = 0;
  for (size_t i = 0; i < HASHTABLE_NUM_BUCKETS; ++i) {
    if (MinEntries > HashTable[i].size())
      MinEntries = HashTable[i].size();
    if (MaxEntries < HashTable[i].size())
      MaxEntries = HashTable[i].size();
    SumSquareErr += ((static_cast<double>(HashTable[i].size()) - Average) *
                     (static_cast<double>(HashTable[i].size()) - Average));
  }
  double Variance = SumSquareErr / static_cast<double>(HASHTABLE_NUM_BUCKETS);

  Out << "\n" << Title << "\n";
  Out << "Number of buckets:           " << HASHTABLE_NUM_BUCKETS << "\n";
  Out << "Pool misses (total strings): " << Misses << "\n";
  Out << "Pool hits:                   " << Hits << "\n";
  Out << "Pool efficiency:             "
      << (static_cast<double>(Hits) / static_cast<double>(Misses)) << "\n";
  Out << "Min entries per bucket:      " << MinEntries << "\n";
  Out << "Max entries per bucket:      " << MaxEntries << "\n";
  Out << "Average entries per bucket:  " << Average << "\n";
  Out << "Standard deviation:          " << sqrt(Variance) << "\n";
  Out << "Size of string table:        " << TheStrings.size() << "\n";
}

uint32_t StringPool::strHash(const char *Str) {
  uint32_t Hash = 0;
  if (Str) {
    while (*Str != 0x00) {
      char Ch = *Str++;

      Hash += static_cast<uint32_t>(Ch);
      Hash += (Hash << 10);
      Hash ^= (Hash >> 6);
    }
    Hash += (Hash << 3);
    Hash ^= (Hash >> 11);
    Hash += (Hash << 15);
  }
  return Hash;
}

void StringPool::dump(std::ostream &Out, const char *Title) {
  Out << "\n" << Title << "\n";
  for (size_t Bucket = 0; Bucket < HASHTABLE_NUM_BUCKETS; ++Bucket) {
    for (auto Index : HashTable[Bucket]) {
      const char *Str = &(TheStrings[Index]);
      Out << "Bucket=0x" << std::setfill('0') << std::setw(8) << std::hex
          << Bucket;
      Out << ", Index=0x" << std::setfill('0') << std::setw(8) << std::hex
          << Index;
      Out << ", Str='" << Str << "'\n";
    }
  }
}
