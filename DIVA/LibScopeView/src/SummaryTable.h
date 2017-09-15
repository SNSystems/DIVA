//===-- LibScopeView/SummaryTable.h -----------------------------*- C++ -*-===//
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
/// Interface for the SummaryTable class.
///
//===----------------------------------------------------------------------===//

#ifndef SUMMARY_TABLE_H
#define SUMMARY_TABLE_H

#include <map>
#include <string>

namespace LibScopeView {

class Object;

class SummaryTable {
public:
  SummaryTable();

  /// \brief Outut the standard summary table for a single file.
  void getPrintedSummaryTable(std::ostream &out);

  /// \brief Increment a specific column in Obj's row.
  void incrementFound(const Object *obj);
  void incrementPrinted(const Object *obj);
  void incrementMissing(const Object *obj);
  void incrementAdded(const Object *obj);

private:
  struct SummaryTableRow {
    SummaryTableRow()
        : ObjectsFound(0), ObjectsPrinted(0), ObjectsMissing(0),
          ObjectsAdded(0) {}
    // Each row has numerous fields that can be incremented as needed.
    uint32_t ObjectsFound;
    uint32_t ObjectsPrinted;
    uint32_t ObjectsMissing;
    uint32_t ObjectsAdded;
  };

  // Map of the rows, indexed via the ObjectsClassID.
  std::map<std::string, SummaryTableRow> Rows;

  // Totals for all four columns of the summary table.
  unsigned int TotalFound;
  unsigned int TotalPrinted;
  unsigned int TotalMissing;
  unsigned int TotalAdded;

  // Column width values.
  const static uint32_t LabelWidth = 19;
  const static uint32_t ColumnWidth = 9;
  const static uint32_t IndentWidth = 5;
  const static uint32_t ColumnBuffer = 5;
};

} // namespace LibScopeView

#endif // SUMMARY_TABLE_H
