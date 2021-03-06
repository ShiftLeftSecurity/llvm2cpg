#include "llvm2cpg/CPG/BitcodeLoader.h"
#include "llvm2cpg/Logger/CPGLogger.h"
#include <ebc/BitcodeContainer.h>
#include <ebc/BitcodeRetriever.h>
#include <ebc/EmbeddedFile.h>
#include <ebc/util/Bitcode.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <set>
#include <sstream>

using namespace llvm2cpg;

BitcodeLoader::BitcodeLoader(CPGLogger &logger) : logger(logger) {}

std::unique_ptr<llvm::Module> BitcodeLoader::loadBitcode(const std::string &path) {
  auto memoryBuffer = llvm::MemoryBuffer::getFile(path);
  if (!memoryBuffer) {
    logger.uiWarning(std::string("Could not get file: " + path));
    return std::unique_ptr<llvm::Module>();
  }
  return loadBitcode(*memoryBuffer.get());
}

std::unique_ptr<llvm::Module> BitcodeLoader::loadIR(const std::string &path) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> memoryBuffer =
      llvm::MemoryBuffer::getFile(path);
  if (!memoryBuffer) {
    logger.uiWarning(std::string("Could not get file: " + path));
    return std::unique_ptr<llvm::Module>();
  }

  return loadBitcode(*memoryBuffer.get());
}

std::vector<std::unique_ptr<llvm::Module>> BitcodeLoader::extractBitcode(const std::string &path) {
  std::vector<std::unique_ptr<llvm::Module>> modules;

  ebc::BitcodeRetriever retriever(path);
  std::vector<ebc::BitcodeRetriever::BitcodeInfo> bitcodeInfo = retriever.GetBitcodeInfo();
  if (bitcodeInfo.empty()) {
    logger.uiWarning(std::string("No bitcode found in ") + path);
    return modules;
  }

  std::set<std::string> availableArchitectures;
  for (ebc::BitcodeRetriever::BitcodeInfo &info : bitcodeInfo) {
    if (info.bitcodeContainer) {
      availableArchitectures.insert(info.arch);
    }
  }

  if (availableArchitectures.size() > 1) {
    std::stringstream message;
    message << "Found more than one bitcode slice: ";
    for (const std::string &arch : availableArchitectures) {
      message << arch << " ";
    }
    logger.uiWarning(message.str());
  }

  const std::string &arch = *availableArchitectures.begin();

  for (ebc::BitcodeRetriever::BitcodeInfo &info : bitcodeInfo) {
    if (!info.bitcodeContainer || arch != info.arch) {
      continue;
    }
    logger.uiInfo(std::string("Extracting bitcode for ") + info.arch);
    for (std::unique_ptr<ebc::EmbeddedFile> &file : info.bitcodeContainer->GetRawEmbeddedFiles()) {
      std::pair<const char *, size_t> rawBuffer = file->GetRawBuffer();
      if (rawBuffer.first == nullptr || rawBuffer.second == 0) {
        continue;
      }

      /// This is a dirty hack, but we should check that the bitcode file is not a Bitcode Wrapper
      /// Which happens whenever we get the bitcode from Xcode's version of Clang
      char *data = const_cast<char *>(rawBuffer.first);
      ebc::BitcodeType bitcodeType =
          ebc::util::bitcode::GetBitcodeType(*reinterpret_cast<std::uint64_t *>(data));

      /// There is a case on macOS when -fembed-bitcode emits a Bitcode Wrapper of size 20
      /// LLVM cannot parse this bitcode and errors. We better skip this case.
      if (bitcodeType == ebc::BitcodeType::BitcodeWrapper && rawBuffer.second == 20) {
        continue;
      }

      if (bitcodeType == ebc::BitcodeType::Unknown) {
        continue;
      }

      std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBuffer(
          llvm::StringRef(rawBuffer.first, rawBuffer.second), "", false);
      if (!buffer) {
        logger.logWarning(std::string("Cannot create memory buffer"));
        continue;
      }
      std::unique_ptr<llvm::Module> module = loadBitcode(*buffer);
      if (module) {
        modules.push_back(std::move(module));
      }
    }
  }

  return modules;
}

std::unique_ptr<llvm::Module> BitcodeLoader::loadBitcode(llvm::MemoryBuffer &buffer) {
  contexts.emplace_back(new llvm::LLVMContext);
  llvm::SMDiagnostic diagnostic;
  std::unique_ptr<llvm::Module> module =
      llvm::parseIR(buffer.getMemBufferRef(), diagnostic, *contexts.back());
  if (!module) {
    /// Destroy newly created context since it's not used
    contexts.pop_back();
    std::string message;
    llvm::raw_string_ostream stream(message);
    diagnostic.print("Cannot load bitcode", stream, false, false);
    stream.flush();
    /// Trim newline
    message[message.size() - 1] = '\0';
    logger.uiWarning(message);
  }
  return module;
}
