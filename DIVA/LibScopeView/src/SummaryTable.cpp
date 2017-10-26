//===-- LibScopeView/SummaryTable.cpp ---------------------------*- C++ -*-===//
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
/// Implementation for the SummaryTable class.
///
//===----------------------------------------------------------------------===//

#include "SummaryTable.h"
#include "Object.h"
#include "PrintSettings.h"
#include "ScopeVisitor.h"

#include <assert.h>
#include <iomanip>
#include <ostream>
#include <vector>

using namespace LibScopeView;

class SummaryTable::SummaryTableCounter : public ConstScopeVisitor {
public:
  SummaryTableCounter(SummaryTable &SumTable, const PrintSettings *PSettings)
      : Table(SumTable), Settings(PSettings) {}

private:
  SummaryTable &Table;
  const PrintSettings *Settings;

  void visitImpl(const Object *Obj) override {
    Table.incrementFound(Obj);
    if (!Settings || Settings->printObject(*Obj))
      Table.incrementPrinted(Obj);
    visitChildren(Obj);
  }
};

SummaryTable::SummaryTable(const Object &Root, const PrintSettings *Settings)
    : TotalFound(0), TotalPrinted(0) {
  // Create a list of row labels for each DIVA Object.
  static const std::vector<std::string> RowLabels = {"Alias",
                                                     "Block",
                                                     "Class",
                                                     "CodeLine",
                                                     "CompileUnit",
                                                     "Enum",
                                                     "Function",
                                                     "Member",
                                                     "Namespace",
                                                     "Parameter",
                                                     "PrimitiveType",
                                                     "Struct",
                                                     "TemplateParameter",
                                                     "Union",
                                                     "Using",
                                                     "Variable"};

  // Add a row to the table for each DIVA object.
  for (auto Label : RowLabels) {
    Rows.emplace(Label, SummaryTableRow());
  }

  // Gather the stats.
  SummaryTableCounter(*this, Settings).visit(&Root);
}

void SummaryTable::printSummaryTable(std::ostream &Out) const {
  // Calculate and create indent and divider strings.
  const uint32_t NumberOfColumns = 2;
  const uint32_t DividerLength = (LabelWidth + (ColumnWidth * NumberOfColumns));

  const std::string Indent(IndentWidth, ' ');
  const std::string Divider(DividerLength, '-');

  // Column headers.
  const std::string ObjectLabel("Object");
  const std::string TotalLabel("Total");
  const std::string PrintedLabel("Printed");
  const std::string TotalsLabel("Totals");

  // Output the header.
  Out << Indent << Divider << std::endl
      << std::left << Indent << std::setw(LabelWidth) << ObjectLabel
      << std::right << std::setw(ColumnWidth) << TotalLabel
      << std::setw(ColumnWidth) << PrintedLabel << std::endl
      << Indent << Divider << "\n";

  // Output each row.
  for (auto Row : Rows) {
    auto RowLabel = Row.first;
    auto RowData = Row.second;
    Out << Indent << std::left << std::setw(LabelWidth) << RowLabel
        << std::right << std::setw(ColumnWidth) << RowData.ObjectsFound
        << std::setw(ColumnWidth) << RowData.ObjectsPrinted << "\n";
  }

  // Output the footer.
  Out << Indent << Divider << std::endl
      << std::left << Indent << std::setw(LabelWidth) << TotalsLabel
      << std::right << std::setw(ColumnWidth) << TotalFound
      << std::setw(ColumnWidth) << TotalPrinted << "\n"
      << "\n";
}

void SummaryTable::incrementFound(const Object *Obj) {
  if (SummaryTableRow *Row = getCorrespondingRow(Obj)) {
    ++(Row->ObjectsFound);
    ++TotalFound;
  }
}

void SummaryTable::incrementPrinted(const Object *Obj) {
  if (SummaryTableRow *Row = getCorrespondingRow(Obj)) {
    ++(Row->ObjectsPrinted);
    ++TotalPrinted;
  }
}

SummaryTable::SummaryTableRow *
SummaryTable::getCorrespondingRow(const Object *Obj) {
  assert(Obj);
  if (!Obj)
    return nullptr;

  auto RowIt = Rows.find(Obj->getKindAsString());
  if (RowIt == Rows.end())
    return nullptr;

  return &RowIt->second;
}
