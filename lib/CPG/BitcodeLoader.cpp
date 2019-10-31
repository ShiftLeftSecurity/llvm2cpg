#include "llvm2cpg/CPG/BitcodeLoader.h"
#include "llvm2cpg/Logger/CPGLogger.h"
#include <ebc/BitcodeContainer.h>
#include <ebc/BitcodeRetriever.h>
#include <ebc/EmbeddedFile.h>
#include <ebc/util/Bitcode.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <sstream>

using namespace llvm2cpg;

BitcodeLoader::BitcodeLoader(CPGLogger &logger) : logger(logger) {}

std::unique_ptr<llvm::Module> BitcodeLoader::loadBitcode(const std::string &path,
                                                         llvm::LLVMContext &context) {
  auto memoryBuffer = llvm::MemoryBuffer::getFile(path);
  if (!memoryBuffer) {
    logger.error(std::string("Could not get file: " + path));
    return std::unique_ptr<llvm::Module>();
  }
  return loadBitcode(*memoryBuffer.get(), context);
}

std::unique_ptr<llvm::Module> BitcodeLoader::loadIR(const std::string &path,
                                                    llvm::LLVMContext &context) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> memoryBuffer =
      llvm::MemoryBuffer::getFile(path);
  if (!memoryBuffer) {
    logger.error(std::string("Could not get file: " + path));
    return std::unique_ptr<llvm::Module>();
  }

  return loadBitcode(*memoryBuffer.get(), context);
}

std::vector<std::unique_ptr<llvm::Module>>
BitcodeLoader::extractBitcode(const std::string &path, llvm::LLVMContext &context) {
  std::vector<std::unique_ptr<llvm::Module>> modules;

  ebc::BitcodeRetriever retriever(path);
  std::vector<ebc::BitcodeRetriever::BitcodeInfo> bitcodeInfo = retriever.GetBitcodeInfo();
  if (bitcodeInfo.empty()) {
    logger.warning(std::string("No bitcode found in ") + path);
    return modules;
  }

  for (ebc::BitcodeRetriever::BitcodeInfo &info : bitcodeInfo) {
    if (!info.bitcodeContainer) {
      logger.warning(std::string("No bitcode found for ") + info.arch);
      continue;
    }
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
      assert(bitcodeType != ebc::BitcodeType::Unknown);
      if (bitcodeType == ebc::BitcodeType::BitcodeWrapper) {
        continue;
      }

      std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBuffer(
          llvm::StringRef(rawBuffer.first, rawBuffer.second), "", false);
      if (!buffer) {
        logger.error(std::string("Cannot create memory buffer"));
        continue;
      }
      std::unique_ptr<llvm::Module> module = loadBitcode(*buffer, context);
      if (module) {
        modules.push_back(std::move(module));
      }
    }
  }

  return modules;
}

std::unique_ptr<llvm::Module> BitcodeLoader::loadBitcode(llvm::MemoryBuffer &buffer,
                                                         llvm::LLVMContext &context) {
  llvm::SMDiagnostic diagnostic;
  std::unique_ptr<llvm::Module> module =
      llvm::parseIR(buffer.getMemBufferRef(), diagnostic, context);
  if (!module) {
    std::string message;
    llvm::raw_string_ostream stream(message);
    diagnostic.print("llvm2cpg", stream);
    stream.flush();
    logger.error(message);
  }
  return module;
}
