#pragma once

#include <llvm/IR/Module.h>
#include <memory>
#include <string>

namespace llvm2cpg {

class BitcodeLoader {
public:
  BitcodeLoader() = default;
  std::unique_ptr<llvm::Module> loadBitcode(const std::string &path, llvm::LLVMContext &context);

private:
};

} // namespace llvm2cpg
