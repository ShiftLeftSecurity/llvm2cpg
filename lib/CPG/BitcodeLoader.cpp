#include "llvm2cpg/CPG/BitcodeLoader.h"
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Support/MemoryBuffer.h>

using namespace llvm2cpg;

std::unique_ptr<llvm::Module> BitcodeLoader::loadBitcode(const std::string &path,
                                                         llvm::LLVMContext &context) {
  auto memoryBuffer = llvm::MemoryBuffer::getFile(path);
  if (!memoryBuffer) {
    llvm::errs() << "Could not get file: " << path << "\n";
    return std::unique_ptr<llvm::Module>();
  }
  llvm::Expected<std::unique_ptr<llvm::Module>> module =
      llvm::parseBitcodeFile(memoryBuffer.get()->getMemBufferRef(), context);
  if (!module) {
    logAllUnhandledErrors(module.takeError(), llvm::errs(), "\nllvm::loadBitcode failed: ");
    return std::unique_ptr<llvm::Module>();
  }

  return std::move(module.get());
}
