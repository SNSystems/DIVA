//===-- Diva/main.cpp -------------------------------------------*- C++ -*-===//
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
/// Contains the entry point for the DIVA executable.
///
//===----------------------------------------------------------------------===//

#include "DivaOptions.h"
#include "ElfDwarfReader.h"
#include "Error.h"
#include "FileUtilities.h"
#include "PrintSettings.h"
#include "ScopeTextPrinter.h"
#include "ScopeYAMLPrinter.h"
#include "StringPool.h"
#include "SummaryTable.h"
#include "Utilities.h"

#include <assert.h>
#include <memory>

namespace {

/// \brief Read an input file, creating a Scope tree.
std::unique_ptr<LibScopeView::ScopeRoot>
readInputFile(const std::string &InputFilePath,
              const LibScopeView::PrintSettings &Settings) {
  // Check that the file exists.
  if (!LibScopeView::doesFileExist(InputFilePath))
    fatalError(LibScopeError::ErrorCode::ERR_FILE_NOT_FOUND, InputFilePath);

  // Create an appropriate reader.
  std::unique_ptr<LibScopeView::Reader> Reader;
  if (LibScopeView::isFileFormatElf(InputFilePath))
    Reader = std::make_unique<ElfDwarfReader::DwarfReader>();

  if (!Reader)
    fatalError(LibScopeError::ErrorCode::ERR_INVALID_FILE, InputFilePath);

  // Load the file.
  std::unique_ptr<LibScopeView::ScopeRoot> Root =
      Reader->loadFile(InputFilePath, Settings);
  if (!Root)
    // Currently the ElfDwarfReader will always call fatalError itself so we
    // should never reach this code.
    fatalError(LibScopeError::ErrorCode::ERR_READ_FAILED, InputFilePath);

  return Root;
}

void printScopeView(const LibScopeView::ScopeRoot &Root,
                    const std::string &InputFilePath,
                    const DivaOptions &Options) {
  if (Options.ShowScopeAllocation)
    LibScopeView::printAllocationInfo(std::cout);

  std::vector<std::unique_ptr<LibScopeView::ScopePrinter>> Printers;

  // Create text printer.
  if (Options.OutputFormats.count(OutputFormat::TEXT))
    Printers.emplace_back(std::make_unique<LibScopeView::ScopeTextPrinter>(
        Options.PrintingSettings, InputFilePath));
  // Create YAML printer.
  if (Options.OutputFormats.count(OutputFormat::YAML))
    Printers.emplace_back(std::make_unique<LibScopeView::ScopeYAMLPrinter>(
        Options.PrintingSettings, InputFilePath, YAML_OUTPUT_VERSION_STR));

  // Print the Logical Views.
  for (auto &Printer : Printers) {
    if (Options.PrintingSettings.SplitOutput) {
      Printer->print(&Root, Options.PrintingSettings.OutputDirectory);
    } else if (!Options.PrintingSettings.QuietMode) {
      Printer->print(&Root, std::cout);
    }
  }

  // Print summary.
  if (Options.ShowSummary) {
    const auto *Settings = &Options.PrintingSettings;
    // Print settings were ignored for YAML.
    if (Options.OutputFormats.count(OutputFormat::YAML))
      Settings = nullptr;
    LibScopeView::SummaryTable Table(Root, Settings);
    std::cout << '\n';
    Table.printSummaryTable(std::cout);
  }
}

} // namespace

int main(int argc, char *argv[]) {
  auto StartTime = LibScopeView::getCurrentTime();

  // Library and general initialization.
  LibScopeView::initialize();

  // Argument parsing.
  const std::vector<std::string> CMDArgs(argv + 1, argv + argc);
  const DivaOptions Options(CMDArgs, /*HelpOut*/ std::cout,
                            /*VersionOut*/ std::cerr,
                            /*ErrOut*/ std::cerr);

  // Load and print each input file.
  for (const std::string &InputFilePath : Options.InputFiles) {
    auto Root = readInputFile(InputFilePath, Options.PrintingSettings);
    printScopeView(*Root, InputFilePath, Options);
  }

  // Print string pool data.
  if (Options.DumpStringPool)
    LibScopeView::StringPool::dumpPool(std::cout);
  if (Options.ShowStringPoolInfo)
    LibScopeView::StringPool::poolInfo(std::cout);

  // Library termination.
  LibScopeView::terminate();

  // Print performance data.
  if (Options.ShowPerformanceTime) {
    auto EndTime = LibScopeView::getCurrentTime();
    LibScopeView::printTimeTaken(StartTime, EndTime);
  }
  if (Options.ShowPerformanceMemory) {
    LibScopeView::printMemoryUsage(LibScopeView::getPeakMemoryUsage());
  }

  return 0;
}
