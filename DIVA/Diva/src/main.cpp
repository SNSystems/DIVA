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
#include "ScopeYAMLPrinter.h"
#include "StringPool.h"
#include "SummaryTable.h"
#include "Utilities.h"

#include <assert.h>
#include <memory>

namespace {

typedef std::unique_ptr<LibScopeView::Reader> ReaderUPtr;

/// \brief Allocate an appropriate reader for the given file.
ReaderUPtr createReader(const std::string &InputFilePath) {
  if (LibScopeView::isFileFormatElf(InputFilePath))
    return std::make_unique<ElfDwarfReader::DwarfReader>();

  fatalError(LibScopeError::ErrorCode::ERR_INVALID_FILE, InputFilePath);
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

  std::vector<ReaderUPtr> Readers;

  // Create readers and load the input files.
  for (const std::string &InputFilePath : Options.InputFiles) {
    // Check that the file exists.
    if (!LibScopeView::doesFileExist(InputFilePath))
      fatalError(LibScopeError::ErrorCode::ERR_FILE_NOT_FOUND, InputFilePath);

    Readers.push_back(createReader(InputFilePath));
    assert(Readers.back());
    bool Result =
        Readers.back()->loadFile(InputFilePath, Options.PrintingSettings);

    if (!Result)
      // Currently the ElfDwarfReader will always call fatalError itself so we
      // should never reach this code.
      fatalError(LibScopeError::ErrorCode::ERR_READ_FAILED, InputFilePath);
  }

  if (Options.ShowScopeAllocation)
    LibScopeView::printAllocationInfo();

  // Print the scope views in the readers.
  for (auto &AReader : Readers) {
    // Print the Logical View.
    if (Options.OutputFormats.count(OutputFormat::TEXT)) {
      AReader->print(Options.PrintingSettings);
    }
    // Print YAML.
    if (Options.OutputFormats.count(OutputFormat::YAML)) {
      // YAML_OUTPUT_VERSION_STR is defined by CMake.
      LibScopeView::ScopeYAMLPrinter YAMLPrinter(Options.PrintingSettings,
                                                 AReader->getInputFile(),
                                                 YAML_OUTPUT_VERSION_STR);
      if (Options.PrintingSettings.SplitOutput) {
        YAMLPrinter.print(
            static_cast<LibScopeView::ScopeRoot *>(AReader->getScopesRoot()),
            Options.PrintingSettings.OutputDirectory);
      } else {
        YAMLPrinter.print(AReader->getScopesRoot(), std::cout);
      }
    }
    // Print summary.
    if (Options.ShowSummary) {
      const LibScopeView::PrintSettings *Settings = &Options.PrintingSettings;
      // Print settings were ignored for YAML.
      if (Options.OutputFormats.count(OutputFormat::YAML))
        Settings = nullptr;
      LibScopeView::SummaryTable Table(*AReader->getScopesRoot(), Settings);
      Table.printSummaryTable(std::cout);
    }
  }

  // Print string pool data.
  if (Options.DumpStringPool)
    LibScopeView::StringPool::dumpPool();
  if (Options.ShowStringPoolInfo)
    LibScopeView::StringPool::poolInfo();

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
